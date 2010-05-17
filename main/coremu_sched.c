/*
 * COREMU Parallel Emulator Framework
 *
 * Scheduling support for COREMU parallel emulation library
 *
 * Copyright (C) 2010 PPI, Fudan Univ. <http://ppi.fudan.edu.cn/system_research_group>
 *
 * Authors:
 *  Zhaoguo Wang    <zgwang@fudan.edu.cn>
 *  Yufei Chen      <chenyufei@fudan.edu.cn>
 *  Ran Liu         <naruilone@gmail.com>
 *  Xi Wu           <wuxi@fudan.edu.cn>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#define DEBUG_COREMU     0
#define VERBOSE_COREMU   0

#define DEBUG_CM_SCHED   0
#include "coremu_utils.h"
#include "coremu_core.h"
#include "coremu_sched.h"

int min_prio, max_prio;
int low_prio, avg_prio, high_prio;
int init_all_ok = 0;
int host_cpu_avail = 0;

/* Each thread is modeled as a state in 3 states:
    1. STATE_HALTED: the core is waiting some evnet to work.
    2. STATE_CPU: the core is in a cpu-intensive state.
    3. STATE_IO:  the core is busy doing IO. */
__thread int cur_state;
__thread struct timeval start;
__thread struct timeval end;
__thread int io_count = 0;
__thread int cpu_count = 0;
__thread int hw_io_count = 0;

topo_topology_t topology;
struct topo_topology_info topoinfo;
static unsigned int depth;
static unsigned int cores;

extern int smp_cpus;

static inline void sched_halted(void);
static inline void sched_cpu(void);
static inline void sched_io(void);
static void display_thread_sched_attr(char *msg);
static void topology_init(void);
static void print_children(topo_topology_t topology, topo_obj_t obj, int depth);

static void print_children(topo_topology_t topology, topo_obj_t obj, int depth)
{
    char string[128];
    int i;

    topo_obj_snprintf(string, sizeof(string), topology, obj, "#", 0);
    printf("%*s%s\n", 2*depth, "", string);
    for (i = 0; i < obj->arity; i++)
        print_children(topology, obj->children[i], depth + 1);
}

static void topology_init()
{
    topo_topology_init(&topology);
    topo_topology_load(topology);

    topo_topology_get_info(topology, &topoinfo);
    depth = topo_get_type_or_below_depth(topology, TOPO_OBJ_CORE);
    cores = topo_get_depth_nbobjs(topology, depth);

    fprintf(stderr, "----------------- Dump toplogy[%ud] info -----------------\n", cores);
    /* Dump the toplogy info */
    print_children(topology, topo_get_system_obj(topology), 0);
    fprintf(stderr, "----------------------------------------------------------\n");
}

static void topology_bind_hw()
{
    topo_obj_t obj;
    topo_cpuset_t cpuset;

    obj = topo_get_obj_by_depth(topology, depth, 0);
    cpuset = obj->cpuset;
    topo_cpuset_singlify(&cpuset);

    if (topo_set_cpubind(topology, &cpuset, TOPO_CPUBIND_THREAD)) {
        char s[TOPO_CPUSET_STRING_LENGTH + 1];
        topo_cpuset_snprintf(s, sizeof(s), &obj->cpuset);
        printf("Couldn't bind to cpuset %s\n", s);
        exit(-1);
    }

    fprintf(stderr, "hw [%lu] binds to %d\n",
            (unsigned long int)coremu_gettid() , 0);
}

static void topology_bind_core()
{
    topo_obj_t obj;
    topo_cpuset_t cpuset;   
    CMCore *self = coremu_get_self();
    int index;

#if CM_BIND_SOCKET
    unsigned int socket_depth;
    unsigned int socket_num;

    
    socket_depth = topo_get_type_depth(topology, TOPO_OBJ_SOCKET);
    socket_num =  topo_get_depth_nbobjs(topology, socket_depth);
    if(smp_cpus > host_cpu_avail)
    {
        int i = smp_cpus/host_cpu_avail;
        index = (self->serial / i)/socket_num;
    }
    else
    {
        index = (self->serial % 16) / socket_num;

    }
    obj = topo_get_obj_by_depth(topology, socket_depth, index);

    assert(obj!=0);

    cpuset = obj->cpuset;

    if (topo_set_cpubind(topology, &cpuset, 0)) {
            char s[TOPO_CPUSET_STRING_LENGTH + 1];
            topo_cpuset_snprintf(s, sizeof(s), &obj->cpuset);
            printf("Couldn't bind to cpuset %s\n", s);
    }

#else

#if CM_BIND_SAME_CORE2

    

    if(smp_cpus > host_cpu_avail)
    {
        int i = smp_cpus/host_cpu_avail;
        assert(i > 0 );
        index = (self->serial / i);
        assert(index < cores );
        
    }else
    {
        index = (self->serial % cores);

    }
        
#elif CM_BIND_SAME_CORE

       index = (self->serial % cores);

#endif

        obj = topo_get_obj_by_depth(topology, depth, index);
        cpuset = obj->cpuset;
        topo_cpuset_singlify(&cpuset);

        if (topo_set_cpubind(topology, &cpuset, TOPO_CPUBIND_THREAD)) {
            char s[TOPO_CPUSET_STRING_LENGTH + 1];
            topo_cpuset_snprintf(s, sizeof(s), &obj->cpuset);
            printf("Couldn't bind to cpuset %s\n", s);
            exit(-1);
        }

        fprintf(stderr, "core [%u] binds to %d\n", self->serial, index);

#endif

}

void coremu_init_sched_all()
{
    /* Get the available processors */
    host_cpu_avail = sysconf(_SC_NPROCESSORS_ONLN);

    /* HIGH numeric value indicates LOW priority,
       and vice versa */
    max_prio = -21; min_prio = 19;
    avg_prio = (max_prio + min_prio) / 2;
    low_prio = (avg_prio + min_prio) / 2;
    high_prio = (max_prio + avg_prio) / 2;

    cm_print("[priority]: max[%d], min[%d], "
             "low[%d], avg[%d], high[%d]",
             max_prio, min_prio,
             low_prio, avg_prio, high_prio);

    /* set priority for main thread
       NOTE: now it is HW thread. */
    //assert(! setpriority(PRIO_PROCESS, 0, high_prio));
    //assert(! sched_setscheduler(0, SCHED_RR, NULL));
    display_thread_sched_attr("MAIN thread scheduling settings:");

    topology_init();

    /* bind hardware thread to the fisrt core */
    //topology_bind_hw();
    cur_state = STATE_CNT;
    init_all_ok = 1;
}

int coremu_get_hostcpu()
{
    return host_cpu_avail;
}

int coremu_get_maxprio()
{
    return high_prio;
}

int coremu_get_minprio()
{
    return low_prio;
}

void coremu_init_sched_core()
{
    int policy;
    struct sched_param param;
    assert(! pthread_getschedparam(pthread_self(),
                                   &policy, &param));
    assert(policy == CM_SCHED_POLICY);

    //assert(! setpriority(PRIO_PROCESS, 0, avg_prio));

    /* display the scheduling info */
    display_thread_sched_attr("CORE thread scheduler settings:");

#if CM_ENABLE_BIND_CORE
    /* bind thread to a specific core */
    topology_bind_core();
#endif

    cur_state = STATE_HALTED;
    gettimeofday(&start, NULL);
}

/* Receive some indication of events, and change the
   scheduling policy/priority of the calling thread.

   Xi Wu (wuxi@fudan.edu.cn) */
void coremu_sched(sched_event_t e)
{
#if DEBUG_CM_SCHED
    cm_assert((init_all_ok == 1),
              "scheduling support not initialized?");
    cm_assert(cur_state < STATE_CNT,
              "invalid state");
    coremu_assert_not_hw_thr("hw thr call scheduling?");
#endif

#if CM_ENABLE_SCHED
    switch(e) {
    case EVENT_HALTED:
    {
        sched_halted();
        break;
    }
    case EVENT_CPU:
    {
        sched_cpu();
        break;
    }
    case EVENT_IO:
    {
        sched_io();
        break;
    }
    default:
    {
        assert(0);
    }
    }
#endif
}

void coremu_hw_sched(sched_hw_event_t e)
{
#if CM_ENABLE_SCHED
    static int hw_high_prio_p = 0;
    switch(e) {
    case EVENT_NO_IOREQ:
    {
        assert(! setpriority(PRIO_PROCESS, 0, low_prio));
        hw_high_prio_p = 0;
        break;
    }
    case EVENT_IOREQ:
    {
        if(hw_io_count < CM_HW_IO_CNT) {
            hw_io_count++;
        }

        if((hw_io_count >= CM_HW_IO_CNT)
           && (! hw_high_prio_p))
        {
            assert(! setpriority(PRIO_PROCESS,
                                 0, high_prio));
            hw_high_prio_p = 1;
        }

        break;
    }
    default:
    {
        assert(0);
    }
    }
#endif
}

/* handle the halted event */
static inline void sched_halted()
{
    if(cur_state != STATE_HALTED) {
        /* transition from an active state to a halted state,
           this is likely to last for a while. So we change
           the prio to lowest value */
        assert(! setpriority(PRIO_PROCESS, 0, low_prio));
        io_count = 0; cpu_count = 0;
        cur_state = STATE_HALTED;
    }
}

/* handle the cpu event */
static inline void sched_cpu()
{
    int prio, new_prio;
    prio = getpriority(PRIO_PROCESS, 0);

    if(cur_state == STATE_HALTED)
    {
        /* transit to active. change the prio to avg.*/
        assert(! setpriority(PRIO_PROCESS, 0, avg_prio));
        cur_state = STATE_CPU;
        cpu_count = 0;

    } else if(cur_state == STATE_IO) {

        /* how long have we not done synchronized IO? */
        struct timeval elapsed;
        gettimeofday(&end, NULL);
        timeval_subtract(&elapsed, &end, &start);

        if(elapsed.tv_sec >= CM_NO_IO_LIMIT) {
            /* dec the prio to avg */
            new_prio = avg_prio;
            assert(! setpriority(PRIO_PROCESS, 0, new_prio));

            cur_state = STATE_CPU;
            io_count = 0;
            cpu_count = 0;
        }

    } else if(cur_state == STATE_CPU) {
#if 0
        /* inc the prio for busy working */
        if(prio > high_prio) {
            new_prio = MIN((prio - 5), high_prio);
        } else {
            new_prio = high_prio;
        }

        assert(! setpriority(PRIO_PROCESS, 0, new_prio));
#endif

        cpu_count++;
        if(cpu_count >= CM_PRIO_CPU_CNT) {
            new_prio = avg_prio;
            assert(! setpriority(PRIO_PROCESS, 0, new_prio));
        }
    } else {
        assert(0);
    }
}

/* handle the io event */
static inline void sched_io()
{
    int prio, new_prio;
    prio = getpriority(PRIO_PROCESS, 0);

    if(cur_state == STATE_IO) {

        /* refresh the time of IO */
        gettimeofday(&start, NULL);
        io_count++;

        if(io_count >= CM_BONUS_IO_CNT) {
            /* high bonus for performing sync IO many times. */
            new_prio = high_prio;
            assert(! setpriority(PRIO_PROCESS, 0, new_prio));
        }

    } else if(cur_state == STATE_CPU) {
        /* refresh the start time of IO */
        gettimeofday(&start, NULL);
        io_count = 0;
        cur_state = STATE_IO;
    } else {
        assert(0);
    }
}

static void display_thread_sched_attr(char *msg)
{
    int policy, prio;
    struct sched_param param;

    prio = getpriority(PRIO_PROCESS, 0);
    assert(! pthread_getschedparam(pthread_self(),
                                   &policy, &param));

    cm_print("-- tid[%lu] %s start --",
             (unsigned long int) coremu_gettid(), msg);

    cm_print("policy=%s, priority=%d",
           (policy == SCHED_FIFO)  ? "SCHED_FIFO" :
           (policy == SCHED_RR)    ? "SCHED_RR" :
           (policy == SCHED_OTHER) ? "SCHED_OTHER" :
           "???",
           prio);

    cm_print("-- thr[%lu] %s end --\n",
             (unsigned long int) coremu_gettid(), msg);
}


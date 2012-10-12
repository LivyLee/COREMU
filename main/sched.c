/*
 * COREMU Parallel Emulator Framework
 *
 * Scheduling support for COREMU parallel emulation library
 *
 * Copyright (C) 2010 Parallel Processing Institute (PPI), Fudan Univ.
 *  <http://ppi.fudan.edu.cn/system_research_group>
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

/*#define DEBUG_CM_SCHED*/

#define _GNU_SOURCE
/* system headers */
#include <sys/time.h>
#include <sys/resource.h>
#include <sched.h>
#include <unistd.h>

#include "coremu-config.h"
#include "coremu-thread.h"
#include "utils.h"
#include "core.h"
#include "coremu-sched.h"

#define DEBUG_COREMU
#include "coremu-debug.h"

int min_prio, max_prio;
int low_prio, avg_prio, high_prio;
int host_cpu_avail = 0;

__thread unsigned long int pause_cnt = 0;
#define PAUSE_THRESHOLD     100
#define PAUSE_SLEEP_TIME    100000  // 100,000 ns = 100us

static inline void sched_halted(void);
static inline void sched_pause(void);
static void display_thread_sched_attr(char *msg);

#ifdef ENABLE_HWLOC
/* If enabled, bind threads using the least sockets possible. */
#include <hwloc.h>

hwloc_topology_t topology;
static unsigned int core_depth;
static unsigned int ncores;

#ifdef VERBOSE_QEMU
static void print_children(hwloc_topology_t topology, hwloc_obj_t obj, int depth)
{
    char string[128];
    int i;

    hwloc_obj_snprintf(string, sizeof(string), topology, obj, "#", 0);
    printf("%*s%s\n", 2 * depth, "", string);
    for (i = 0; i < obj->arity; i++)
        print_children(topology, obj->children[i], depth + 1);
}
#endif

static void hwloc_init()
{
    hwloc_topology_init(&topology);
    /* Performthe topology detection. */
    hwloc_topology_load(topology);

    core_depth = hwloc_get_type_or_below_depth(topology, HWLOC_OBJ_CORE);
    coremu_debug("depth: %d", core_depth);
    ncores = hwloc_get_nbobjs_by_depth(topology, core_depth);
    coremu_debug("cores: %d", ncores);

#ifdef VERBOSE_QEMU
    fprintf(stderr, "----------------- Dump toplogy[%ud] info -----------------\n", cores);
    /* Dump the toplogy info */
    print_children(topology, hwloc_get_root_obj(topology), 0);
    fprintf(stderr, "----------------------------------------------------------\n");
#endif
}

static void bind_core()
{
    hwloc_obj_t obj;
    hwloc_cpuset_t cpuset;
    CMCore *self = coremu_get_core_self();
    int index;

    index = (self->serial % ncores);

    /* Refer to hwloc api example the 5th example for more details. */
    obj = hwloc_get_obj_by_depth(topology, core_depth, index);
    if (obj) {
        cpuset = hwloc_cpuset_dup(obj->cpuset);
        /* Get only one logical processor (in case the core is
           SMT/hyperthreaded). */
        hwloc_cpuset_singlify(cpuset);

        /* Bind self. */
        if (hwloc_set_cpubind(topology, cpuset, HWLOC_CPUBIND_THREAD)) {
            char *str;
            hwloc_cpuset_asprintf(&str, obj->cpuset);
            printf("Couldn't bind to cpuset %s\n", str);
            free(str);
        } else {
#ifdef VERBOSE_QEMU
            fprintf(stderr, "core [%u] binds to %d\n", self->serial, index);
#endif
        }

        hwloc_cpuset_free(cpuset);
    }

}
#endif

void coremu_init_sched_all()
{
    /* Get the available processors */
    host_cpu_avail = sysconf(_SC_NPROCESSORS_ONLN);

    /* HIGH numeric value indicates LOW priority,
       and vice versa */
    max_prio = -21;
    min_prio = 19;
    avg_prio = (max_prio + min_prio) / 2;
    low_prio = (avg_prio + min_prio) / 2;
    high_prio = (max_prio + avg_prio) / 2;

    coremu_print("[priority]: max[%d], min[%d], "
             "low[%d], avg[%d], high[%d]",
             max_prio, min_prio, low_prio, avg_prio, high_prio);

    /* set priority for main thread
       NOTE: now it is HW thread. */
    //assert(! setpriority(PRIO_PROCESS, 0, high_prio));
    //assert(! sched_setscheduler(0, SCHED_RR, NULL));
    display_thread_sched_attr("MAIN thread scheduling settings:");
    coremu_thread_setpriority(PRIO_PROCESS, 0, high_prio);

    /* bind hardware thread to the fisrt core */
#ifdef ENABLE_HWLOC
    hwloc_init();
#endif
}

int coremu_get_hostcpu()
{
    return host_cpu_avail;
}

int coremu_get_targetcpu()
{
    return cm_smp_cpus;
}

int coremu_get_thrs_per_core()
{
    return (host_cpu_avail + cm_smp_cpus - 1) / host_cpu_avail;
}

int coremu_physical_core_enough_p()
{
    return host_cpu_avail >= cm_smp_cpus;
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
    CMCore *self;
    struct sched_param param;
    assert(!pthread_getschedparam(pthread_self(), &policy, &param));
    assert(policy == CM_SCHED_POLICY);

    coremu_thread_setpriority(PRIO_PROCESS, 0, avg_prio);
    self = coremu_get_core_self();
    self->tid = coremu_gettid();

    /* display the scheduling info */
    display_thread_sched_attr("CORE thread scheduler settings:");

#ifdef ENABLE_HWLOC
    /* bind thread to a specific core */
    bind_core();
#endif

}

void coremu_cpu_sched(CMSchedEvent e)
{
    switch (e) {
    case CM_EVENT_HALTED:
        sched_halted();
        break;
    case CM_EVENT_PAUSE:
        sched_pause();
        break;
    default:
        assert(0);
    }
}

/* handle the halted event */
#define HALT_SLEEP_MAX_TIME 5000000 // 5000,000ns = 5ms this is the smallest quantum
static inline void sched_halted()
{
    struct timespec halt_interval;
    CMCore *self = coremu_get_core_self();

    self->state = CM_STATE_HALT;
    halt_interval.tv_sec = 0;
    halt_interval.tv_nsec = HALT_SLEEP_MAX_TIME;
    nanosleep(&halt_interval, NULL);
    self->state = CM_STATE_RUN;
}

/* handle the pause event */
static inline void sched_pause()
{
    struct timespec pause_interval;
    CMCore *self = coremu_get_core_self();

    pause_interval.tv_sec = 0;
    pause_interval.tv_nsec = PAUSE_SLEEP_TIME;

    if (pause_cnt < PAUSE_THRESHOLD) {
        pause_cnt++;
    } else if (pause_cnt <= PAUSE_THRESHOLD + 1) {
        pause_cnt++;
        pthread_yield();
    } else {
        self->state = CM_STATE_PAUSE;
        nanosleep(&pause_interval, NULL);
        self->state = CM_STATE_RUN;
        pause_cnt = 0;
    }
}

static void display_thread_sched_attr(char *msg)
{
    int policy;
    struct sched_param param;

    assert(!pthread_getschedparam(pthread_self(), &policy, &param));

    coremu_print("-- tid[%lu] %s start --",
             (unsigned long int)coremu_gettid(), msg);

    coremu_print("policy=%s, priority=%d",
             (policy == SCHED_FIFO) ?
                 "SCHED_FIFO" :
                 (policy == SCHED_RR) ?
                     "SCHED_RR" :
                     (policy == SCHED_OTHER) ?
                         "SCHED_OTHER" :
                         "???", getpriority(PRIO_PROCESS, 0));

    coremu_print("-- thr[%lu] %s end --\n",
             (unsigned long int)coremu_gettid(), msg);
}

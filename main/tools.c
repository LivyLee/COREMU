/* @(#)tools.c
 *
 * Common tools used in COREMU
 *
 * Author: Xi Wu (wuxi@fudan.edu.cn) */

#include "coremu_utils.h"

/**
 * serializing instruction stream using CPUID
 * Example: cpuid(1, eax, ebx, ecx, edx); */
#define cpuid(index, eax, ebx, ecx, edx) \
    asm volatile ("cpuid"                                           \
                  : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)  \
                  : "0" (index))

/* set the relative timeout for tsp */
void maketimeout(struct timespec *tsp, long seconds)
{
    //struct timeval now;

    /* get the current time */
    //gettimeofday(&now, NULL);
    //tsp->tv_sec = now.tv_sec;
    //tsp->tv_nsec = now.tv_usec * 1000; /* usec to nsec */

    /* add the offset to get timeout value */
    tsp->tv_sec = seconds;
    tsp->tv_nsec = 0;           /* must clear the nsec */
}

/* Subtract the `struct timeval' values X and Y,
   storing the result in RESULT.
   Return 1 if the difference is negative, otherwise 0.  */

int timeval_subtract(struct timeval *result,
                     struct timeval *x, struct timeval *y)
{
    /* Perform the carry for the later subtraction by updating y. */
    if (x->tv_usec < y->tv_usec) {
        int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
        y->tv_usec -= 1000000 * nsec;
        y->tv_sec += nsec;
    }
    if (x->tv_usec - y->tv_usec > 1000000) {
        int nsec = (x->tv_usec - y->tv_usec) / 1000000;
        y->tv_usec += 1000000 * nsec;
        y->tv_sec -= nsec;
    }

    /* Compute the time remaining to wait.
       tv_usec is certainly positive. */
    result->tv_sec = x->tv_sec - y->tv_sec;
    result->tv_usec = x->tv_usec - y->tv_usec;

    /* Return 1 if result is negative. */
    return x->tv_sec < y->tv_sec;
}


unsigned long read_host_tsc(void)
{
    unsigned a, d;
    __asm __volatile("rdtsc":"=a"(a), "=d"(d));
    return ((unsigned long)a) | (((unsigned long) d) << 32);
}

/* random a number [min, max) */
int coremu_random(int min, int max)
{
    return random() % (max - min) + min;
}

FILE *coremu_fopen(const char *filename, const char *mode)
{
    FILE *stream = fopen(filename, mode);
    assert(stream != NULL);

    return stream;
}

void coremu_serialize()
{
    uint32_t eax, ebx, ecx, edx;
    cpuid(1, eax, ebx, ecx, edx);
}

pid_t coremu_gettid()
{
    pid_t tid;
    tid = syscall(SYS_gettid);
    return tid;
}

void coremu_backtrace()
{
#define SIZE 100
    void *buffer[SIZE];
    int j, nptrs;
    nptrs = backtrace(buffer, SIZE); /* get the address on the frame */
    char **strings = backtrace_symbols(buffer, nptrs);
    if(strings != NULL) {
        cm_print("Dump the call stack");
        for(j = 0; j < nptrs; j++)
            printf("%s\n", strings[j]);
    }
    free(strings);
}



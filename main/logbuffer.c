/*
 * COREMU Parallel Emulator Framework
 *
 * Copyright (C) 2010 Parallel Processing Institute, Fudan Univ.
 *  <http://ppi.fudan.edu.cn/system_research_group>
 *
 * Authors:
 *  Zhaoguo Wang    <zgwang@fudan.edu.cn>
 *  Yufei Chen      <chenyufei@fudan.edu.cn>
 *  Ran Liu         <naruilone@gmail.com>
 *
 *  License: LGPL version 2.
 */

#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "coremu-malloc.h"
#include "coremu-logbuffer.h"

/*#define DEBUG_COREMU*/
#include "coremu-debug.h"

static pthread_t log_thread_id;
static int logpipe[2];

static void print_logbuf(CMLogBuf *logbuf, const char *msg)
{
    coremu_debug("%s buf: %p, end: %p, file: %p", msg,
                 logbuf->buf, logbuf->end, logbuf->file);
}

static void *coremu_record_thread(void *arg)
{
    coremu_debug("logging thread started");
    CMLogBuf *logbuf;
    while (read(logpipe[0], &logbuf, sizeof(logbuf)) > 0){
        print_logbuf(logbuf, "writing out log");
        /* Note we write till the current end of the log buffer. So we can
           handle buffers that are not full. */
        fwrite(logbuf->buf, logbuf->cur - logbuf->buf, 1, logbuf->file);
        coremu_logbuf_free(logbuf);
    }
    return NULL;
}

void coremu_logbuffer_init(void)
{
    if (pipe(logpipe) != 0) {
        perror("log buffer pipe creation failed");
        exit(1);
    }
    if (pthread_create(&log_thread_id, NULL, coremu_record_thread, NULL) != 0) {
        perror("log thread creation failed");
        exit(1);
    }
}

CMLogBuf *coremu_logbuf_new(int size, FILE *file)
{
    CMLogBuf *logbuf = coremu_mallocz(sizeof(*logbuf));

    logbuf->buf = coremu_mallocz(size);
    logbuf->cur = logbuf->buf;
    logbuf->end = size + logbuf->buf;
    logbuf->file = file;

    print_logbuf(logbuf, "creating new log buffer");
    return logbuf;
}

void coremu_logbuf_free(CMLogBuf *logbuf)
{
    free(logbuf->buf);
    free(logbuf);
}

void coremu_logbuf_flush(CMLogBuf *logbuf)
{
    print_logbuf(logbuf, "flushing buf");
    if (write(logpipe[1], &logbuf, sizeof(logbuf)) == -1) {
        perror("log buffer flush error");
        exit(1);
    }
}

void coremu_logbuf_wait_thread_exit(CMLogBuf *buf)
{
    // Close the write end, then wait the log thread to exit.
    close(logpipe[1]);
    pthread_join(log_thread_id, NULL);

    if (setpriority(PRIO_PROCESS, log_thread_id, -19) != 0) {
        perror("set log thread high priority.");
        exit(1);
    }
}


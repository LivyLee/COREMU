/*
 * COREMU Parallel Emulator Framework
 *
 * Logging facility for COREMU.
 * Note that the order of log message is not reserved.
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

/*
 * Each threads write to its own buffer, when the buffer is full, write the
 * pointer and other information to a pipe. The writing thread reads from the
 * pipe the buffer information and then write the buffer content out.
 *
 * Using pipe make it easy to wake up the writing thread when there's work need
 * to be done.
 *
 * Currently only one log writing thread is used. If this is not enough (writing
 * to the pipe will block), we can increase the priority of this thread or
 * create log thread for each running vCPU. (1-to-1 mapping between vCPU and log
 * thread is much easier than multiplexing log writing in multiple threads and
 * avoiding threads writing to the same file at the same time.)
 */

#ifndef _COREMU_LOGBUFFER_H
#define _COREMU_LOGBUFFER_H

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

void coremu_logbuffer_init(void);

/* I want to make the next_entry function inline, so have to expose the
 * structrue of CMLogBuf here. */
typedef struct {
    void *buf;
    void *cur;      /* Current position in the buffer. */
    void *end;      /* end - buf = bufsize */
    FILE *file;
    int  fileno;
} CMLogBuf;

/* Create and initialize new CMLogBuf instance. */
CMLogBuf *coremu_logbuf_new(int size, FILE *file);
/* Free the memory held by the logbuf and itself. */
void coremu_logbuf_free(CMLogBuf *logbuf);
/* Give the buffer to the writing thread.*/
void coremu_logbuf_flush(CMLogBuf *logbuf);

/* Wait the write thread to finish */
void coremu_logbuf_wait_thread_exit(CMLogBuf *buf);

/* Get the next log entry. */
static inline void *coremu_logbuf_next_entry(CMLogBuf **logbuf_p, int size)
{
    CMLogBuf *logbuf = *logbuf_p;
    if (logbuf->cur + size > logbuf->end) {
        coremu_logbuf_flush(logbuf);
        // Need to create new logbuffer. And USE THAT BUFFER!
        logbuf = coremu_logbuf_new(logbuf->end - logbuf->buf, logbuf->file);
        *logbuf_p = logbuf;
    }
    void *ent = logbuf->cur;
    logbuf->cur += size;
    return ent;
}

#endif /* _COREMU_LOGBUFFER_H */

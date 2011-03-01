#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "closure.h"

// This code can now only work on Linux system.
// It has some problem on OS X. Maybe I'll fix it someday.
#if __WORDSIZE != 64 || !defined(__x86_64__)
#error "This program only works on x86_64 Linux."
#endif

struct trampoline_code {
    char mov_target[2];
    uint32_t target;
    char mov_env[2];
    uint64_t env; // value stored in r10
    char jmp[3];
} __attribute__((packed));
typedef struct trampoline_code trampoline_code;

void *create_closure(void *f) {
    trampoline_code *tramp = (trampoline_code *)f;
    int env_size = (uint8_t *)tramp - (uint8_t *)tramp->env;

    // The closure's "environment"
    uint8_t *env = malloc(env_size);
    assert(env);
    memcpy(env, (uint8_t *)tramp->env, env_size);

    // Copy trampoline code
    trampoline_code *new_tramp = malloc(sizeof(trampoline_code));
    assert(new_tramp);
    new_tramp->mov_target[0] = 0x41;
    new_tramp->mov_target[1] = 0xbb;
    // Get the target address from the trampoline code
    new_tramp->target = tramp->target;
    new_tramp->mov_env[0] = 0x49;
    new_tramp->mov_env[1] = 0xba;
    // Set new_trampironment
    new_tramp->env = (uint64_t) env;
    new_tramp->jmp[0] = 0x49;
    new_tramp->jmp[1] = 0xff;
    new_tramp->jmp[2] = 0xe3;

    return new_tramp;
}

void destory_closure(void *f) {
    trampoline_code *tramp = (trampoline_code *)f;
    if (tramp) {
        free((void *)tramp->env);
        free(tramp);
    }
}

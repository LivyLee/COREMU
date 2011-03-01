#ifndef _COREMU_CLOSURE_H
#define _COREMU_CLOSURE_H

/* Turn the nested function's trampoline code into a "closure" by copying the
 * environment and trampoline code onto heap. */
void *create_closure(void *);

/* Release closure's memory. */
void destory_closure(void *);

#endif /* _COREMU_CLOSURE_H */

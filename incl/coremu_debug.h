/* @(#)cm_debug.h
 *
 * This file provides basic debugging utilities for
 * COREMU (NOT QEMU). Make sure including "cm_utils"
 * for including these debugging facilities.
 */

#ifndef _CM_DEBUG_H
#define _CM_DEBUG_H 1

#if (DEBUG_COREMU) && (CONFIG_COREMU)

#define debug(fmt, args...) fprintf(stderr, "[COREMU] %s(line %d), %s" fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##args)

#else

#define debug(fmt, args...)

#endif

#if (VERBOSE_COREMU) && (CONFIG_COREMU)

#define cm_print(fmt, args...) fprintf(stderr, "%s(line %d), %s: " fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##args)

#define cm_out(fmt, args...) printf("%s(line %d), %s: " fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##args)

#define cm_fprint(stream, fmt, args...) fprintf(stream, "%s(line %d), %s: " fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##args)

#define cm_assert(exp, fmt, args...) { \
        if(! (exp)) { \
            fprintf(stderr, "%s(line %d), %s: " fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
        assert((exp)); }

#else  /* null .. */

#define cm_print(fmt, args...)
#define cm_out(fmt, args...)
#define cm_fprint(stream, fmt, args...)
#define cm_assert(exp, fmt, args...)

#endif

#if CONFIG_COREMU
/* always have effect... */
#define cm_foutput(stream, fmt, args...) fprintf(stream, "%s(line %d), %s: " fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##args);
#endif

#endif /* _DEBUG_H */


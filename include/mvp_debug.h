/** \file debug.h
 * A C library for generating and controlling debug output
 */

#ifndef __CMYTH_DEBUG_H
#define  __CMYTH_DEBUG_H

#include <stdio.h>
#include <stdarg.h>

typedef struct {
	char *name;
	int  cur_level;
	int  (*selector)(int plevel, int slevel);
} cmyth_debug_ctx_t;

/**
 * Make a static initializer for an cmyth_debug_ctx_t which provides a debug
 * context for a subsystem.  This is a macro.
 * \param n subsystem name (a static string is fine)
 * \param l initial debug level for the subsystem
 * \param s custom selector function pointer (NULL is okay)
 */
#define CMYTH_DEBUG_CTX_INIT(n,l,s) { n, l, s }

/**
 * Set the debug level to be used for the subsystem
 * \param ctx the subsystem debug context to use
 * \param level the debug level for the subsystem
 * \return an integer subsystem id used for future interaction
 */
static inline void
__cmyth_dbg_setlevel(cmyth_debug_ctx_t *ctx, int level)
{
	if (ctx != NULL) {
		ctx->cur_level = level;
	}
}

/**
 * Generate a debug message at a given debug level
 * \param ctx the subsystem debug context to use
 * \param level the debug level of the debug message
 * \param fmt a printf style format string for the message
 * \param ... arguments to the format
 */
static inline void
__cmyth_dbg(cmyth_debug_ctx_t *ctx, int level, char *fmt, va_list ap)
{
	if (!ctx) {
		return;
	}
	if ((ctx->selector && ctx->selector(level, ctx->cur_level)) ||
	    (!ctx->selector && (level < ctx->cur_level))) {
		fprintf(stderr, "(%s)", ctx->name);
		vfprintf(stderr, fmt, ap);
	}
}

#endif /*  __CMYTH_DEBUG_H */

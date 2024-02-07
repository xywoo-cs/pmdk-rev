/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2014-2024, Intel Corporation */

/*
 * out.h -- definitions for "out" module
 */

#ifndef PMDK_OUT_H
#define PMDK_OUT_H 1

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

#include "util.h"
#include "log_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Suppress errors messages (LOG()) in non-debug version
 * builds.
 */
#ifndef EVALUATE_DBG_EXPRESSIONS
#if defined(DEBUG) || defined(__clang_analyzer__) || defined(__COVERITY__) ||\
	defined(__KLOCWORK__)
#define EVALUATE_DBG_EXPRESSIONS 1
#else
#define EVALUATE_DBG_EXPRESSIONS 0
#endif
#endif

/* klocwork does not seem to respect __attribute__((noreturn)) */
#if defined(DEBUG) || defined(__KLOCWORK__)

#define OUT_LOG out_log
#define OUT_NONL out_nonl

#else

static __attribute__((always_inline)) inline void
out_log_discard(const char *file, int line, const char *func, int level,
		const char *fmt, ...)
{
	/* suppress unused-parameter errors */
	SUPPRESS_UNUSED(file, line, func, level, fmt);
}

static __attribute__((always_inline)) inline void
out_nonl_discard(int level, const char *fmt, ...)
{
	/* suppress unused-parameter errors */
	SUPPRESS_UNUSED(level, fmt);
}

#define OUT_LOG out_log_discard
#define OUT_NONL out_nonl_discard

#endif

#if defined(__KLOCWORK__)
#define TEST_ALWAYS_TRUE_EXPR(cnd)
#define TEST_ALWAYS_EQ_EXPR(cnd)
#define TEST_ALWAYS_NE_EXPR(cnd)
#else
#define TEST_ALWAYS_TRUE_EXPR(cnd)\
	if (__builtin_constant_p(cnd))\
		ASSERT_COMPILE_ERROR_ON(cnd);
#define TEST_ALWAYS_EQ_EXPR(lhs, rhs)\
	if (__builtin_constant_p(lhs) && __builtin_constant_p(rhs))\
		ASSERT_COMPILE_ERROR_ON((lhs) == (rhs));
#define TEST_ALWAYS_NE_EXPR(lhs, rhs)\
	if (__builtin_constant_p(lhs) && __builtin_constant_p(rhs))\
		ASSERT_COMPILE_ERROR_ON((lhs) != (rhs));
#endif

/* produce debug/trace output */
#define LOG(level, ...) do { \
	if (!EVALUATE_DBG_EXPRESSIONS) break;\
	OUT_LOG(__FILE__, __LINE__, __func__, level, __VA_ARGS__);\
} while (0)

/* produce debug/trace output without prefix and new line */
#define LOG_NONL(level, ...) do { \
	if (!EVALUATE_DBG_EXPRESSIONS) break; \
	OUT_NONL(level, __VA_ARGS__); \
} while (0)

/* assert a condition is true at runtime */
#if defined(DEBUG) || defined(__clang_analyzer__) || defined(__COVERITY__) ||\
	defined(__KLOCWORK__)
#define ASSERT_rt(cnd) do { \
	if ((cnd)) break; \
	CORE_LOG_FATAL("assertion failure: %s", #cnd);\
} while (0)

/* assertion with extra info printed if assertion fails at runtime */
#define ASSERTinfo_rt(cnd, info) do { \
	if ((cnd)) break; \
	CORE_LOG_FATAL("assertion failure: %s (%s = %s)", #cnd, #info, info);\
} while (0)

/* assert two integer values are equal at runtime */
#define ASSERTeq_rt(lhs, rhs) do { \
	if ((lhs) == (rhs)) break; \
	CORE_LOG_FATAL( \
		"assertion failure: %s (0x%llx) == %s (0x%llx)", #lhs, \
		(unsigned long long)(lhs), #rhs, (unsigned long long)(rhs)); \
} while (0)

/* assert two integer values are not equal at runtime */
#define ASSERTne_rt(lhs, rhs) do { \
	if ((lhs) != (rhs)) break; \
	CORE_LOG_FATAL("assertion failure: %s (0x%llx) != %s (0x%llx)", #lhs,\
		(unsigned long long)(lhs), #rhs, (unsigned long long)(rhs)); \
} while (0)

/* assert a condition is true */
#define ASSERT(cnd)\
	do {\
		/*\
		 * Detect useless asserts on always true expression. Please use\
		 * COMPILE_ERROR_ON(!cnd) or ASSERT_rt(cnd) in such cases.\
		 */\
		TEST_ALWAYS_TRUE_EXPR(cnd);\
		ASSERT_rt(cnd);\
	} while (0)

/* assertion with extra info printed if assertion fails */
#define ASSERTinfo(cnd, info)\
	do {\
		/* See comment in ASSERT. */\
		TEST_ALWAYS_TRUE_EXPR(cnd);\
		ASSERTinfo_rt(cnd, info);\
	} while (0)

/* assert two integer values are equal */
#define ASSERTeq(lhs, rhs)\
	do {\
		/* See comment in ASSERT. */\
		TEST_ALWAYS_EQ_EXPR(lhs, rhs);\
		ASSERTeq_rt(lhs, rhs);\
	} while (0)

/* assert two integer values are not equal */
#define ASSERTne(lhs, rhs)\
	do {\
		/* See comment in ASSERT. */\
		TEST_ALWAYS_NE_EXPR(lhs, rhs);\
		ASSERTne_rt(lhs, rhs);\
	} while (0)
#else
#define ASSERT_rt(cnd)
#define ASSERTinfo_rt(cnd, info)
#define ASSERTeq_rt(lhs, rhs)
#define ASSERTne_rt(lhs, rhs)
#define ASSERT(cnd)
#define ASSERTinfo(cnd, info)
#define ASSERTeq(lhs, rhs)
#define ASSERTne(lhs, rhs)
#endif /* DEBUG */

#define ERR(use_errno, ...)\
	out_err(use_errno, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define ERR_W_ERRNO(f, ...)\
	CORE_LOG_ERROR_W_ERRNO_LAST(f, ##__VA_ARGS__)

#define ERR_WO_ERRNO(f, ...)\
	CORE_LOG_ERROR_LAST(f, ##__VA_ARGS__)

void out_init(const char *log_prefix, const char *log_level_var,
		const char *log_file_var, int major_version,
		int minor_version);
void out_fini(void);
void out(const char *fmt, ...) FORMAT_PRINTF(1, 2);
void out_nonl(int level, const char *fmt, ...) FORMAT_PRINTF(2, 3);
void out_log_va(const char *file, int line, const char *func, int level,
		const char *fmt, va_list ap);
void out_log(const char *file, int line, const char *func, int level,
	const char *fmt, ...) FORMAT_PRINTF(5, 6);
void out_err(int use_errno, const char *file, int line, const char *func,
	const char *fmt, ...) FORMAT_PRINTF(5, 6);
void NORETURN out_fatal(const char *file, int line, const char *func,
	const char *fmt, ...) FORMAT_PRINTF(4, 5);

#ifdef __cplusplus
}
#endif

#endif

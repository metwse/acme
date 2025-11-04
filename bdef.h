/**
 * @file bdef.h
 * @brief Basic definitions.
 *
 * Limits, basic types, and some other definitions
 */

#ifndef BDEF_H
#define BDEF_H

#include <stddef.h>


#define b_max(a, b) ((a) > (b) ? (a) : (b))
#define b_numbits(t) (sizeof(t) * CHAR_BIT)

/**
 * 'b_mem' is a signed integer big enough to count the total memory used by BS.
 * 'b_umem' is a corresponding unsigned type. Usually, 'ptrdiff_t' should work,
 * but we use 'long long' for 32-bit machines.
 */
typedef unsigned long long b_umem;
typedef long long b_mem;

/* chars used as small naturals (so that 'char' is reserved for characters) */
typedef unsigned char b_ubyte;
typedef char b_byte;

/**
 * Internal assertions for in-house debugging
 */
#ifdef B_ASSERT
#include <assert.h>
#include <stdio.h>  // IWYU pragma: export

#define bI_assert_stringify_detail(a) #a
#define bI_assert_stringify(a) bI_assert_stringify_detail(a)

#define b_assert_expr(c, fmt, ...) \
	if (!(c)) { \
		fprintf(stderr, "["__FILE__ ":" bI_assert_stringify(__LINE__) "] " fmt "\n"\
			__VA_OPT__(,)__VA_ARGS__); \
		assert(c); \
	}
#define b_assert(c) c
#else
#define b_assert_expr(c, ...) ((void) 0)
#define b_assert(c) ((void) 0)
#endif

/* type casts (a macro highlights casts in the code) */
#define cast(t, exp)	((t) (exp))


#endif

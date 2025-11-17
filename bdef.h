/**
 * @file bdef.h
 * @brief Basic definitions.
 *
 * Limits, basic types, and platform-abstraction definitions.
 */

#ifndef BDEF_H
#define BDEF_H

#include <stddef.h>
#include <assert.h>
#include <stdio.h>  // IWYU pragma: export
#include <signal.h>  // IWYU pragma: export


/** maximum of two nums */
#define b_max(a, b) ((a) > (b) ? (a) : (b))

/** type casts (a macro highlights casts in the code) */
#define cast(t, exp) ((t) (exp))


#define bI_assert_stringify_detail(a) #a
#define bI_assert_stringify(a) bI_assert_stringify_detail(a)

/** @brief internal assertions for in-house debugging */
#define b_assert_expr(c, fmt, ...) do { \
		if (!(c)) { \
			fprintf(stderr, "["__FILE__ ":" \
				bI_assert_stringify(__LINE__) "] " \
				"Assertion failed for: " \
				   bI_assert_stringify(c) \
				"\n> " fmt "\n" __VA_OPT__(,)__VA_ARGS__); \
			raise(2); \
		} \
	} while(0)

/** @brief code get into an unreachable branch */
#define b_unreachable() do { \
		fprintf(stderr, "reached unreachable branch"); assert(0); \
	} while (0)

/**
 * @brief Extra checks for flow of code.
 *
 * Used for program logic validation, i.e. no non-terminal have 0 child checked
 * using this.
 */
#define b_assert_logic(c, fmt, ...) \
	b_assert_expr(c, "logic error: " fmt " is/are not meaningful" \
		      __VA_OPT__(,)__VA_ARGS__)

/** @brief macro highlights memory allocation checks */
#define b_assert_mem(c) \
	b_assert_expr(c, "out of memory")

/** @brief macro highlights boundary checks */
#define b_assert_boundary(c, fmt, ...) \
	b_assert_expr(c, fmt __VA_OPT__(,)__VA_ARGS__)

/** @brief assertions for expensive checks */
#ifdef B_ASSERT
#define b_assert_agressive(c, fmt, ...) \
	b_assert_expr(c, fmt __VA_OPT__(,)__VA_ARGS__)
#else
#define b_assert_agressive(...) ((void) 0);
#endif


/** for suppressing unused function wargnings */
#ifdef __GNUC__
#define b_unused_fn(x) __attribute__((__unused__)) x
#else
#define b_unused_fn(x) x
#endif


/**
 * @brief An unsigned integer large enough to count all memory used by BS.
 *
 * 'b_umem' (unsigned long long) guarantees a 64-bit width, even on
 * 32-bit platforms where 'size_t' would only be 32-bit. This ensures
 * consistent memory counting and addressing capabilities.
 */
typedef unsigned long long b_umem;
/**
 * @brief 'b_umem' is the corresponding unsigned type to `b_mem`. Usually,
 * 'ptrdiff_t' should work, but we use 'long long' for 32-bit machines.
 */
typedef long long b_mem;

/**
 * @brief chars used as small naturals (so that 'char' is reserved for
 * characters)
 */
typedef unsigned char b_ubyte;
/** @brief 'b_ubyte' is the corresponding unsigned type to `b_byte`. */
typedef char b_byte;


#endif

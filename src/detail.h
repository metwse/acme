// internal macros of acme (taken from rdesc)

#ifndef DETAIL_HPP
#define DETAIL_HPP


#include <stdio.h> // IWYU pragma: begin_exports
#include <signal.h> // IWYU pragma: end_exports


#define assert_stringify_detail(a) #a
#define assert_stringify(a) assert_stringify_detail(a)

#define assert(c, fmt, ...) do { \
		if (!(c)) { \
			fprintf(stderr, "[" __FILE__ ":" \
				assert_stringify(__LINE__) "] " \
				"Assertion failed for: " \
				assert_stringify(c) \
				"\n> " fmt "\n" __VA_OPT__(,)__VA_ARGS__); \
			raise(SIGINT); \
		} \
	} while(0)


/* macro highlights memory allocation checks */
#define assert_mem(c) assert(c, "out of memory")

/* extra checks for flow of code. */
#define assert_logic(c, fmt, ...) \
	assert(c, "logic error: " fmt " is/are not meaningful" \
	       __VA_OPT__(,)__VA_ARGS__)

#if defined(__GNUC__) || defined(__clang__)
    #define unreachable() __builtin_unreachable()
#elif defined(_MSC_VER)
    #define unreachable() __assume(false)
#else
    #define unreachable()
#endif

#endif

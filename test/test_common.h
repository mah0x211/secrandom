#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include "../src/secrandom.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Platform detection
#if defined(_WIN32)
# define PLATFORM_NAME "Windows"
#elif defined(__APPLE__)
# define PLATFORM_NAME "macOS"
#elif defined(__linux__)
# define PLATFORM_NAME "Linux"
#elif defined(__FreeBSD__)
# define PLATFORM_NAME "FreeBSD"
#elif defined(__OpenBSD__)
# define PLATFORM_NAME "OpenBSD"
#elif defined(__NetBSD__)
# define PLATFORM_NAME "NetBSD"
#else
# define PLATFORM_NAME "Unknown"
#endif

// Test counters
static int total_tests  = 0;
static int passed_tests = 0;

// Test macros
#define TEST_START(name)                                                       \
    do {                                                                       \
        printf("Testing %s... ", name);                                        \
        fflush(stdout);                                                        \
        total_tests++;                                                         \
    } while (0)

#define TEST_PASS()                                                            \
    do {                                                                       \
        printf("PASS\n");                                                      \
        passed_tests++;                                                        \
    } while (0)

#define TEST_SKIP()                                                            \
    do {                                                                       \
        printf("SKIPPED (not supported on this platform)\n");                  \
        passed_tests++;                                                        \
    } while (0)

#define TEST_FAIL(...)                                                         \
    do {                                                                       \
        printf("FAIL\n");                                                      \
        fprintf(stderr, "  Error: ");                                          \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, "\n");                                                 \
    } while (0)

#define TEST_SECTION(name) printf("\n[%s]\n", name)

// Test summary
#define PRINT_TEST_SUMMARY()                                                   \
    do {                                                                       \
        printf("\n=== TEST SUMMARY ===\n");                                    \
        printf("Total: %d, Passed: %d, Failed: %d\n", total_tests,             \
               passed_tests, total_tests - passed_tests);                      \
        if (passed_tests == total_tests) {                                     \
            printf("All tests passed successfully!\n");                        \
        }                                                                      \
    } while (0)

// Common test utilities
static inline int is_buffer_all_zeros(const char *buf, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        if (buf[i] != 0) {
            return 0;
        }
    }
    return 1;
}

static inline int is_buffer_random(const char *buf, size_t len)
{
    // Check if buffer contains some non-zero bytes
    // (very unlikely to get all zeros from a random source)
    return !is_buffer_all_zeros(buf, len);
}

#endif /* TEST_COMMON_H */

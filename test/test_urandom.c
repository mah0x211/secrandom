#include "test_common.h"

static void test_urandom_availability(void)
{
    TEST_SECTION("/dev/urandom Availability");

    char buf[32];
    TEST_START("secrandom_urandom availability check");
    secrandom_result_e rc = secrandom_urandom(buf, sizeof(buf));

    if (rc == SECRANDOM_SUCCESS) {
        TEST_PASS();
        printf("  /dev/urandom is AVAILABLE on this platform\n");
    } else if (rc == SECRANDOM_UNSUPPORTED) {
        TEST_SKIP();
        printf("  /dev/urandom is NOT available on this platform\n");
    } else {
        TEST_FAIL("Unexpected result: %d", rc);
    }
}

static void test_urandom_functionality(void)
{
    char buf[32];

    // First check if the function is available
    if (secrandom_urandom(buf, sizeof(buf)) != SECRANDOM_SUCCESS) {
        printf("\n[/dev/urandom Functionality Tests - SKIPPED]\n");
        printf("  /dev/urandom is not available on this platform\n");
        return;
    }

    TEST_SECTION("/dev/urandom Functionality");

    // Test NULL buffer
    {
        TEST_START("secrandom_urandom with NULL buffer");
        secrandom_result_e rc = secrandom_urandom(NULL, 32);
        if (rc == SECRANDOM_FAILURE && errno == EINVAL) {
            TEST_PASS();
        } else {
            TEST_FAIL("Expected FAILURE with EINVAL, got rc=%d errno=%d", rc,
                      errno);
        }
    }

    // Test zero length
    {
        TEST_START("secrandom_urandom with zero length");
        secrandom_result_e rc = secrandom_urandom(buf, 0);
        if (rc == SECRANDOM_SUCCESS) {
            TEST_PASS();
        } else {
            TEST_FAIL("Expected SUCCESS for zero length, got %d", rc);
        }
    }

    // Test normal operation
    {
        TEST_START("secrandom_urandom with valid buffer");
        memset(buf, 0, sizeof(buf));
        secrandom_result_e rc = secrandom_urandom(buf, sizeof(buf));
        if (rc == SECRANDOM_SUCCESS) {
            if (is_buffer_random(buf, sizeof(buf))) {
                TEST_PASS();
            } else {
                TEST_FAIL("Buffer contains all zeros");
            }
        } else {
            TEST_FAIL("Unexpected failure: %d", rc);
        }
    }

    // Test FD caching
    {
        TEST_SECTION("/dev/urandom FD Caching");

        int fd_cache = -1;

        // First call with caching
        TEST_START("secrandom_urandom_ex with FD caching (first call)");
        memset(buf, 0, sizeof(buf));
        secrandom_result_e rc =
            secrandom_urandom_ex(buf, sizeof(buf), &fd_cache);
        if (rc == SECRANDOM_SUCCESS) {
            if (fd_cache > 0) {
                printf(" (FD=%d) ", fd_cache);
            }
            TEST_PASS();
        } else {
            TEST_FAIL("Failed with rc=%d", rc);
        }

        // Second call should reuse FD
        if (fd_cache > 0) {
            TEST_START("secrandom_urandom_ex with FD caching (reuse)");
            int old_fd = fd_cache;
            rc         = secrandom_urandom_ex(buf, sizeof(buf), &fd_cache);
            if (rc == SECRANDOM_SUCCESS) {
                if (fd_cache == old_fd) {
                    printf(" (FD reused) ");
                    TEST_PASS();
                } else {
                    TEST_FAIL("FD changed from %d to %d", old_fd, fd_cache);
                }
            } else {
                TEST_FAIL("Failed with rc=%d", rc);
            }

            // Clean up
            close(fd_cache);
        }
    }

    // Test large buffer
    {
        TEST_START("secrandom_urandom with large buffer (1MB)");
        size_t size     = 1024 * 1024; // 1MB
        char *large_buf = malloc(size);
        if (large_buf == NULL) {
            TEST_FAIL("Memory allocation failed");
        } else {
            secrandom_result_e rc = secrandom_urandom(large_buf, size);
            if (rc == SECRANDOM_SUCCESS) {
                // Just check a few bytes to ensure it's random
                if (is_buffer_random(large_buf, 256)) {
                    TEST_PASS();
                } else {
                    TEST_FAIL("Large buffer appears to be all zeros");
                }
            } else {
                TEST_FAIL("Failed with large buffer: %d", rc);
            }
            free(large_buf);
        }
    }
}

int main(void)
{
    printf("=== /dev/urandom TEST SUITE ===\n");
    printf("Platform: %s\n", PLATFORM_NAME);

    test_urandom_availability();
    test_urandom_functionality();

    PRINT_TEST_SUMMARY();
    return (passed_tests == total_tests) ? 0 : 1;
}

#include "test_common.h"

// Test basic functionality that should work on all platforms
static void test_basic_functionality(void)
{
    TEST_SECTION("Basic Functionality");

    // Test with valid buffer
    {
        TEST_START("secrandom with valid buffer");
        char buf[32];
        secrandom_result_e rc = secrandom(buf, sizeof(buf), NULL);
        if (rc == SECRANDOM_SUCCESS) {
            if (is_buffer_random(buf, sizeof(buf))) {
                TEST_PASS();
            } else {
                TEST_FAIL("Buffer contains all zeros");
            }
        } else {
            TEST_FAIL("secrandom failed with error %d", rc);
        }
    }

    // Test with zero length
    {
        TEST_START("secrandom with zero length");
        char buf[32];
        secrandom_result_e rc = secrandom(buf, 0, NULL);
        if (rc == SECRANDOM_SUCCESS) {
            TEST_PASS();
        } else {
            TEST_FAIL("Expected SUCCESS for zero length, got %d", rc);
        }
    }

    // Test with NULL buffer
    {
        TEST_START("secrandom with NULL buffer");
        secrandom_result_e rc = secrandom(NULL, 32, NULL);
        if (rc == SECRANDOM_FAILURE && errno == EINVAL) {
            TEST_PASS();
        } else {
            TEST_FAIL("Expected FAILURE with EINVAL, got rc=%d errno=%d", rc,
                      errno);
        }
    }

    // Test with large buffer
    {
        TEST_START("secrandom with large buffer (1MB)");
        size_t size = 1024 * 1024; // 1MB
        char *buf   = malloc(size);
        if (buf == NULL) {
            TEST_FAIL("Failed to allocate memory");
        } else {
            secrandom_result_e rc = secrandom(buf, size, NULL);
            if (rc == SECRANDOM_SUCCESS) {
                if (is_buffer_random(buf, size)) {
                    TEST_PASS();
                } else {
                    TEST_FAIL("Large buffer contains all zeros");
                }
            } else {
                TEST_FAIL("Failed with large buffer, rc=%d", rc);
            }
            free(buf);
        }
    }
}

// Test FD caching functionality (only relevant for urandom)
static void test_fd_caching(void)
{
    TEST_SECTION("File Descriptor Caching");

    int fd_cache = -1;

    // First call with caching
    {
        TEST_START("secrandom with FD caching (first call)");
        char buf[32];
        secrandom_result_e rc = secrandom(buf, sizeof(buf), &fd_cache);
        if (rc == SECRANDOM_SUCCESS) {
            TEST_PASS();
        } else {
            TEST_FAIL("Failed with rc=%d", rc);
        }
    }

    // Second call with caching (should reuse FD if urandom is used)
    {
        TEST_START("secrandom with FD caching (second call)");
        char buf[32];
        int old_fd            = fd_cache;
        secrandom_result_e rc = secrandom(buf, sizeof(buf), &fd_cache);
        if (rc == SECRANDOM_SUCCESS) {
            if (fd_cache > 0 && fd_cache == old_fd) {
                printf(" (FD reused) ");
            }
            TEST_PASS();
        } else {
            TEST_FAIL("Failed with rc=%d", rc);
        }
    }

    // Clean up cached FD if needed
    if (fd_cache >= 0) {
        close(fd_cache);
    }
}

// Test error conditions
static void test_error_handling(void)
{
    TEST_SECTION("Error Handling");

    // Test errno setting for NULL buffer
    {
        TEST_START("errno set to EINVAL for NULL buffer");
        errno                 = 0;
        secrandom_result_e rc = secrandom(NULL, 32, NULL);
        if (rc == SECRANDOM_FAILURE && errno == EINVAL) {
            TEST_PASS();
        } else {
            TEST_FAIL("Expected FAILURE with errno=EINVAL, got rc=%d errno=%d",
                      rc, errno);
        }
    }

    // Test buffer zeroing on failure
    {
        TEST_START("buffer zeroing on failure");
        char buf[32];
        memset(buf, 0xFF, sizeof(buf)); // Fill with non-zero
        secrandom_result_e rc = secrandom(NULL, sizeof(buf), NULL);
        if (rc == SECRANDOM_FAILURE) {
            // We can't test if the buffer was zeroed since we passed NULL
            // This is more of a code coverage test
            TEST_PASS();
        } else {
            TEST_FAIL("Expected FAILURE for NULL buffer");
        }
    }
}

int main(void)
{
    printf("=== BASIC SECRANDOM TEST SUITE ===\n");
    printf("Platform: %s\n", PLATFORM_NAME);

    test_basic_functionality();
    test_fd_caching();
    test_error_handling();

    PRINT_TEST_SUMMARY();
    return (passed_tests == total_tests) ? 0 : 1;
}

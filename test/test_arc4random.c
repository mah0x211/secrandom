#include "test_common.h"

static void test_arc4random_availability(void)
{
    TEST_SECTION("arc4random_buf Availability");

    char buf[32];
    TEST_START("secrandom_arc4random availability check");
    secrandom_result_e rc = secrandom_arc4random(buf, sizeof(buf));

    if (rc == SECRANDOM_SUCCESS) {
        TEST_PASS();
        printf("  arc4random_buf is AVAILABLE on this platform\n");
    } else if (rc == SECRANDOM_UNSUPPORTED) {
        TEST_SKIP();
        printf("  arc4random_buf is NOT available on this platform\n");
    } else {
        TEST_FAIL("Unexpected result: %d", rc);
    }
}

static void test_arc4random_functionality(void)
{
    char buf[32];

    // First check if the function is available
    if (secrandom_arc4random(buf, sizeof(buf)) != SECRANDOM_SUCCESS) {
        printf("\n[arc4random_buf Functionality Tests - SKIPPED]\n");
        printf("  arc4random_buf is not available on this platform\n");
        return;
    }

    TEST_SECTION("arc4random_buf Functionality");

    // Test NULL buffer
    {
        TEST_START("secrandom_arc4random with NULL buffer");
        secrandom_result_e rc = secrandom_arc4random(NULL, 32);
        if (rc == SECRANDOM_FAILURE && errno == EINVAL) {
            TEST_PASS();
        } else {
            TEST_FAIL("Expected FAILURE with EINVAL, got rc=%d errno=%d", rc,
                      errno);
        }
    }

    // Test zero length
    {
        TEST_START("secrandom_arc4random with zero length");
        secrandom_result_e rc = secrandom_arc4random(buf, 0);
        if (rc == SECRANDOM_SUCCESS) {
            TEST_PASS();
        } else {
            TEST_FAIL("Expected SUCCESS for zero length, got %d", rc);
        }
    }

    // Test normal operation
    {
        TEST_START("secrandom_arc4random with valid buffer");
        memset(buf, 0, sizeof(buf));
        secrandom_result_e rc = secrandom_arc4random(buf, sizeof(buf));
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

    // Test large buffer
    {
        TEST_START("secrandom_arc4random with large buffer (1MB)");
        size_t size     = 1024 * 1024; // 1MB
        char *large_buf = malloc(size);
        if (large_buf == NULL) {
            TEST_FAIL("Memory allocation failed");
        } else {
            secrandom_result_e rc = secrandom_arc4random(large_buf, size);
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
    printf("=== ARC4RANDOM TEST SUITE ===\n");
    printf("Platform: %s\n", PLATFORM_NAME);

    test_arc4random_availability();
    test_arc4random_functionality();

    PRINT_TEST_SUMMARY();
    return (passed_tests == total_tests) ? 0 : 1;
}

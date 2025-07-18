#include "test_common.h"

static void test_bcrypt_availability(void)
{
    TEST_SECTION("BCryptGenRandom Availability");

    char buf[32];
    TEST_START("secrandom_bcrypt availability check");
    secrandom_result_e rc = secrandom_bcrypt(buf, sizeof(buf));

    if (rc == SECRANDOM_SUCCESS) {
        TEST_PASS();
        printf("  BCryptGenRandom is AVAILABLE on this platform\n");
    } else if (rc == SECRANDOM_UNSUPPORTED) {
        TEST_SKIP();
        printf("  BCryptGenRandom is NOT available on this platform\n");
    } else {
        TEST_FAIL("Unexpected result: %d", rc);
    }
}

static void test_bcrypt_functionality(void)
{
    char buf[32];

    // First check if the function is available
    if (secrandom_bcrypt(buf, sizeof(buf)) != SECRANDOM_SUCCESS) {
        printf("\n[BCryptGenRandom Functionality Tests - SKIPPED]\n");
        printf("  BCryptGenRandom is not available on this platform\n");
        return;
    }

    TEST_SECTION("BCryptGenRandom Functionality");

    // Test NULL buffer
    {
        TEST_START("secrandom_bcrypt with NULL buffer");
        secrandom_result_e rc = secrandom_bcrypt(NULL, 32);
        if (rc == SECRANDOM_FAILURE && errno == EINVAL) {
            TEST_PASS();
        } else {
            TEST_FAIL("Expected FAILURE with EINVAL, got rc=%d errno=%d", rc,
                      errno);
        }
    }

    // Test zero length
    {
        TEST_START("secrandom_bcrypt with zero length");
        secrandom_result_e rc = secrandom_bcrypt(buf, 0);
        if (rc == SECRANDOM_SUCCESS) {
            TEST_PASS();
        } else {
            TEST_FAIL("Expected SUCCESS for zero length, got %d", rc);
        }
    }

    // Test normal operation
    {
        TEST_START("secrandom_bcrypt with valid buffer");
        memset(buf, 0, sizeof(buf));
        secrandom_result_e rc = secrandom_bcrypt(buf, sizeof(buf));
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

    // Test large buffer (tests chunking for ULONG_MAX)
    {
        TEST_START("secrandom_bcrypt with large buffer (1MB)");
        size_t size     = 1024 * 1024; // 1MB
        char *large_buf = malloc(size);
        if (large_buf == NULL) {
            TEST_FAIL("Memory allocation failed");
        } else {
            secrandom_result_e rc = secrandom_bcrypt(large_buf, size);
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
    printf("=== BCryptGenRandom TEST SUITE ===\n");
    printf("Platform: %s\n", PLATFORM_NAME);

    test_bcrypt_availability();
    test_bcrypt_functionality();

    PRINT_TEST_SUMMARY();
    return (passed_tests == total_tests) ? 0 : 1;
}

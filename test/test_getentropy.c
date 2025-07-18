#include "test_common.h"

static void test_getentropy_availability(void)
{
    TEST_SECTION("getentropy Availability");

    char buf[32];
    TEST_START("secrandom_getentropy availability check");
    secrandom_result_e rc = secrandom_getentropy(buf, sizeof(buf));

    if (rc == SECRANDOM_SUCCESS) {
        TEST_PASS();
        printf("  getentropy is AVAILABLE on this platform\n");
    } else if (rc == SECRANDOM_UNSUPPORTED) {
        TEST_SKIP();
        printf("  getentropy is NOT available on this platform\n");
    } else {
        TEST_FAIL("Unexpected result: %d", rc);
    }
}

static void test_getentropy_functionality(void)
{
    char buf[512];

    // First check if the function is available
    if (secrandom_getentropy(buf, 32) != SECRANDOM_SUCCESS) {
        printf("\n[getentropy Functionality Tests - SKIPPED]\n");
        printf("  getentropy is not available on this platform\n");
        return;
    }

    TEST_SECTION("getentropy Functionality");

    // Test NULL buffer
    {
        TEST_START("secrandom_getentropy with NULL buffer");
        secrandom_result_e rc = secrandom_getentropy(NULL, 32);
        if (rc == SECRANDOM_FAILURE && errno == EINVAL) {
            TEST_PASS();
        } else {
            TEST_FAIL("Expected FAILURE with EINVAL, got rc=%d errno=%d", rc,
                      errno);
        }
    }

    // Test zero length
    {
        TEST_START("secrandom_getentropy with zero length");
        secrandom_result_e rc = secrandom_getentropy(buf, 0);
        if (rc == SECRANDOM_SUCCESS) {
            TEST_PASS();
        } else {
            TEST_FAIL("Expected SUCCESS for zero length, got %d", rc);
        }
    }

    // Test normal operation (within 256 byte limit)
    {
        TEST_START("secrandom_getentropy with valid buffer (32 bytes)");
        memset(buf, 0, sizeof(buf));
        secrandom_result_e rc = secrandom_getentropy(buf, 32);
        if (rc == SECRANDOM_SUCCESS) {
            if (is_buffer_random(buf, 32)) {
                TEST_PASS();
            } else {
                TEST_FAIL("Buffer contains all zeros");
            }
        } else {
            TEST_FAIL("Unexpected failure: %d", rc);
        }
    }

    // Test exactly 256 bytes (getentropy limit)
    {
        TEST_START("secrandom_getentropy with 256 bytes (limit)");
        memset(buf, 0, sizeof(buf));
        secrandom_result_e rc = secrandom_getentropy(buf, 256);
        if (rc == SECRANDOM_SUCCESS) {
            if (is_buffer_random(buf, 256)) {
                TEST_PASS();
            } else {
                TEST_FAIL("Buffer contains all zeros");
            }
        } else {
            TEST_FAIL("Failed at exact limit: %d", rc);
        }
    }

    // Test more than 256 bytes (should be handled in chunks)
    {
        TEST_START("secrandom_getentropy with > 256 bytes");
        memset(buf, 0, sizeof(buf));
        secrandom_result_e rc = secrandom_getentropy(buf, 512);
        if (rc == SECRANDOM_SUCCESS) {
            if (is_buffer_random(buf, 512)) {
                TEST_PASS();
            } else {
                TEST_FAIL("Buffer contains all zeros");
            }
        } else {
            TEST_FAIL("Failed with > 256 bytes: %d", rc);
        }
    }

    // Test large buffer
    {
        TEST_START("secrandom_getentropy with large buffer (1MB)");
        size_t size     = 1024 * 1024; // 1MB
        char *large_buf = malloc(size);
        if (large_buf == NULL) {
            TEST_FAIL("Memory allocation failed");
        } else {
            secrandom_result_e rc = secrandom_getentropy(large_buf, size);
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
    printf("=== GETENTROPY TEST SUITE ===\n");
    printf("Platform: %s\n", PLATFORM_NAME);

    test_getentropy_availability();
    test_getentropy_functionality();

    PRINT_TEST_SUMMARY();
    return (passed_tests == total_tests) ? 0 : 1;
}

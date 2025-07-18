#include <openssl/opensslv.h>
#include "test_common.h"

static void test_ossl_availability(void)
{
    TEST_SECTION("OpenSSL Availability");

    char buf[32];
    TEST_START("secrandom_ossl availability check");
    secrandom_result_e rc = secrandom_ossl(buf, sizeof(buf));

    if (rc == SECRANDOM_SUCCESS) {
        TEST_PASS();
        printf("  OpenSSL is AVAILABLE on this platform\n");
#ifdef OPENSSL_VERSION_NUMBER
        printf("  OpenSSL version: 0x%08lx\n", OPENSSL_VERSION_NUMBER);
#endif
    } else if (rc == SECRANDOM_UNSUPPORTED) {
        TEST_SKIP();
        printf("  OpenSSL is NOT available on this platform\n");
    } else {
        TEST_FAIL("Unexpected result: %d", rc);
    }
}

static void test_ossl_functionality(void)
{
    char buf[32];

    // First check if the function is available
    if (secrandom_ossl(buf, sizeof(buf)) != SECRANDOM_SUCCESS) {
        printf("\n[OpenSSL Functionality Tests - SKIPPED]\n");
        printf("  OpenSSL is not available on this platform\n");
        return;
    }

    TEST_SECTION("OpenSSL Functionality");

    // Test NULL buffer
    {
        TEST_START("secrandom_ossl with NULL buffer");
        secrandom_result_e rc = secrandom_ossl(NULL, 32);
        if (rc == SECRANDOM_FAILURE && errno == EINVAL) {
            TEST_PASS();
        } else {
            TEST_FAIL("Expected FAILURE with EINVAL, got rc=%d errno=%d", rc,
                      errno);
        }
    }

    // Test zero length
    {
        TEST_START("secrandom_ossl with zero length");
        secrandom_result_e rc = secrandom_ossl(buf, 0);
        if (rc == SECRANDOM_SUCCESS) {
            TEST_PASS();
        } else {
            TEST_FAIL("Expected SUCCESS for zero length, got %d", rc);
        }
    }

    // Test normal operation
    {
        TEST_START("secrandom_ossl with valid buffer");
        memset(buf, 0, sizeof(buf));
        secrandom_result_e rc = secrandom_ossl(buf, sizeof(buf));
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

    // Test FIPS mode detection
    {
        TEST_START("secrandom_ossl_fips_enabled");
        int fips_enabled = secrandom_ossl_fips_enabled();
        printf("%s ", fips_enabled ? "(FIPS enabled)" : "(FIPS disabled)");
        // This is just informational, always pass
        TEST_PASS();
    }

    // Test large buffer (tests chunking for INT_MAX)
    {
        TEST_START("secrandom_ossl with large buffer (1MB)");
        size_t size     = 1024 * 1024; // 1MB
        char *large_buf = malloc(size);
        if (large_buf == NULL) {
            TEST_FAIL("Memory allocation failed");
        } else {
            secrandom_result_e rc = secrandom_ossl(large_buf, size);
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
    printf("=== OpenSSL TEST SUITE ===\n");
    printf("Platform: %s\n", PLATFORM_NAME);

    test_ossl_availability();
    test_ossl_functionality();

    PRINT_TEST_SUMMARY();
    return (passed_tests == total_tests) ? 0 : 1;
}

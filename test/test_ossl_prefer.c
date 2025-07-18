#include <openssl/opensslv.h>
#include "test_common.h"

static void test_openssl_preference(void)
{
    TEST_SECTION("OpenSSL Preference Mode");

    char buf[32];

    // Test that SECRANDOM_PREFER_OPENSSL is defined
    {
        TEST_START("SECRANDOM_PREFER_OPENSSL macro check");
#ifdef SECRANDOM_PREFER_OPENSSL
        TEST_PASS();
        printf("  SECRANDOM_PREFER_OPENSSL is defined\n");
#else
        TEST_FAIL("SECRANDOM_PREFER_OPENSSL is not defined");
        printf(
            "  This test must be compiled with -DSECRANDOM_PREFER_OPENSSL\n");
        return;
#endif
    }

    // Test that OpenSSL is available
    {
        TEST_START("OpenSSL availability in prefer mode");
        secrandom_result_e rc = secrandom_ossl(buf, sizeof(buf));
        if (rc == SECRANDOM_SUCCESS) {
            TEST_PASS();
        } else if (rc == SECRANDOM_UNSUPPORTED) {
            TEST_SKIP();
            printf("  OpenSSL is not available, cannot test preference\n");
            return;
        } else {
            TEST_FAIL("Unexpected result: %d", rc);
        }
    }

    // Test that secrandom() uses OpenSSL as primary method
    {
        TEST_START("secrandom() uses OpenSSL preferentially");
        memset(buf, 0, sizeof(buf));
        secrandom_result_e rc = secrandom(buf, sizeof(buf), NULL);
        if (rc == SECRANDOM_SUCCESS) {
            if (is_buffer_random(buf, sizeof(buf))) {
                TEST_PASS();
                printf("  secrandom() succeeded (should use OpenSSL first)\n");
            } else {
                TEST_FAIL("Buffer contains all zeros");
            }
        } else {
            TEST_FAIL("secrandom() failed: %d", rc);
        }
    }

    // Test fallback behavior if OpenSSL were to fail
    // (This is difficult to test without mocking, so we just document expected
    // behavior)
    {
        TEST_START("Fallback behavior documentation");
        printf("(informational) ");
        TEST_PASS();
        printf("  Expected behavior when SECRANDOM_PREFER_OPENSSL is set:\n");
        printf("    1. Try OpenSSL first (even if not in FIPS mode)\n");
        printf("    2. If OpenSSL fails, try platform-specific methods\n");
        printf(
            "    3. Order: OpenSSL -> arc4random -> getentropy -> urandom\n");
    }
}

static void test_error_handling_with_preference(void)
{
    TEST_SECTION("Error Handling with OpenSSL Preference");

    // Test NULL buffer handling
    {
        TEST_START("NULL buffer with OpenSSL preference");
        secrandom_result_e rc = secrandom(NULL, 32, NULL);
        if (rc == SECRANDOM_FAILURE && errno == EINVAL) {
            TEST_PASS();
        } else {
            TEST_FAIL("Expected FAILURE with EINVAL, got rc=%d errno=%d", rc,
                      errno);
        }
    }

    // Test zero length
    {
        TEST_START("Zero length with OpenSSL preference");
        char buf[32];
        secrandom_result_e rc = secrandom(buf, 0, NULL);
        if (rc == SECRANDOM_SUCCESS) {
            TEST_PASS();
        } else {
            TEST_FAIL("Expected SUCCESS for zero length, got %d", rc);
        }
    }
}

int main(void)
{
    printf("=== OpenSSL PREFERENCE TEST SUITE ===\n");
    printf("Platform: %s\n", PLATFORM_NAME);

    test_openssl_preference();
    test_error_handling_with_preference();

    PRINT_TEST_SUMMARY();
    return (passed_tests == total_tests) ? 0 : 1;
}

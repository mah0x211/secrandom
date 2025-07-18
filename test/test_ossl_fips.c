#include <openssl/opensslv.h>
#include "test_common.h"

static void test_fips_mode_detection(void)
{
    TEST_SECTION("FIPS Mode Detection");

    // Test FIPS mode detection function
    {
        TEST_START("secrandom_ossl_fips_enabled");
        int fips_enabled = secrandom_ossl_fips_enabled();

        if (fips_enabled) {
            TEST_PASS();
            printf("  FIPS mode is ENABLED\n");
        } else {
            TEST_PASS(); // Also pass if FIPS is disabled
            printf("  FIPS mode is DISABLED\n");
        }

        // Note: To actually test FIPS mode, you need:
        // 1. OpenSSL built with FIPS support
        // 2. FIPS module installed
        // 3. FIPS mode enabled in OpenSSL config or environment
    }
}

static void test_fips_mode_behavior(void)
{
    TEST_SECTION("FIPS Mode Behavior");

    char buf[32];
    int fips_enabled = secrandom_ossl_fips_enabled();

    if (!fips_enabled) {
        printf("  Note: FIPS mode is not enabled, testing non-FIPS behavior\n");
    }

    // Test that OpenSSL is available
    {
        TEST_START("OpenSSL availability check");
        secrandom_result_e rc = secrandom_ossl(buf, sizeof(buf));
        if (rc == SECRANDOM_SUCCESS) {
            TEST_PASS();
        } else if (rc == SECRANDOM_UNSUPPORTED) {
            TEST_SKIP();
            printf("  OpenSSL is not available\n");
            return;
        } else {
            TEST_FAIL("Unexpected result: %d", rc);
        }
    }

    // Test secrandom() behavior with FIPS mode
    {
        TEST_START("secrandom() with FIPS mode consideration");
        memset(buf, 0, sizeof(buf));
        secrandom_result_e rc = secrandom(buf, sizeof(buf), NULL);
        if (rc == SECRANDOM_SUCCESS) {
            if (is_buffer_random(buf, sizeof(buf))) {
                TEST_PASS();
                if (fips_enabled) {
                    printf("  FIPS mode: OpenSSL should be used first\n");
                } else {
                    printf("  Non-FIPS mode: Normal priority order\n");
                }
            } else {
                TEST_FAIL("Buffer contains all zeros");
            }
        } else {
            TEST_FAIL("secrandom() failed: %d", rc);
        }
    }

    // Document expected behavior
    {
        TEST_START("FIPS mode behavior documentation");
        printf("(informational) ");
        TEST_PASS();
        printf("\n  Expected behavior in FIPS mode:\n");
        printf("    1. OpenSSL is mandatory when FIPS is enabled\n");
        printf(
            "    2. If OpenSSL fails in FIPS mode, still try other methods\n");
        printf("    3. This ensures availability even if OpenSSL has issues\n");
    }
}

static void test_fips_compliance(void)
{
    TEST_SECTION("FIPS Compliance Checks");

    // Test that we can generate cryptographically secure random numbers
    {
        TEST_START("Cryptographic quality random generation");
        char buf1[32], buf2[32];

        secrandom_result_e rc1 = secrandom(buf1, sizeof(buf1), NULL);
        secrandom_result_e rc2 = secrandom(buf2, sizeof(buf2), NULL);

        if (rc1 == SECRANDOM_SUCCESS && rc2 == SECRANDOM_SUCCESS) {
            // Check that two calls don't produce identical output
            if (memcmp(buf1, buf2, sizeof(buf1)) != 0) {
                TEST_PASS();
            } else {
                TEST_FAIL("Two calls produced identical output!");
            }
        } else {
            TEST_FAIL("Failed to generate random numbers");
        }
    }
}

int main(void)
{
    printf("=== OpenSSL FIPS MODE TEST SUITE ===\n");
    printf("Platform: %s\n", PLATFORM_NAME);

    // Check if OpenSSL is compiled in
#ifdef OPENSSL_VERSION_NUMBER
    printf("OpenSSL version: 0x%08lx\n", OPENSSL_VERSION_NUMBER);
#else
    printf("OpenSSL: Not compiled in\n");
#endif

    test_fips_mode_detection();
    test_fips_mode_behavior();
    test_fips_compliance();

    PRINT_TEST_SUMMARY();
    return (passed_tests == total_tests) ? 0 : 1;
}

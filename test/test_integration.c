#include <openssl/opensslv.h>
#include "test_common.h"

static void test_secrandom_automatic_selection(void)
{
    TEST_SECTION("Automatic Method Selection");

    char buf[32];

    // Test that secrandom() automatically selects an available method
    {
        TEST_START("secrandom() automatic selection");
        memset(buf, 0, sizeof(buf));
        secrandom_result_e rc = secrandom(buf, sizeof(buf), NULL);
        if (rc == SECRANDOM_SUCCESS) {
            if (is_buffer_random(buf, sizeof(buf))) {
                TEST_PASS();

                // Try to determine which method was used
                printf("  Available methods on this platform:\n");
                if (secrandom_ossl(buf, 1) == SECRANDOM_SUCCESS) {
                    printf("    - OpenSSL\n");
                }
                if (secrandom_arc4random(buf, 1) == SECRANDOM_SUCCESS) {
                    printf("    - arc4random_buf\n");
                }
                if (secrandom_getentropy(buf, 1) == SECRANDOM_SUCCESS) {
                    printf("    - getentropy\n");
                }
                if (secrandom_urandom(buf, 1) == SECRANDOM_SUCCESS) {
                    printf("    - /dev/urandom\n");
                }
                if (secrandom_bcrypt(buf, 1) == SECRANDOM_SUCCESS) {
                    printf("    - BCryptGenRandom\n");
                }
            } else {
                TEST_FAIL("Buffer contains all zeros");
            }
        } else {
            TEST_FAIL("secrandom() failed: %d", rc);
        }
    }
}

static void test_error_consistency(void)
{
    TEST_SECTION("Error Handling Consistency");

    // Test that all methods handle NULL buffer consistently
    {
        TEST_START("NULL buffer handling across all methods");
        int consistent = 1;

        // Test each method
        secrandom_result_e rc;

        rc = secrandom_ossl(NULL, 32);
        if (rc != SECRANDOM_FAILURE && rc != SECRANDOM_UNSUPPORTED) {
            consistent = 0;
        }

        rc = secrandom_arc4random(NULL, 32);
        if (rc != SECRANDOM_FAILURE && rc != SECRANDOM_UNSUPPORTED) {
            consistent = 0;
        }

        rc = secrandom_getentropy(NULL, 32);
        if (rc != SECRANDOM_FAILURE && rc != SECRANDOM_UNSUPPORTED) {
            consistent = 0;
        }

        rc = secrandom_urandom(NULL, 32);
        if (rc != SECRANDOM_FAILURE && rc != SECRANDOM_UNSUPPORTED) {
            consistent = 0;
        }

        rc = secrandom_bcrypt(NULL, 32);
        if (rc != SECRANDOM_FAILURE && rc != SECRANDOM_UNSUPPORTED) {
            consistent = 0;
        }

        if (consistent) {
            TEST_PASS();
        } else {
            TEST_FAIL("Inconsistent NULL buffer handling");
        }
    }

    // Test errno setting
    {
        TEST_START("errno setting for secrandom() with NULL buffer");
        errno                 = 0;
        secrandom_result_e rc = secrandom(NULL, 32, NULL);
        if (rc == SECRANDOM_FAILURE && errno == EINVAL) {
            TEST_PASS();
        } else {
            TEST_FAIL("Expected FAILURE with errno=EINVAL, got rc=%d errno=%d",
                      rc, errno);
        }
    }
}

static void test_performance_characteristics(void)
{
    TEST_SECTION("Performance Characteristics");

    // This is more of an informational test
    {
        TEST_START("Relative performance of different methods");

        char buf[4096];

        printf("\n");

        // Test each available method
        if (secrandom_ossl(buf, sizeof(buf)) == SECRANDOM_SUCCESS) {
            printf("    OpenSSL: available\n");
        }

        if (secrandom_arc4random(buf, sizeof(buf)) == SECRANDOM_SUCCESS) {
            printf("    arc4random_buf: available\n");
        }

        if (secrandom_getentropy(buf, sizeof(buf)) == SECRANDOM_SUCCESS) {
            printf("    getentropy: available\n");
        }

        if (secrandom_urandom(buf, sizeof(buf)) == SECRANDOM_SUCCESS) {
            printf("    /dev/urandom: available\n");
        }

        if (secrandom_bcrypt(buf, sizeof(buf)) == SECRANDOM_SUCCESS) {
            printf("    BCryptGenRandom: available\n");
        }

        TEST_PASS(); // Always pass, this is informational
    }
}

int main(void)
{
    printf("=== INTEGRATION TEST SUITE ===\n");
    printf("Platform: %s\n", PLATFORM_NAME);

    test_secrandom_automatic_selection();
    test_error_consistency();
    test_performance_characteristics();

    PRINT_TEST_SUMMARY();
    return (passed_tests == total_tests) ? 0 : 1;
}

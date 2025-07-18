# secrandom

[![test](https://github.com/mah0x211/secrandom/actions/workflows/test.yml/badge.svg)](https://github.com/mah0x211/secrandom/actions/workflows/test.yml)
[![codecov](https://codecov.io/gh/mah0x211/secrandom/branch/master/graph/badge.svg)](https://codecov.io/gh/mah0x211/secrandom)

A header-only C library for generating secure random bytes across multiple platforms. It provides a unified API that automatically selects the best available method for secure random number generation, including OpenSSL, `arc4random_buf`, `getentropy` and `/dev/urandom`.


## Overview

The secrandom library provides a unified interface for secure random number generation across different platforms. It automatically detects and utilizes the best available cryptographic random number generator, ensuring your application gets high-quality random bytes regardless of the target platform.


## Features

- Header-only implementation (no compilation required)
- Cross-platform support (Linux, macOS)
- Automatic method selection based on platform capabilities
- Support for `OpenSSL`, `arc4random_buf`, `getentropy`, and `/dev/urandom`
- File descriptor caching for improved performance
- FIPS mode support when using OpenSSL
- Comprehensive error handling with proper errno setting
- MIT licensed


## Installation

Simply copy the `secrandom.h` file to your project directory, and include it in your source files.

```c
#include "secrandom.h"
```

**Note:** If you are using OpenSSL, ensure that the OpenSSL headers are included before `secrandom.h` to enable OpenSSL-specific functionality.

```c
#include <openssl/opensslv.h> // Include OpenSSL headers first
#include "secrandom.h"
```


## Usage

```c
#include <stdio.h>
#include <string.h>
#include "secrandom.h"

int main() {
    char buf[64];
    size_t len = sizeof(buf);
    secrandom_result_e rc = secrandom(buf, len, NULL);
    
    switch(rc){
        case SECRANDOM_SUCCESS:
            printf("Generated random bytes\n");
            break;
        case SECRANDOM_FAILURE:
            fprintf(stderr, "Failed to generate random bytes\n");
            return 1;
        case SECRANDOM_UNSUPPORTED:
            fprintf(stderr, "Random generation not supported on this platform\n");
            return 1;
    }

    // Print the generated random bytes as a hex string
    for (size_t i = 0; i < len; i++) {
        printf("%02x", (unsigned char)buf[i]);
    }
    printf("\n");
    
    return 0;
}
```

## API

```c
/**
 * @brief Generate secure random bytes using OpenSSL.
 *
 * @param buf Pointer to the buffer where random bytes will be stored.
 * @param len Number of bytes to generate.
 * @return secrandom_result_e Returns SECRANDOM_SUCCESS on success, or 
 * SECRANDOM_FAILURE on failure, or SECRANDOM_UNSUPPORTED if the platform does 
 * not support OpenSSL.
 */
static inline secrandom_result_e secrandom_ossl(void *buf, size_t len);

/**
 * @brief Generate secure random bytes using arc4random_buf.
 *
 * @param buf Pointer to the buffer where random bytes will be stored.
 * @param len Number of bytes to generate.
 * @return secrandom_result_e Returns SECRANDOM_SUCCESS on success, or 
 * SECRANDOM_UNSUPPORTED if the platform does not support arc4random_buf().
 */
static inline secrandom_result_e secrandom_arc4random(void *buf, size_t len);

/**
 * @brief Generate secure random bytes using getentropy.
 *
 * @param buf Pointer to the buffer where random bytes will be stored.
 * @param len Number of bytes to generate.
 * @return secrandom_result_e Returns SECRANDOM_SUCCESS on success, or 
 * SECRANDOM_FAILURE on failure, or SECRANDOM_UNSUPPORTED if the platform does 
 * not support getentropy().
 */
static inline secrandom_result_e secrandom_getentropy(void *buf, size_t len);


/**
 * @brief Generate secure random bytes using /dev/urandom with caching.
 *
 * @param buf Pointer to the buffer where random bytes will be stored.
 * @param len Number of bytes to generate.
 * @param fd_urandom Pointer to an integer for caching file descriptor for
 * /dev/urandom.
 * @return secrandom_result_e Returns SECRANDOM_SUCCESS on success, or 
 * SECRANDOM_FAILURE on failure, or SECRANDOM_UNSUPPORTED if the platform does
 * not support /dev/urandom.
 */
static inline secrandom_result_e secrandom_urandom_ex(void *buf, size_t len,
                                                      int *fd_urandom);

#define secrandom_urandom(buf, len) secrandom_urandom_ex(buf, len, NULL)


/**
 * @brief Generate secure random bytes.
 *
 * @param buf Pointer to the buffer where random bytes will be stored.
 * @param len Number of bytes to generate.
 * @param fd_urandom Pointer to an integer for caching file descriptor for
 * /dev/urandom (if applicable).
 * @return secrandom_result_e Returns SECRANDOM_SUCCESS on success,
 * or SECRANDOM_FAILURE on failure, or SECRANDOM_UNSUPPORTED if
 * random generation is not supported on this platform.
 */
static inline secrandom_result_e secrandom(void *buf, size_t len, int *fd_cached);
```

You can use the `secrandom` function to generate secure random bytes. It will automatically choose the best available method based on the platform and compile-time options.

If an OpenSSL header has been included prior to `secrandom.h`—meaning the `OPENSSL_VERSION_NUMBER` macro is already defined—the `secrandom_ossl()` API is enabled.

If FIPS mode is enabled or the `SECRANDOM_PREFER_OPENSSL` macro is defined, `secrandom_ossl()` will be called first. If it succeeds, `SECRANDOM_SUCCESS` will be returned.

If it does not succeed, the following APIs will be called in order, and if any succeeds, `SECRANDOM_SUCCESS` will be returned:

1. `secrandom_arc4random()`
2. `secrandom_getentropy()`
3. `secrandom_urandom_ex()`
4. If it has not yet been called, `secrandom_ossl()`

If all of the above fail in the following ways in the code:

- `SECRANDOM_UNSUPPORTED`: `errno` is set to `ENOSYS` (Function not implemented).
- `SECRANDOM_FAILURE`: `errno` is set to `EIO` (Input/output error).

And, return the corresponding error code of `secrandom_result_e`.


## Testing

The project includes comprehensive test suites:

```bash
# Run basic tests (no OpenSSL required)
make test

# Run OpenSSL-specific tests
make test-openssl

# Clean test binaries
make clean
```

Test coverage includes:

- Basic functionality tests
- Platform-specific method tests (`arc4random`, `getentropy`, `/dev/urandom`)
- OpenSSL integration tests
- Error handling and edge cases
- FIPS mode detection
- Performance characteristics


## License

MIT License - Copyright (C) 2025 Masatoshi Fukunaga


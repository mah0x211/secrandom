/**
 *  Copyright (C) 2025 Masatoshi Fukunaga
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 */

#ifndef secrandom_h
#define secrandom_h

#include <errno.h>
#include <limits.h>
#include <stddef.h>

typedef enum secrandom_result_e {
    SECRANDOM_SUCCESS     = 0,
    SECRANDOM_FAILURE     = -1,
    SECRANDOM_UNSUPPORTED = -2
} secrandom_result_e;

#if defined(OPENSSL_VERSION_NUMBER)
# include <openssl/crypto.h>
# include <openssl/rand.h>
# if OPENSSL_VERSION_NUMBER >= 0x30000000L
#  include <openssl/evp.h>
# endif

/**
 * @brief Generate secure random bytes using OpenSSL.
 *
 * @param buf Pointer to the buffer where random bytes will be stored.
 * @param len Number of bytes to generate.
 * @return secrandom_result_e Returns SECRANDOM_SUCCESS on success,
 * or SECRANDOM_FAILURE on failure.
 */
static inline secrandom_result_e secrandom_ossl(void *buf, size_t len)
{
    unsigned char *p = buf;
    size_t left      = len;

    if (buf == NULL) {
        errno = EINVAL;
        return SECRANDOM_FAILURE;
    } else if (len == 0) {
        return SECRANDOM_SUCCESS;
    }

    /* RAND_bytes accepts int, so we need to chunk large requests */
    while (left > 0) {
        int chunk = (left > INT_MAX) ? INT_MAX : (int)left;
        if (RAND_bytes(p, chunk) != 1) {
            return SECRANDOM_FAILURE;
        }
        p += chunk;
        left -= (size_t)chunk;
    }
    return SECRANDOM_SUCCESS;
}

/**
 * @brief Check if OpenSSL is in FIPS mode.
 *
 * @return int Returns 1 if FIPS mode is enabled, 0 otherwise.
 */
static inline int secrandom_ossl_fips_enabled(void)
{
# if OPENSSL_VERSION_NUMBER >= 0x30000000L
    return EVP_default_properties_is_fips_enabled(NULL);
# elif defined(FIPS_mode)
    return FIPS_mode();
# else
    return 0;
# endif
}

#else  /* OpenSSL not available */

/**
 * @brief Generate secure random bytes using OpenSSL.
 *
 * @param buf Pointer to the buffer where random bytes will be stored.
 * @param len Number of bytes to generate.
 * @return secrandom_result_e Returns SECRANDOM_UNSUPPORTED as
 * OpenSSL is not available.
 */
static inline secrandom_result_e secrandom_ossl(void *buf, size_t len)
{
    (void)buf;
    (void)len;
    return SECRANDOM_UNSUPPORTED;
}

/**
 * @brief Check if OpenSSL is in FIPS mode.
 *
 * @return int Returns 0 as OpenSSL is not available.
 */
static inline int secrandom_ossl_fips_enabled(void)
{
    return 0;
}
#endif /* OPENSSL */

#if defined(__GLIBC__)
# include <features.h>
# define SECRANDOM_GLIBC_PREREQ(...) __GLIBC_PREREQ(__VA_ARGS__)
#else
# define SECRANDOM_GLIBC_PREREQ(...) 0
#endif

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||      \
    defined(__NetBSD__) /* arc4random_buf (*BSD / macOS) */

# include <stdlib.h>

/**
 * @brief Generate secure random bytes using arc4random_buf.
 *
 * @param buf Pointer to the buffer where random bytes will be stored.
 * @param len Number of bytes to generate.
 * @return secrandom_result_e Returns SECRANDOM_SUCCESS on success.
 */
static inline secrandom_result_e secrandom_arc4random(void *buf, size_t len)
{
    if (buf == NULL) {
        errno = EINVAL;
        return SECRANDOM_FAILURE;
    } else if (len == 0) {
        return SECRANDOM_SUCCESS;
    }
    arc4random_buf(buf, len);
    return SECRANDOM_SUCCESS;
}

#else /* arc4random_buf */

/**
 * @brief Generate secure random bytes using arc4random_buf.
 *
 * @param buf Pointer to the buffer where random bytes will be stored.
 * @param len Number of bytes to generate.
 * @return secrandom_result_e Returns SECRANDOM_UNSUPPORTED as
 * arc4random_buf is not available.
 */
static inline secrandom_result_e secrandom_arc4random(void *buf, size_t len)
{
    (void)buf;
    (void)len;
    return SECRANDOM_UNSUPPORTED;
}

#endif /* arc4random_buf */

#if (defined(__linux__) && defined(__GLIBC__) &&                               \
     SECRANDOM_GLIBC_PREREQ(2, 25)) ||                                         \
    defined(__OpenBSD__) ||                                                    \
    (defined(__FreeBSD__) && __FreeBSD_version >= 1200000) ||                  \
    (defined(__NetBSD__) && __NetBSD_Version__ >= 1000000000)
/* getentropy (glibc 2.25+ / OpenBSD 5.6+ / FreeBSD 12+ / NetBSD 10+) */

# include <sys/random.h>

/**
 * @brief Generate secure random bytes using getentropy.
 *
 * @param buf Pointer to the buffer where random bytes will be stored.
 * @param len Number of bytes to generate.
 * @return secrandom_result_e Returns SECRANDOM_SUCCESS on success,
 * or SECRANDOM_FAILURE on failure.
 */
static inline secrandom_result_e secrandom_getentropy(void *buf, size_t len)
{
    unsigned char *p = buf;
    size_t left      = len;

    if (buf == NULL) {
        errno = EINVAL;
        return SECRANDOM_FAILURE;
    } else if (len == 0) {
        return SECRANDOM_SUCCESS;
    }

    while (left > 0) {
        size_t chunk = left > 256 ? 256 : left;
        if (getentropy(p, chunk) != 0) {
            return SECRANDOM_FAILURE;
        }
        p += chunk;
        left -= chunk;
    }
    return SECRANDOM_SUCCESS;
}

#else /* getentropy */

/**
 * @brief Generate secure random bytes using getentropy.
 *
 * @param buf Pointer to the buffer where random bytes will be stored.
 * @param len Number of bytes to generate.
 * @return secrandom_result_e Returns SECRANDOM_UNSUPPORTED as
 * getentropy is not available.
 */
static inline secrandom_result_e secrandom_getentropy(void *buf, size_t len)
{
    (void)buf;
    (void)len;
    return SECRANDOM_UNSUPPORTED;
}

#endif /* getentropy */

#if defined(_WIN32) /* Windows (BCryptGenRandom) */

/**
 * @brief Generate secure random bytes using /dev/urandom on Windows.
 *
 * @param buf Pointer to the buffer where random bytes will be stored.
 * @param len Number of bytes to generate.
 * @param fd_urandom Pointer to an integer for caching file descriptor for
 * /dev/urandom.
 * @return secrandom_result_e Returns SECRANDOM_UNSUPPORTED as
 * /dev/urandom is not available on Windows.
 */
static inline secrandom_result_e secrandom_urandom_ex(void *buf, size_t len,
                                                      int *fd_urandom)
{
    (void)buf;
    (void)len;
    (void)fd_urandom;
    return SECRANDOM_UNSUPPORTED;
}

/**
 * @brief Generate secure random bytes using /dev/urandom on Windows.
 *
 * @param buf Pointer to the buffer where random bytes will be stored.
 * @param len Number of bytes to generate.
 * @return secrandom_result_e Returns SECRANDOM_UNSUPPORTED as
 * /dev/urandom is not available on Windows.
 */
# define secrandom_urandom(buf, len) secrandom_urandom_ex(buf, len, 0)

# define WIN32_LEAN_AND_MEAN
# include <bcrypt.h>
# include <windows.h>
# ifdef _MSC_VER
#  pragma comment(lib, "bcrypt")
# endif

/**
 * @brief Generate secure random bytes using BCryptGenRandom.
 *
 * @param buf Pointer to the buffer where random bytes will be stored.
 * @param len Number of bytes to generate.
 * @return secrandom_result_e Returns SECRANDOM_SUCCESS on success,
 * or SECRANDOM_FAILURE on failure.
 */
static inline secrandom_result_e secrandom_bcrypt(void *buf, size_t len)
{
    unsigned char *p = buf;
    size_t left      = len;

    if (buf == NULL) {
        errno = EINVAL;
        return SECRANDOM_FAILURE;
    } else if (len == 0) {
        return SECRANDOM_SUCCESS;
    }

    /* BCryptGenRandom accepts ULONG, so we need to chunk large requests */
    while (left > 0) {
        ULONG chunk = (left > ULONG_MAX) ? ULONG_MAX : (ULONG)left;
        if (BCryptGenRandom(NULL, p, chunk, BCRYPT_USE_SYSTEM_PREFERRED_RNG) !=
            0) {
            return SECRANDOM_FAILURE;
        }
        p += chunk;
        left -= (size_t)chunk;
    }
    return SECRANDOM_SUCCESS;
}

#else /* POSIX */

/* Feature test macros for O_CLOEXEC */
# ifndef _POSIX_C_SOURCE
#  define _POSIX_C_SOURCE 200809L
# endif
# ifndef _XOPEN_SOURCE
#  define _XOPEN_SOURCE 700
# endif

# include <fcntl.h>
# include <unistd.h>

/* O_CLOEXEC fallback for very old systems */
# ifndef O_CLOEXEC
#  define O_CLOEXEC 0
# endif

/**
 * @brief Generate secure random bytes using /dev/urandom with caching.
 *
 * @param buf Pointer to the buffer where random bytes will be stored.
 * @param len Number of bytes to generate.
 * @param fd_urandom Pointer to an integer for caching file descriptor for
 * /dev/urandom.
 * @return secrandom_result_e Returns SECRANDOM_SUCCESS on success,
 * or SECRANDOM_FAILURE on failure.
 */
static inline secrandom_result_e secrandom_urandom_ex(void *buf, size_t len,
                                                      int *fd_urandom)
{
    unsigned char *p = buf;
    size_t left      = len;
    int fd           = -1;

    if (buf == NULL) {
        errno = EINVAL;
        return SECRANDOM_FAILURE;
    } else if (len == 0) {
        return SECRANDOM_SUCCESS;
    }

    if (fd_urandom && *fd_urandom >= 0) {
        fd = *fd_urandom;
    } else if ((fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC)) < 0) {
        return SECRANDOM_FAILURE;
    }
# if O_CLOEXEC == 0
    /* Set FD_CLOEXEC if O_CLOEXEC wasn't available */
    else {
        int flags = fcntl(fd, F_GETFD);
        if (flags >= 0) {
            fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
        }
    }
# endif

    /* fill the buffer with random bytes */
    while (left > 0) {
        ssize_t n = read(fd, p, left);
        if (n < 0) {
            /* read error */
            if (errno != EINTR) {
                close(fd);
                if (fd_urandom) {
                    *fd_urandom = -1;
                }
                return SECRANDOM_FAILURE;
            }
            /* EINTR - retry */
            continue;
        } else if (n == 0) {
            /* EOF - should not happen with /dev/urandom */
            close(fd);
            if (fd_urandom) {
                *fd_urandom = -1;
            }
            errno = EIO;
            return SECRANDOM_FAILURE;
        }
        p += (size_t)n;
        left -= (size_t)n;
    }

    /* close fd if not using cached mode */
    if (fd_urandom) {
        *fd_urandom = fd;
    } else {
        close(fd);
    }
    return SECRANDOM_SUCCESS;
}

# define secrandom_urandom(buf, len) secrandom_urandom_ex(buf, len, 0)

/**
 * @brief Generate secure random bytes using BCryptGenRandom.
 *
 * @param buf Pointer to the buffer where random bytes will be stored.
 * @param len Number of bytes to generate.
 * @return secrandom_result_e Returns SECRANDOM_UNSUPPORTED as
 * BCryptGenRandom is not available on this platform.
 */
static inline secrandom_result_e secrandom_bcrypt(void *buf, size_t len)
{
    (void)buf;
    (void)len;
    errno = ENOSYS; /* Function not implemented */
    return SECRANDOM_UNSUPPORTED;
}

#endif /* _POSIX */

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
static inline secrandom_result_e secrandom(void *buf, size_t len,
                                           int *fd_urandom)
{
    secrandom_result_e rc = SECRANDOM_SUCCESS;

    if (buf == NULL) {
        errno = EINVAL;
        return SECRANDOM_FAILURE;
    } else if (len == 0) {
        return SECRANDOM_SUCCESS;
    }

#if defined(_WIN32)
    /* On Windows, uses only BCryptGenRandom */
    (void)fd_urandom; /* Unused parameter on Windows */
    if ((rc = secrandom_bcrypt(buf, len)) == SECRANDOM_SUCCESS) {
        return SECRANDOM_SUCCESS;
    }

#else /* POSIX */

    const int ossl_mandatory =
        secrandom_ossl_fips_enabled() /* FIPS Provider / FIPS_mode() */
# ifdef SECRANDOM_PREFER_OPENSSL
        || 1 /* Prefer OpenSSL as primary source */
# endif
        ;

    if (ossl_mandatory &&
        (rc = secrandom_ossl(buf, len)) == SECRANDOM_SUCCESS) {
        return SECRANDOM_SUCCESS;
    }
    /* fallback to other methods if OpenSSL is not mandatory */

    if ((rc = secrandom_arc4random(buf, len)) == SECRANDOM_SUCCESS ||
        (rc = secrandom_getentropy(buf, len)) == SECRANDOM_SUCCESS ||
        (rc = secrandom_urandom_ex(buf, len, fd_urandom)) ==
            SECRANDOM_SUCCESS ||
        /* use OpenSSL as fallback if not mandatory */
        (ossl_mandatory == 0 &&
         (rc = secrandom_ossl(buf, len)) == SECRANDOM_SUCCESS)) {
        return SECRANDOM_SUCCESS;
    }
#endif /* POSIX */

    if (rc == SECRANDOM_UNSUPPORTED) {
        errno = ENOSYS; /* Function not implemented */
    } else {
        errno = EIO; /* Input/output error */
    }
    return rc;
}

#endif /* secrandom_h */

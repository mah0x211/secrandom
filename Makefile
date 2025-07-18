# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wpedantic -std=c99 \
         -Wformat=2 -Wcast-align -Wconversion -Wdouble-promotion \
         -Wfloat-equal -Wpointer-arith -Wshadow -Wuninitialized \
         -Wunused -Wvla -Wwrite-strings -Wstrict-prototypes \
         -Wmissing-prototypes -Wredundant-decls -Winline \
         -fno-common -fstack-protector-strong

# Coverage flags
COV_FLAGS = --coverage -fprofile-arcs -ftest-coverage

# OpenSSL detection
OPENSSL_CFLAGS ?= $(shell pkg-config --cflags openssl 2>/dev/null || echo "")
OPENSSL_LIBS ?= $(shell pkg-config --libs openssl 2>/dev/null || echo "")

# Windows-specific libraries
ifeq ($(OS),Windows_NT)
    LDFLAGS += -lbcrypt
endif

# Test directory
TEST_DIR = test

# Basic test binaries (no external dependencies)
BASIC_TESTS = test_basic test_arc4random test_getentropy test_bcrypt test_urandom

# OpenSSL-dependent test binaries
OPENSSL_TESTS = test_ossl test_integration test_ossl_prefer test_ossl_fips

# All test binaries
ALL_TESTS = $(BASIC_TESTS) $(OPENSSL_TESTS)

# Default target
all: test

# Run basic tests (no OpenSSL required)
test: $(BASIC_TESTS)
	@echo "=== Running Basic Tests ==="
	@for test in $(BASIC_TESTS); do \
		echo "\nRunning $$test..."; \
		./$$test || exit 1; \
	done
	@echo "\n=== Basic Tests Complete ==="

# Run OpenSSL tests
test-openssl: $(OPENSSL_TESTS)
	@echo "=== Running OpenSSL Tests ==="
	@for test in $(OPENSSL_TESTS); do \
		echo "\nRunning $$test..."; \
		./$$test || exit 1; \
	done
	@echo "\n=== OpenSSL Tests Complete ==="

# Run all tests
test-all: test test-openssl

# Build rules for basic tests
test_basic: $(TEST_DIR)/test_basic.c
	$(CC) $(CFLAGS) -o $@ $<

test_arc4random: $(TEST_DIR)/test_arc4random.c
	$(CC) $(CFLAGS) -o $@ $<

test_getentropy: $(TEST_DIR)/test_getentropy.c
	$(CC) $(CFLAGS) -o $@ $<

test_bcrypt: $(TEST_DIR)/test_bcrypt.c
	$(CC) $(CFLAGS) -o $@ $<

test_urandom: $(TEST_DIR)/test_urandom.c
	$(CC) $(CFLAGS) -o $@ $<

# Build rules for OpenSSL tests
test_ossl: $(TEST_DIR)/test_ossl.c
	$(CC) $(CFLAGS) $(OPENSSL_CFLAGS) -o $@ $< $(OPENSSL_LIBS)

test_integration: $(TEST_DIR)/test_integration.c
	$(CC) $(CFLAGS) $(OPENSSL_CFLAGS) -o $@ $< $(OPENSSL_LIBS)

test_ossl_prefer: $(TEST_DIR)/test_ossl_prefer.c
	$(CC) $(CFLAGS) $(OPENSSL_CFLAGS) -DSECRANDOM_PREFER_OPENSSL -o $@ $< $(OPENSSL_LIBS)

test_ossl_fips: $(TEST_DIR)/test_ossl_fips.c
	$(CC) $(CFLAGS) $(OPENSSL_CFLAGS) -o $@ $< $(OPENSSL_LIBS)

# Coverage target for tests (no OpenSSL)
coverage: clean
	@echo "=== Running Tests Coverage ==="
	@for test in $(BASIC_TESTS); do \
		$(CC) $(CFLAGS) $(COV_FLAGS) -o $$test\_cov $(TEST_DIR)/$$test.c; \
		echo "Running $$test\_cov..."; \
		./$$test\_cov || exit 1; \
	done
	lcov --capture --directory . --output-file coverage.info
	lcov --ignore-errors unused --remove coverage.info '/usr/include/*' 'test/*' --output-file coverage.info
	genhtml coverage.info --output-directory coverage_report
	@echo "Coverage report generated in coverage_report/index.html"

# Coverage target for OpenSSL tests
coverage-openssl: clean
	@echo "=== Running OpenSSL Tests Coverage ==="
	@for test in $(OPENSSL_TESTS); do \
		if [ "$$test" = "test_ossl_prefer" ]; then \
			$(CC) $(CFLAGS) $(COV_FLAGS) $(OPENSSL_CFLAGS) -DSECRANDOM_PREFER_OPENSSL -o $$test\_cov $(TEST_DIR)/$$test.c $(OPENSSL_LIBS); \
		else \
			$(CC) $(CFLAGS) $(COV_FLAGS) $(OPENSSL_CFLAGS) -o $$test\_cov $(TEST_DIR)/$$test.c $(OPENSSL_LIBS); \
		fi; \
		echo "Running $$test\_cov..."; \
		./$$test\_cov || exit 1; \
	done
	lcov --capture --directory . --output-file coverage-openssl.info
	lcov --ignore-errors unused --remove coverage-openssl.info '/usr/include/*' 'test/*' --output-file coverage-openssl.info
	genhtml coverage-openssl.info --output-directory coverage_report_openssl
	@echo "OpenSSL coverage report generated in coverage_report_openssl/index.html"

# Coverage target - run all tests with coverage (for local use)
coverage-all: clean
	@echo "=== Running Full Coverage Tests ==="
	@echo "Building basic tests with coverage..."
	@for test in $(BASIC_TESTS); do \
		$(CC) $(CFLAGS) $(COV_FLAGS) -o $$test\_cov $(TEST_DIR)/$$test.c; \
		echo "Running $$test\_cov..."; \
		./$$test\_cov || exit 1; \
	done
	@echo "Building OpenSSL tests with coverage..."
	@for test in $(OPENSSL_TESTS); do \
		if [ "$$test" = "test_ossl_prefer" ]; then \
			$(CC) $(CFLAGS) $(COV_FLAGS) $(OPENSSL_CFLAGS) -DSECRANDOM_PREFER_OPENSSL -o $$test\_cov $(TEST_DIR)/$$test.c $(OPENSSL_LIBS); \
		else \
			$(CC) $(CFLAGS) $(COV_FLAGS) $(OPENSSL_CFLAGS) -o $$test\_cov $(TEST_DIR)/$$test.c $(OPENSSL_LIBS); \
		fi; \
		echo "Running $$test\_cov..."; \
		./$$test\_cov || exit 1; \
	done
	lcov --capture --directory . --output-file coverage-all.info
	lcov --ignore-errors unused --remove coverage-all.info '/usr/include/*' 'test/*' --output-file coverage-all.info
	genhtml coverage-all.info --output-directory coverage_report_all
	@echo "Full coverage report generated in coverage_report_all/index.html"

# Address Sanitizer
asan: clean
	$(CC) $(CFLAGS) -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer \
		$(OPENSSL_CFLAGS) -o test_asan $(TEST_DIR)/test_integration.c $(OPENSSL_LIBS)
	./test_asan

# Clean
clean:
	rm -f $(ALL_TESTS) test_coverage test_asan *_cov
	rm -f *.gcda *.gcno coverage.info coverage-openssl.info coverage-all.info
	rm -rf coverage_report coverage_report_openssl coverage_report_all

# Help
help:
	@echo "Available targets:"
	@echo "  make         - Run basic tests (no OpenSSL required)"
	@echo "  make test-openssl - Run OpenSSL-specific tests"
	@echo "  make test-all - Run all tests"
	@echo "  make coverage - Generate coverage report"
	@echo "  make asan    - Run tests with Address Sanitizer"
	@echo "  make clean   - Remove all generated files"
	@echo ""
	@echo "Individual test targets:"
	@echo "  make test_basic      - Basic functionality tests"
	@echo "  make test_arc4random - arc4random_buf tests"
	@echo "  make test_getentropy - getentropy tests"
	@echo "  make test_bcrypt     - BCryptGenRandom tests"
	@echo "  make test_urandom    - /dev/urandom tests"
	@echo "  make test_ossl       - OpenSSL tests"
	@echo "  make test_integration - Integration tests"
	@echo "  make test_ossl_prefer - OpenSSL preference tests"
	@echo "  make test_ossl_fips  - FIPS mode tests"

.PHONY: all test test-openssl test-all coverage asan clean help

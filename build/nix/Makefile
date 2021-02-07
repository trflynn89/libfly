# Define the path to the source directory.
SOURCE_ROOT := $(CURDIR)/../..

# Define the project version.
VERSION := $(shell cat $(SOURCE_ROOT)/VERSION.md)

# Import the build API.
include /usr/local/src/fly/api.mk

# Main targets.
$(eval $(call ADD_TARGET, libfly, fly, LIB))

# Test targets.
$(eval $(call ADD_TARGET, catch2, extern/catchorg, LIB))
$(eval $(call ADD_TARGET, libfly_unit_tests, test, TEST, libfly))
$(eval $(call ADD_TARGET, libfly_benchmarks, bench, BIN, libfly))

# Paths to ignore during code coverage reporting.
#
# 1. Ignore test files.
# 2. Ignore benchmarking files.
# 3. Ignore external third-party files.
# 4. Ignore system_impl.hpp - this file is entirely constexpr functions that do not execute at
#    runtime, which llvm-cov doesn't seem to recognize.
# 5. Ignore literal_parser.hpp - this file is entirely constexpr functions that do not execute at
#    runtime, which llvm-cov doesn't seem to recognize.
$(eval $(call IGNORE_FOR_COVERAGE, test/))
$(eval $(call IGNORE_FOR_COVERAGE, bench/))
$(eval $(call IGNORE_FOR_COVERAGE, extern/))
$(eval $(call IGNORE_FOR_COVERAGE, fly/system/nix/system_impl.hpp))
$(eval $(call IGNORE_FOR_COVERAGE, fly/types/numeric/detail/literal_parser.hpp))

# Import the build system.
include /usr/local/src/fly/build.mk

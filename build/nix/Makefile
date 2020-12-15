# Define the path to the source directory.
SOURCE_ROOT := $(CURDIR)/../..

# Define the project version.
VERSION := $(shell cat $(SOURCE_ROOT)/VERSION.md)

# Import the build API.
include api.mk

# Main targets.
$(eval $(call ADD_TARGET, libfly, fly, LIB))

# Test targets.
$(eval $(call ADD_TARGET, libfly_unit_tests, test, TEST))
$(eval $(call ADD_TARGET, libfly_benchmarks, bench, BIN))

# Paths to ignore during code coverage reporting.
#
# 1. Ignore benchmarking files.
# 2. Ignore test files.
# 3. Ignore system_impl.hpp - this file is entirely constexpr functions that do not execute at
#    runtime, which llvm-cov doesn't seem to recognize.
# 4. Ignore literal_parser.hpp - this file is entirely constexpr functions that do not execute at
#    runtime, which llvm-cov doesn't seem to recognize.
$(eval $(call IGNORE_FOR_COVERAGE, bench/))
$(eval $(call IGNORE_FOR_COVERAGE, test/))
$(eval $(call IGNORE_FOR_COVERAGE, fly/system/nix/system_impl.hpp))
$(eval $(call IGNORE_FOR_COVERAGE, fly/types/numeric/detail/literal_parser.hpp))

# Import the build system.
include build.mk

SOURCE_ROOT := $(CURDIR)/../..
VERSION := $(shell cat $(SOURCE_ROOT)/VERSION.md)

include /usr/local/src/fly/api.mk

# Main targets.
$(eval $(call ADD_TARGET, libfly, fly, LIB))

# Test targets.
$(eval $(call ADD_TARGET, catch2, extern/catchorg, LIB))
$(eval $(call ADD_TARGET, libfly_unit_tests, test, TEST, catch2 libfly))
$(eval $(call ADD_TARGET, libfly_benchmarks, bench, BIN, catch2 libfly))

# Paths to exclude from style enforcement.
$(eval $(call EXCLUDE_FROM_STYLE_ENFORCEMENT, extern/))

# Paths to exclude from code coverage reporting.
#
# 1. Ignore test files.
# 2. Ignore benchmarking files.
# 3. Ignore external third-party files.
# 4. Ignore system_impl.hpp - this file is entirely constexpr functions that do not execute at
#    runtime, which llvm-cov doesn't seem to recognize.
# 5. Ignore literal_parser.hpp - this file is entirely constexpr functions that do not execute at
#    runtime, which llvm-cov doesn't seem to recognize.
$(eval $(call EXCLUDE_FROM_COVERAGE, test/))
$(eval $(call EXCLUDE_FROM_COVERAGE, bench/))
$(eval $(call EXCLUDE_FROM_COVERAGE, extern/))
$(eval $(call EXCLUDE_FROM_COVERAGE, fly/system/nix/system_impl.hpp))
$(eval $(call EXCLUDE_FROM_COVERAGE, fly/types/numeric/detail/literal_parser.hpp))

# Paths to exclude from generation of compilation database.
$(eval $(call EXCLUDE_FROM_COMPILATION_DATABASE, extern/))

include /usr/local/src/fly/build.mk

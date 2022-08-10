SOURCE_ROOT := $(CURDIR)
VERSION := $(shell cat $(SOURCE_ROOT)/VERSION.md)

include $(SOURCE_ROOT)/extern/trflynn89/flymake/src/api.mk

$(eval $(call ADD_SCRIPT, download_bench_data, build/scripts/download_build_data.py, bench))
$(eval $(call ADD_SCRIPT, download_test_data, build/scripts/download_build_data.py, test))

# Main targets.
$(eval $(call ADD_TARGET, libfly, fly, LIB))

# Test targets.
$(eval $(call ADD_TARGET, catch2, extern/catchorg, LIB))
$(eval $(call ADD_TARGET, libfly_unit_tests, test, TEST, catch2 libfly))
$(eval $(call ADD_TARGET, libfly_benchmarks, bench, BIN, \
    catch2 libfly download_bench_data download_test_data))

# Paths to exclude from style enforcement.
$(eval $(call EXCLUDE_FROM_STYLE_ENFORCEMENT, extern/))

# Paths to exclude from code coverage reporting.
#
# 1. Ignore test files.
# 2. Ignore benchmarking files.
# 3. Ignore external third-party files.
# 4. Ignore literal_parser.hpp - this file is entirely constexpr functions that do not execute at
#    runtime, which llvm-cov doesn't seem to recognize.
# 5. Ignore format_parameter_type.hpp - this file is entirely constexpr functions that do not
#    execute at runtime, which llvm-cov doesn't seem to recognize.
$(eval $(call EXCLUDE_FROM_COVERAGE, test/))
$(eval $(call EXCLUDE_FROM_COVERAGE, bench/))
$(eval $(call EXCLUDE_FROM_COVERAGE, extern/))
$(eval $(call EXCLUDE_FROM_COVERAGE, fly/types/numeric/detail/literal_parser.hpp))
$(eval $(call EXCLUDE_FROM_COVERAGE, fly/types/string/detail/format_parameter_type.hpp))

# Paths to exclude from generation of compilation database.
$(eval $(call EXCLUDE_FROM_COMPILATION_DATABASE, extern/))

# Override default flymake configuration.
output ?= $(SOURCE_ROOT)/build

include $(SOURCE_ROOT)/extern/trflynn89/flymake/src/build.mk

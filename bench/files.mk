SRC_DIRS_$(d) += \
    bench/coders \
    bench/json \
    bench/string \
    bench/util \
    test/util

SRC_$(d) := \
    $(d)/main.cpp

# All benchmarks should have Catch2 on the include path.
CXXFLAGS_$(d) += \
    -I$(SOURCE_ROOT)/test/Catch2/single_include \
    -DCATCH_CONFIG_PREFIX_ALL \
    -DCATCH_CONFIG_FAST_COMPILE \
    -DCATCH_CONFIG_ENABLE_OPTIONAL_STRINGMAKER

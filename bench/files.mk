include $(SOURCE_ROOT)/extern/catchorg/flags.mk

SRC_DIRS_$(d) += \
    bench/coders \
    bench/json \
    bench/string \
    bench/util \
    test/util

SRC_$(d) := \
    $(d)/main.cpp

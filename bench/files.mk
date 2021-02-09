include $(SOURCE_ROOT)/extern/catchorg/flags.mk

SRC_DIRS_$(d) += \
    $(d)/coders \
    $(d)/json \
    $(d)/string \
    $(d)/util \
    $(d)/../test/util

SRC_$(d) := \
    $(d)/main.cpp

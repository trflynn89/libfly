SRC_DIRS_$(d) := \
    cpp/some_lib

SRC_$(d) := \
    $(d)/some_lib.cpp

LDLIBS_$(d) := \
    -lfly

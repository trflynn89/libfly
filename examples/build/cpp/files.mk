SRC_DIRS_$(d) := \
    cpp/some_lib

SRC_$(d) := \
    $(d)/main.cpp

LDLIBS_$(d) := \
    -lfly

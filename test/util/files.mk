SRC_$(d) := \
    $(d)/capture_stream.cpp \
    $(d)/path_util.cpp \
    $(d)/task_manager.cpp \
    $(d)/waitable_task_runner.cpp

# Always optimize test utility methods.
CFLAGS_$(d) += -O2 -Wno-inline
CXXFLAGS_$(d) += -O2 -Wno-inline

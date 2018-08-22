# Define source files
SRC_$(d) := \
    $(d)/googletest/googletest/src/gtest-all.cc \
    $(d)/googletest/googletest/src/gtest_main.cc \
    $(d)/util/waitable_task_runner.cpp

# Define source files for mocked system calls
ifneq ($(MOCK_SYSTEM_CALLS), )

SRC_$(d) += \
    $(d)/mock/mock_system.cpp \
    $(d)/mock/nix/mock_list.cpp \
    $(foreach mock, $(MOCK_SYSTEM_CALLS), $(d)/mock/nix/mock_$(mock).cpp)

endif

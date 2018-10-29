# Define source files - only include address sanitizer default overrides in the
# build if it is actually being used.
ifeq ($(release), 0)
    ifeq ($(arch), x64)
        SRC_$(d) := \
            $(d)/sanitizer.cpp
    endif
endif

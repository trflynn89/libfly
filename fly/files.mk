SRC_DIRS_$(d) := \
    $(d)/assert \
    $(d)/coders \
    $(d)/config \
    $(d)/logger \
    $(d)/net \
    $(d)/parser \
    $(d)/path \
    $(d)/system \
    $(d)/task \
    $(d)/types

# Add libfly.a and libfly.so to the archived release package.
$(eval $(call ADD_REL_LIB))

# Add all header files to the archived release package.
$(eval $(call ADD_REL_INC, $(d), *.hpp, fly))

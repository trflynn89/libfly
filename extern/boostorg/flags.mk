CXXFLAGS_$(d) += \
    -isystem $(SOURCE_ROOT)/extern/boostorg/json/include \
    -DBOOST_JSON_STANDALONE

# On macOS, Boost warns (via #warning) that <memory_resource> does not exist. This is then promoted
# to an error due to -Werror; set -Wno-error for macOS only.
ifeq ($(SYSTEM), MACOS)
    CXXFLAGS_$(d) += -Wno-error
endif

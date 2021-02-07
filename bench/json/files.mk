include $(SOURCE_ROOT)/extern/boostorg/flags.mk

SRC_$(d) := \
    $(d)/benchmark_json.cpp

# JSON for Modern C++ - https://github.com/nlohmann/json
NLOHMANN_FLAGS_$(d) := \
    -I$(d)/extern/nlohmann/single_include

CFLAGS_$(d) += $(BOOST_FLAGS_$(d)) $(NLOHMANN_FLAGS_$(d))
CXXFLAGS_$(d) += $(BOOST_FLAGS_$(d)) $(NLOHMANN_FLAGS_$(d))

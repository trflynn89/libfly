SRC_$(d) := \
    $(d)/benchmark_json.cpp

# Boost.JSON - https://github.com/boostorg/json
BOOST_FLAGS_$(d) := \
    -I$(d)/extern/boost/include \
    -DBOOST_JSON_STANDALONE \
    -Wno-cast-align \
    -Wno-float-equal \
    -Wno-newline-eof \
    -Wno-old-style-cast \
    -Wno-sign-conversion \
    -Wno-undef

# JSON for Modern C++ - https://github.com/nlohmann/json
NLOHMANN_FLAGS_$(d) := \
    -I$(d)/extern/nlohmann/single_include

CFLAGS_$(d) += $(BOOST_FLAGS_$(d)) $(NLOHMANN_FLAGS_$(d))
CXXFLAGS_$(d) += $(BOOST_FLAGS_$(d)) $(NLOHMANN_FLAGS_$(d))

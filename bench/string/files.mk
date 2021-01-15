SRC_$(d) := \
    $(d)/benchmark_string.cpp

# {fmt} - https://github.com/fmtlib/fmt
FMT_FLAGS_$(d) := \
    -I$(d)/extern/fmt/include \
    -DFMT_HEADER_ONLY

CFLAGS_$(d) += $(FMT_FLAGS_$(d))
CXXFLAGS_$(d) += $(FMT_FLAGS_$(d))

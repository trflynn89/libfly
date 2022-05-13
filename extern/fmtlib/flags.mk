CXXFLAGS_$(d) += \
    -I$(SOURCE_ROOT)/extern/fmtlib/fmt/include \
    -DFMT_HEADER_ONLY \
    -Wno-inline \
    -Wno-strict-overflow

CXXFLAGS_$(d) += \
    -I$(SOURCE_ROOT)/extern/boostorg/json/include \
    -DBOOST_JSON_STANDALONE \
    -Wno-cast-align \
    -Wno-float-equal \
    -Wno-newline-eof \
    -Wno-old-style-cast \
    -Wno-sign-conversion \
    -Wno-undef

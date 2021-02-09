SRC_DIRS_$(d) := \
    $(d)/base64 \
    $(d)/huffman

$(eval $(call WILDCARD_SOURCES, CPP))

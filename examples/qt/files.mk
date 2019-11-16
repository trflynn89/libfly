# Define source files
SRC_$(d) := \
    $(d)/main.cpp

# Define Qt resource collection files
QRC_$(d) := \
    $(d)/clocks.qrc

# Define Qt modules
QT_MODULES_$(d) := \
    Core \
    Gui \
    Qml

# Add libfly_qt_example binary to release package
$(eval $(call ADD_REL_BIN, libfly_qt_example))

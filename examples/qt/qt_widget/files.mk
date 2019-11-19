# Define source files
SRC_$(d) := \
    $(d)/main.cpp \
    $(d)/notepad.cpp

# Define Qt user interface files
QT_UI_$(d) := \
    $(d)/notepad.ui

# Define Qt meta-object header files
QT_MOC_$(d) := \
    $(d)/notepad.h

# Define Qt resource collection files
QT_QRC_$(d) := \
    $(d)/notepad.qrc

# Define Qt modules
QT_MODULES_$(d) := \
    Core \
    Gui \
    PrintSupport \
    Widgets

# Add libfly_qt_widget_example binary to release package
$(eval $(call ADD_REL_BIN, libfly_qt_widget_example))

# Define source files
SRC_$(d) := \
    $(d)/main.cpp \
    $(d)/main_window.cpp

# Define Qt user interface files
QT_UI_$(d) := \
    $(d)/main_window.ui

# Define Qt meta-object header files
QT_MOC_$(d) := \
    $(d)/main_window.h

# Define Qt resource collection files
QT_QRC_$(d) := \
    $(d)/example.qrc

# Define Qt modules
QT_MODULES_$(d) := \
    Core \
    Gui \
    Widgets

# Add libfly_qt_widget_example binary to release package
$(eval $(call ADD_REL_BIN, libfly_qt_widget_example))

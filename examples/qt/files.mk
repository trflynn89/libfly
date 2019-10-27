# Define source files
SRC_$(d) := \
    $(d)/main.cpp \
    $(d)/main_window.cpp

# Define Qt UIC/MOC/RCC source files
QT_UIC_$(d) := main_window
QT_MOC_$(d) := main_window
QT_RCC_$(d) := example

# Define libraries to link
LDLIBS_$(d) := \
    -latomic \
    -lpthread

# Add libfly_qt_example binary to release package
$(eval $(call ADD_REL_BIN, libfly_qt_example))

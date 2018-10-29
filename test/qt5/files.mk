# Define the directories to include and compile
SRC_DIRS_$(d) := \
    fly/sanitizer

# Define source files
SRC_$(d) := \
    $(d)/main.cpp \
    $(d)/main_window.cpp

# Define QT5 UIC/MOC/RCC source files
QT5_UIC_$(d) := main_window
QT5_MOC_$(d) := main_window
QT5_RCC_$(d) := example

# Define libraries to link
LDLIBS_$(d) := \
    -latomic \
    -lpthread

# Add libfly_qt5_example binary to release package
$(eval $(call ADD_REL_BIN, libfly_qt5_example))

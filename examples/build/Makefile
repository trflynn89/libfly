# Example usage of the build system.

# Define the path to the source directory
SOURCE_ROOT := $(CURDIR)

# Define the project version
VERSION = 1.0.0

# Import the build API
include /usr/local/src/fly/api.mk

# Main targets
$(eval $(call ADD_TARGET, libfly_cpp_example, cpp, BIN))
$(eval $(call ADD_TARGET, libfly_mvn_example, jar/src/main/java, JAR))

# Test targets
$(eval $(call ADD_TARGET, libfly_cpp_example_tests, cpp/test, TEST))

# Import the build system
include /usr/local/src/fly/build.mk

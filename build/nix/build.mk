# Import the build system and define make targets. Applications using this build
# system should include this file last. Before including this file, applications
# must define SOURCE_ROOT, the path to the application's source tree, and
# VERSION, a version number of the form "X.Y.C".

.PHONY: all
.PHONY: clean
.PHONY: tests
.PHONY: gcov
.PHONY: install
.PHONY: setup
.PHONY: style
.PHONY: $(TARGETS)

# Verify expected variables
ifeq ($(SOURCE_ROOT),)
    $(error SOURCE_ROOT must be defined)
else ifeq ($(wildcard $(SOURCE_ROOT)/.*),)
    $(error SOURCE_ROOT $(SOURCE_ROOT) does not exist)
endif

ifeq ($(VERSION),)
    $(error VERSION must be defined)
else ifneq ($(words $(subst ., ,$(VERSION))), 3)
    $(error VERSION $(VERSION) must be of the form X.Y.Z)
endif

# Get the path to this file
BUILD_ROOT := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
BUILD_ROOT := $(patsubst %/,%,$(BUILD_ROOT))

# Define 'all' target before importing the build system
all: $(TARGETS)

# Import the build system
include $(BUILD_ROOT)/qt5.mk
include $(BUILD_ROOT)/system.mk
include $(BUILD_ROOT)/config.mk
include $(BUILD_ROOT)/release.mk
include $(BUILD_ROOT)/flags.mk
include $(BUILD_ROOT)/files.mk
include $(BUILD_ROOT)/compile.mk
include $(BUILD_ROOT)/stack.mk
include $(BUILD_ROOT)/target.mk

# Clean up output files
clean:
	@echo "[Clean $(subst $(CURDIR)/,,$(OUT_DIR))]"
	$(Q)$(RM) -r $(OUT_DIR)

# Run all unit tests
tests: $(TEST_BINARIES)
	$(Q)failed=0; \
	for tgt in $(TEST_BINARIES) ; do \
		$$tgt; \
		if [[ $$? -ne 0 ]] ; then \
			failed=$$((failed+1)); \
		fi; \
	done; \
	exit $$failed

# Create coverage reports
gcov: tests
	$(Q)for obj in $$(find $(OBJ_DIR) -name "*.o" -print) ; do \
		path=$$(dirname "$$obj"); \
		file=$$(basename "$$obj"); \
		\
		pushd $$path > /dev/null; \
		gcov $(GCOV_FLAGS) $$file; \
		\
		popd > /dev/null; \
	done; \
	\
	find . -name "*\#\#*.gcov" | xargs grep -l "/usr/include" | xargs -I {} $(RM) {}

# Install the target
install: $(TARGET_PACKAGES)
	$(Q)failed=0; \
	for pkg in $(TARGET_PACKAGES) ; do \
		if [[ -f $$pkg ]] ; then \
			sudo tar -C / $(TAR_EXTRACT_FLAGS) $$pkg; \
			if [[ $$? -ne 0 ]] ; then \
				failed=$$((failed+1)); \
			fi; \
		fi; \
	done; \
	exit $$failed

# Install dependencies
setup:
ifeq ($(HOST), DEBIAN)
	$(Q)sudo apt-get install -y git make gcc g++ gcc-multilib g++-multilib clang llvm

ifeq ($(qt5), 1)
	$(Q)sudo apt-get install -y mesa-common-dev
	$(Q)$(QT5_INSTALL)
endif
else
	$(Q)echo "No setup rules defined for host $(HOST), check build.mk"
	$(Q)exit 1
endif

# Style enforcement
style:
	clang-format -i $$(find $(SOURCE_ROOT)/fly -iname "*.h" -o -iname "*.cpp")


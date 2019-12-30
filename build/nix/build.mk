# Import the build system and define make targets. Applications using this build
# system should include this file last. Before including this file, applications
# must define SOURCE_ROOT, the path to the application's source tree, and
# VERSION, a version number of the form "X.Y.C".

.PHONY: all
.PHONY: clean
.PHONY: tests
.PHONY: coverage
.PHONY: install
.PHONY: setup
.PHONY: qt
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
include $(BUILD_ROOT)/system.mk
include $(BUILD_ROOT)/config.mk
include $(BUILD_ROOT)/qt.mk
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
		if [[ $(toolchain) == "clang" ]] && [[ $(release) -eq 0 ]] ; then \
			export LLVM_PROFILE_FILE="$$tgt.profraw"; \
		fi; \
		\
		$$tgt; \
		\
		if [[ $$? -ne 0 ]] ; then \
			failed=$$((failed+1)); \
		fi; \
	done; \
	exit $$failed

# Create coverage reports
coverage: report := $(OUT_DIR)/coverage
coverage:
ifeq ($(toolchain), clang)
	$(Q)llvm-profdata merge \
		--output $(report).prodata \
		$(addsuffix .profraw, $(TEST_BINARIES))

	$(Q)llvm-cov show \
		--instr-profile=$(report).prodata \
		--ignore-filename-regex="test/.*" \
		$(TEST_BINARIES) > $(report)

	$(Q)llvm-cov report \
		--instr-profile=$(report).prodata \
		--ignore-filename-regex="test/.*" \
		$(TEST_BINARIES)

else ifeq ($(toolchain), gcc)
	$(Q)lcov --capture --directory $(OUT_DIR) --output-file $(report)
	$(Q)lcov --remove $(report) '/usr/*' '*/test/*' --output-file $(report)
	$(Q)lcov --list $(report)

else
	$(Q)echo "No coverage rules for toolchain $(toolchain), check build.mk"
	$(Q)exit 1
endif

# Install the target
install: $(TARGET_PACKAGES)
	$(Q)failed=0; \
	for pkg in $(TARGET_PACKAGES) ; do \
		if [[ -f $$pkg ]] ; then \
			$(SUDO) tar -C / $(TAR_EXTRACT_FLAGS) $$pkg; \
			if [[ $$? -ne 0 ]] ; then \
				failed=$$((failed+1)); \
			fi; \
		fi; \
	done; \
	exit $$failed

# Install dependencies
setup:
ifeq ($(HOST), DEBIAN)
	$(Q)$(SUDO) apt install -y git make clang clang-format lld llvm gcc g++ lcov
ifeq ($(arch), x86)
	$(Q)$(SUDO) apt install -y gcc-multilib g++-multilib
endif

else ifeq ($(HOST), REDHAT)
	$(Q)$(SUDO) dnf install -y git make clang lld llvm gcc gcc-c++ lcov \
		libstdc++-static libasan libatomic
ifeq ($(arch), x86)
	$(Q)$(SUDO) dnf install -y glibc-devel.i686 \
		libstdc++-static.i686 libasan.i686 libatomic.i686
endif

else
	$(Q)echo "No setup rules for host $(HOST), check build.mk"
	$(Q)exit 1
endif

# Install Qt
qt:
# The x86 Qt installer has not been updated since 2017, and only supports up to
# Qt 5.5. But Qt 5.13+ is needed for C++17, so only x64 Qt projects are allowed.
ifeq ($(arch), x86)
	$(error Architecture $(arch) not supported, check qt.mk)
endif

ifeq ($(HOST), DEBIAN)
	$(Q)$(SUDO) apt install -y mesa-common-dev
endif
	$(Q)$(QT_INSTALL)

# Style enforcement
style:
	$(Q)clang-format -i $$(find $(SOURCE_ROOT) \
		-not \( -path "*googletest*" -prune \) \
		-name "*.h" -o -name "*.c" -o -name "*.cc" -o -name "*.cpp")

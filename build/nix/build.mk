# Import the build system and generate all make targets. Applications using this build system should
# include this file last in their Makefile. The following top-level targets are defined:
#
#     all = Build all targets added via $(ADD_TARGET) in the application Makefile.
#     clean = Remove the build output directory for the current configuration.
#     tests = Run all unit tests added via $(ADD_TARGET) with a target type of TEST.
#     profile = Run all unit tests and generate a code profile report of the unit test execution.
#     coverage = Generate a code coverage report of the last unit test execution.
#     install = Extract any target installation package in the file system root directory.
#     setup = Install the toolchain packages required to build libfly.
#     style = Run clang-format on all source files.
#
# The above targets may be configured by a set of command line arguments defined in config.mk.
.PHONY: all
.PHONY: clean
.PHONY: tests
.PHONY: profile
.PHONY: coverage
.PHONY: install
.PHONY: setup
.PHONY: style

# Get the path to this file.
BUILD_ROOT := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
BUILD_ROOT := $(patsubst %/,%,$(BUILD_ROOT))

# Define 'all' target before importing the build system.
all: $(TARGETS)

# Import the build system
include $(BUILD_ROOT)/system.mk
include $(BUILD_ROOT)/config.mk
include $(BUILD_ROOT)/release.mk
include $(BUILD_ROOT)/flags.mk
include $(BUILD_ROOT)/files.mk
include $(BUILD_ROOT)/compile.mk
include $(BUILD_ROOT)/stack.mk
include $(BUILD_ROOT)/target.mk

# Clean up output files.
clean:
	@echo -e "[$(RED)Clean$(DEFAULT) $(subst $(CURDIR)/,,$(OUT_DIR))]"
	$(Q)$(RM) -r $(OUT_DIR)

# Run all unit tests.
tests: args :=
tests: $(TEST_BINARIES)
	$(Q)failed=0; \
	\
	for tgt in $(TEST_BINARIES) ; do \
		if [[ $(toolchain) == "clang" ]] && [[ $(mode) == "debug" ]] ; then \
			export LLVM_PROFILE_FILE="$$tgt.profraw"; \
		fi; \
		\
		$$tgt $(args); \
		\
		if [[ $$? -ne 0 ]] ; then \
			failed=$$((failed+1)); \
		fi; \
	done; \
	\
	exit $$failed

# Create profile report.
profile: report := $(ETC_DIR)/profile
profile: args :=
profile: $(TEST_BINARIES)
ifeq ($(toolchain), gcc)
	$(Q)failed=0; \
	for tgt in $(TEST_BINARIES) ; do \
		$$tgt $(args); \
		\
		if [[ $$? -ne 0 ]] ; then \
			failed=$$((failed+1)); \
		else \
			gprof $$tgt > $(report); \
			$(RM) gmon.out; \
		fi; \
	done; \
	exit $$failed
else
	$(Q)echo "No profiling rules for toolchain $(toolchain), check build.mk"
	$(Q)exit 1
endif

# Create coverage report.
coverage: report := $(ETC_DIR)/coverage
coverage:
	$(Q)mkdir -p $(dir $(report))

ifeq ($(toolchain), clang)
	$(Q)llvm-profdata merge \
		--output $(report).prodata \
		$(addsuffix .profraw, $(TEST_BINARIES))

	$(Q)llvm-cov show \
		--instr-profile=$(report).prodata \
		$(addprefix --ignore-filename-regex=, $(COVERAGE_BLACKLIST)) \
		$(TEST_BINARIES) > $(report)

ifeq ($(verbose), 1)
	$(Q)llvm-cov show \
		--instr-profile=$(report).prodata \
		--format=html -Xdemangler=llvm-cxxfilt \
		$(addprefix --ignore-filename-regex=, $(COVERAGE_BLACKLIST)) \
		$(TEST_BINARIES) > $(report).html
endif

	$(Q)llvm-cov report \
		--instr-profile=$(report).prodata \
		$(addprefix --ignore-filename-regex=, $(COVERAGE_BLACKLIST)) \
		$(TEST_BINARIES)

else ifeq ($(toolchain), gcc)
	$(Q)lcov --capture \
		--directory $(CXX_DIR) \
		--output-file $(report)

	$(Q)lcov --remove \
		$(report) \
		'/usr/*' $(addprefix *, $(addsuffix *, $(COVERAGE_BLACKLIST))) \
		--output-file $(report)

	$(Q)lcov --list $(report)

else
	$(Q)echo "No coverage rules for toolchain $(toolchain), check build.mk"
	$(Q)exit 1
endif

# Install the target.
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

# Install dependencies.
setup:
ifeq ($(SYSTEM), LINUX)
ifeq ($(VENDOR), DEBIAN)
	$(Q)$(SUDO) apt install -y git make clang clang-format clang-tidy lld llvm gcc g++ lcov \
		openjdk-14-jdk
ifeq ($(arch), x86)
	$(Q)$(SUDO) apt install -y gcc-multilib g++-multilib
endif
else ifeq ($(VENDOR), REDHAT)
	$(Q)$(SUDO) dnf install -y git make clang lld llvm gcc gcc-c++ lcov libstdc++-static libasan \
		libatomic java-14-openjdk-devel
ifeq ($(arch), x86)
	$(Q)$(SUDO) dnf install -y glibc-devel.i686 libstdc++-static.i686 libasan.i686 libatomic.i686
endif
else
	$(Q)echo "No setup rules for vendor $(VENDOR), check build.mk"
	$(Q)exit 1
endif
else ifeq ($(SYSTEM), MACOS)
	$(Q)xcode-select --install
else
	$(Q)echo "No setup rules for system $(SYSTEM), check build.mk"
	$(Q)exit 1
endif

# Style enforcement.
style:
	$(Q)clang-format -i $$(find $(SOURCE_ROOT) \
		-not \( -path "*Catch2*" -prune \) -type f \
		-name "*.h" -o -name "*.hh" -o -name "*.hpp" \
		-o -name "*.c" -o -name "*.cc" -o -name "*.cpp" \
		-o -name "*.m" -o -name "*.mm" \
		-o -name "*.java")

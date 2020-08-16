# Import the build system and define make targets. Applications using this build
# system should include this file last. Before including this file, applications
# must define SOURCE_ROOT, the path to the application's source tree, and
# VERSION, a version number of the form "X.Y.C".

.PHONY: all
.PHONY: clean
.PHONY: tests
.PHONY: coverage
.PHONY: profile
.PHONY: install
.PHONY: setup
.PHONY: style
.PHONY: $(TARGETS)

# Get the path to this file
BUILD_ROOT := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
BUILD_ROOT := $(patsubst %/,%,$(BUILD_ROOT))

# Define 'all' target before importing the build system
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

# Clean up output files
clean:
	@echo -e "[$(RED)Clean$(DEFAULT) $(subst $(CURDIR)/,,$(OUT_DIR))]"
	$(Q)$(RM) -r $(OUT_DIR)

# Run all unit tests
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
	for tgt in $(TEST_MAVEN_PATHS) ; do \
		$(MVN) $(MVN_FLAGS) -f $(SOURCE_ROOT)/$$tgt/pom.xml test; \
		\
		if [[ $$? -ne 0 ]] ; then \
			failed=$$((failed+1)); \
		fi; \
	done; \
	\
	exit $$failed

# Create coverage report
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
		--directory $(CPP_DIR) \
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

# Create profile report
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
ifeq ($(SYSTEM), LINUX)
ifeq ($(VENDOR), DEBIAN)
	$(Q)$(SUDO) apt install -y git make clang clang-format clang-tidy lld llvm gcc g++ lcov \
		openjdk-14-jdk maven
ifeq ($(arch), x86)
	$(Q)$(SUDO) apt install -y gcc-multilib g++-multilib
endif
else ifeq ($(VENDOR), REDHAT)
	$(Q)$(SUDO) dnf install -y git make clang lld llvm gcc gcc-c++ lcov libstdc++-static libasan \
		libatomic java-14-openjdk-devel maven
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

# Style enforcement
style:
	$(Q)clang-format -i $$(find $(SOURCE_ROOT) \
		-not \( -path "*Catch2*" -prune \) -type f \
		-name "*.h" -o -name "*.hh" -o -name "*.hpp" \
		-o -name "*.c" -o -name "*.cc" -o -name "*.cpp" \
		-o -name "*.java")

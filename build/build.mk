# Import the build system and define make targets. Applications using this build
# system should include this file last. Before including this file, applications
# must define SOURCE_ROOT, the path to the application's source tree, and
# VERSION, a version number of the form "X.Y.C".

.PHONY: all
.PHONY: targets
.PHONY: clean
.PHONY: tests

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

# Import the build system
include $(BUILD_ROOT)/system.mk
include $(BUILD_ROOT)/config.mk
include $(BUILD_ROOT)/target.mk
include $(BUILD_ROOT)/flags.mk
include $(BUILD_ROOT)/stack.mk
include $(BUILD_ROOT)/files.mk
include $(BUILD_ROOT)/compile.mk
include $(BUILD_ROOT)/release.mk

# Define 'all' target before including source directory
all: $(TARGET_NAME)

# Include top-level source directory's files.mk file
ifeq ($(TARGET_TYPE), BIN)
    $(eval $(call INCLUDE_BIN_DIR, $(SOURCE_ROOT), $(TARGET_PATH)))
else ifeq ($(TARGET_TYPE), LIB)
    $(eval $(call INCLUDE_LIB_DIR, $(SOURCE_ROOT), $(TARGET_PATH)))
endif

# Clean up output files
clean:
	@echo "[Clean $(OUT_DIR)]"
	$(Q)$(RM) $(CLEAN)

# Run all unit tests
tests:
	$(Q)numPass=0; numFail=0; \
	for tgt in $(TEST_TARGETS) ; do \
		printf -- "----------- [Test $$tgt] -----------\n\n"; \
		$(MAKE) -j $(NUM_CORES) target=$$tgt run; \
		if [ $$? -ne 0 ] ; then \
			printf -- "[ERROR $$tgt]\n\n"; \
			numFail=$$((numFail+1)); \
		else \
			printf -- "[SUCCESS $$tgt]\n\n"; \
			numPass=$$((numPass+1)); \
		fi; \
	done; \
	printf -- "----------- [Pass $$numPass, Fail $$numFail] -----------\n\n"; \
	exit $$numFail

# Build and run the target
run: $(TARGET_NAME)
ifeq ($(TARGET_TYPE), BIN)
	@$(TARGET_NAME)
endif

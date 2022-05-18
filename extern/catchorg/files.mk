include $(SOURCE_ROOT)/extern/catchorg/flags.mk

SRC_$(d) := \
    $(d)/Catch2/src/catch2/catch_approx.cpp \
    $(d)/Catch2/src/catch2/catch_assertion_result.cpp \
    $(d)/Catch2/src/catch2/catch_config.cpp \
    $(d)/Catch2/src/catch2/catch_message.cpp \
    $(d)/Catch2/src/catch2/catch_registry_hub.cpp \
    $(d)/Catch2/src/catch2/catch_session.cpp \
    $(d)/Catch2/src/catch2/catch_test_case_info.cpp \
    $(d)/Catch2/src/catch2/catch_test_spec.cpp \
    $(d)/Catch2/src/catch2/catch_timer.cpp \
    $(d)/Catch2/src/catch2/catch_tostring.cpp \
    $(d)/Catch2/src/catch2/catch_totals.cpp \
    $(d)/Catch2/src/catch2/catch_version.cpp \
    $(d)/Catch2/src/catch2/generators/internal/catch_generators_combined_tu.cpp \
    $(d)/Catch2/src/catch2/interfaces/catch_interfaces_combined_tu.cpp \
    $(d)/Catch2/src/catch2/interfaces/catch_interfaces_generatortracker.cpp \
    $(d)/Catch2/src/catch2/interfaces/catch_interfaces_reporter.cpp \
    $(d)/Catch2/src/catch2/internal/catch_assertion_handler.cpp \
    $(d)/Catch2/src/catch2/internal/catch_case_insensitive_comparisons.cpp \
    $(d)/Catch2/src/catch2/internal/catch_clara.cpp \
    $(d)/Catch2/src/catch2/internal/catch_combined_tu.cpp \
    $(d)/Catch2/src/catch2/internal/catch_commandline.cpp \
    $(d)/Catch2/src/catch2/internal/catch_console_colour.cpp \
    $(d)/Catch2/src/catch2/internal/catch_context.cpp \
    $(d)/Catch2/src/catch2/internal/catch_debug_console.cpp \
    $(d)/Catch2/src/catch2/internal/catch_debugger.cpp \
    $(d)/Catch2/src/catch2/internal/catch_enforce.cpp \
    $(d)/Catch2/src/catch2/internal/catch_enum_values_registry.cpp \
    $(d)/Catch2/src/catch2/internal/catch_exception_translator_registry.cpp \
    $(d)/Catch2/src/catch2/internal/catch_fatal_condition_handler.cpp \
    $(d)/Catch2/src/catch2/internal/catch_floating_point_helpers.cpp \
    $(d)/Catch2/src/catch2/internal/catch_istream.cpp \
    $(d)/Catch2/src/catch2/internal/catch_list.cpp \
    $(d)/Catch2/src/catch2/internal/catch_output_redirect.cpp \
    $(d)/Catch2/src/catch2/internal/catch_random_number_generator.cpp \
    $(d)/Catch2/src/catch2/internal/catch_random_seed_generation.cpp \
    $(d)/Catch2/src/catch2/internal/catch_reporter_registry.cpp \
    $(d)/Catch2/src/catch2/internal/catch_reporter_spec_parser.cpp \
    $(d)/Catch2/src/catch2/internal/catch_result_type.cpp \
    $(d)/Catch2/src/catch2/internal/catch_reusable_string_stream.cpp \
    $(d)/Catch2/src/catch2/internal/catch_run_context.cpp \
    $(d)/Catch2/src/catch2/internal/catch_section.cpp \
    $(d)/Catch2/src/catch2/internal/catch_singletons.cpp \
    $(d)/Catch2/src/catch2/internal/catch_source_line_info.cpp \
    $(d)/Catch2/src/catch2/internal/catch_stdstreams.cpp \
    $(d)/Catch2/src/catch2/internal/catch_string_manip.cpp \
    $(d)/Catch2/src/catch2/internal/catch_stringref.cpp \
    $(d)/Catch2/src/catch2/internal/catch_tag_alias_registry.cpp \
    $(d)/Catch2/src/catch2/internal/catch_test_case_info_hasher.cpp \
    $(d)/Catch2/src/catch2/internal/catch_test_case_registry_impl.cpp \
    $(d)/Catch2/src/catch2/internal/catch_test_case_tracker.cpp \
    $(d)/Catch2/src/catch2/internal/catch_test_registry.cpp \
    $(d)/Catch2/src/catch2/internal/catch_test_spec_parser.cpp \
    $(d)/Catch2/src/catch2/internal/catch_textflow.cpp \
    $(d)/Catch2/src/catch2/internal/catch_wildcard_pattern.cpp \
    $(d)/Catch2/src/catch2/internal/catch_xmlwriter.cpp \
    $(d)/Catch2/src/catch2/matchers/catch_matchers_floating_point.cpp \
    $(d)/Catch2/src/catch2/matchers/catch_matchers_string.cpp \
    $(d)/Catch2/src/catch2/matchers/catch_matchers_templated.cpp \
    $(d)/Catch2/src/catch2/matchers/internal/catch_matchers_combined_tu.cpp \
    $(d)/Catch2/src/catch2/reporters/catch_reporter_automake.cpp \
    $(d)/Catch2/src/catch2/reporters/catch_reporter_combined_tu.cpp \
    $(d)/Catch2/src/catch2/reporters/catch_reporter_common_base.cpp \
    $(d)/Catch2/src/catch2/reporters/catch_reporter_compact.cpp \
    $(d)/Catch2/src/catch2/reporters/catch_reporter_console.cpp \
    $(d)/Catch2/src/catch2/reporters/catch_reporter_cumulative_base.cpp \
    $(d)/Catch2/src/catch2/reporters/catch_reporter_junit.cpp \
    $(d)/Catch2/src/catch2/reporters/catch_reporter_multi.cpp \
    $(d)/Catch2/src/catch2/reporters/catch_reporter_registrars.cpp \
    $(d)/Catch2/src/catch2/reporters/catch_reporter_sonarqube.cpp \
    $(d)/Catch2/src/catch2/reporters/catch_reporter_streaming_base.cpp \
    $(d)/Catch2/src/catch2/reporters/catch_reporter_tap.cpp \
    $(d)/Catch2/src/catch2/reporters/catch_reporter_teamcity.cpp \
    $(d)/Catch2/src/catch2/reporters/catch_reporter_xml.cpp

CXXFLAGS_$(d) += \
    -Wno-sign-conversion

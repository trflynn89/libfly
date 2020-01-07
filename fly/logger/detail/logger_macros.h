#pragma once

/**
 * Helper macro to add a log, so all of the public logging macros do not have to
 * insert the file, function, and line macros.
 */
#define _LOG(level, message)                                                   \
    fly::Logger::AddLog(level, __FILE__, __FUNCTION__, __LINE__, message)

/**
 * Return the first argument in a list of variadic arguments, expected to be the
 * logging format string.
 */
#define _GET_FORMAT_STRING(...) _GET_FORMAT_STRING_HELPER(__VA_ARGS__, _)

/**
 * Return all but the first argument in a list of variadic arguments, expected
 * to be the arguments to be formatted.
 *
 * If there is only one argument (just the format string), this macro expands to
 * nothing.
 *
 * If there is more than one argument, this macro expands to a comma followed by
 * all but the first argument.
 *
 * Currently supports up to 49 arguments.
 */
#define _GET_FORMAT_ARGS(...)                                                  \
    _GET_FORMAT_ARGS_HELPER(_GET_FORMAT_ARGS_LABEL(__VA_ARGS__), __VA_ARGS__)

//==============================================================================
#define _GET_FORMAT_STRING_HELPER(F, ...) F

#define _GET_FORMAT_ARGS_HELPER(LABEL, ...)                                    \
    _GET_FORMAT_ARGS_HELPER2(LABEL, __VA_ARGS__)

#define _GET_FORMAT_ARGS_HELPER2(LABEL, ...)                                   \
    _GET_FORMAT_ARGS_HELPER_##LABEL(__VA_ARGS__)

#define _GET_FORMAT_ARGS_HELPER_SINGLE(first)

#define _GET_FORMAT_ARGS_HELPER_MULTIPLE(first, ...) , __VA_ARGS__

#define _GET_FORMAT_ARGS_LABEL(...)                                            \
    _GET_FORMAT_ARGS_LABEL_HELPER(                                             \
        __VA_ARGS__,                                                           \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        MULTIPLE,                                                              \
        SINGLE,                                                                \
        _)

#define _GET_FORMAT_ARGS_LABEL_HELPER(                                         \
    ARG_01,                                                                    \
    ARG_02,                                                                    \
    ARG_03,                                                                    \
    ARG_04,                                                                    \
    ARG_05,                                                                    \
    ARG_06,                                                                    \
    ARG_07,                                                                    \
    ARG_08,                                                                    \
    ARG_09,                                                                    \
    ARG_10,                                                                    \
    ARG_11,                                                                    \
    ARG_12,                                                                    \
    ARG_13,                                                                    \
    ARG_14,                                                                    \
    ARG_15,                                                                    \
    ARG_16,                                                                    \
    ARG_17,                                                                    \
    ARG_18,                                                                    \
    ARG_19,                                                                    \
    ARG_20,                                                                    \
    ARG_21,                                                                    \
    ARG_22,                                                                    \
    ARG_23,                                                                    \
    ARG_24,                                                                    \
    ARG_25,                                                                    \
    ARG_26,                                                                    \
    ARG_27,                                                                    \
    ARG_28,                                                                    \
    ARG_29,                                                                    \
    ARG_30,                                                                    \
    ARG_31,                                                                    \
    ARG_32,                                                                    \
    ARG_33,                                                                    \
    ARG_34,                                                                    \
    ARG_35,                                                                    \
    ARG_36,                                                                    \
    ARG_37,                                                                    \
    ARG_38,                                                                    \
    ARG_39,                                                                    \
    ARG_40,                                                                    \
    ARG_41,                                                                    \
    ARG_42,                                                                    \
    ARG_43,                                                                    \
    ARG_44,                                                                    \
    ARG_45,                                                                    \
    ARG_46,                                                                    \
    ARG_47,                                                                    \
    ARG_48,                                                                    \
    ARG_49,                                                                    \
    ARG_50,                                                                    \
    ...)                                                                       \
    ARG_50

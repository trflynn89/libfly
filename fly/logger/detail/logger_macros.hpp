#pragma once

/**
 * Return the first argument in a list of variadic arguments, expected to be the logging format
 * string.
 *
 * Note that __VA_ARGS__ is wrapped in parentheses. MSVC apparently parses __VA_ARGS__ in a way that
 * is not standards compliant. See: https://stackoverflow.com/q/5134523
 *
 * The macros used here are still standards compliant, but are unfortunately quite a bit messier
 * than they need to be. The basic trick is to pass (__VA_ARGS__) into a helper macro, which in turn
 * uses that to form a call to a second helper macro that can actually parse __VA_ARGS__.
 */
#define FLY_FORMAT_STRING(...) FLY_FORMAT_STRING_HELPER((__VA_ARGS__, _))

/**
 * Return all but the first argument in a list of variadic arguments, expected to be the arguments
 * to be formatted.
 *
 * If there is only one argument (just the format string), this macro expands to nothing.
 *
 * If there is more than one argument, this macro expands to a comma followed by all but the first
 * argument.
 *
 * Currently supports up to and including 50 arguments.
 *
 * Note that the same __VA_ARGS__ mess described in FLY_FORMAT_STRING is used here.
 */
#define FLY_FORMAT_ARGS(...)                                                                       \
    FLY_FORMAT_ARGS_HELPER(FLY_FORMAT_ARGS_LABEL(__VA_ARGS__), (__VA_ARGS__))

//==================================================================================================
#define FLY_FORMAT_STRING_HELPER(args) FLY_FORMAT_STRING_HELPER2 args

#define FLY_FORMAT_STRING_HELPER2(first, ...) first

//==================================================================================================
#define FLY_FORMAT_ARGS_HELPER(label, args) FLY_FORMAT_ARGS_HELPER2(label, args)

#define FLY_FORMAT_ARGS_HELPER2(label, args) FLY_FORMAT_ARGS_HELPER_##label(args)

#define FLY_FORMAT_ARGS_HELPER_SINGLE(first)

#define FLY_FORMAT_ARGS_HELPER_MULTIPLE(args) FLY_FORMAT_ARGS_HELPER_MULTIPLE2 args

#define FLY_FORMAT_ARGS_HELPER_MULTIPLE2(first, ...) , __VA_ARGS__

#define FLY_FORMAT_ARGS_EXPAND(x) x

#define FLY_FORMAT_ARGS_LABEL(...)                                                                 \
    FLY_FORMAT_ARGS_EXPAND(FLY_FORMAT_ARGS_LABEL_HELPER(                                           \
        __VA_ARGS__,                                                                               \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        MULTIPLE,                                                                                  \
        SINGLE,                                                                                    \
        _))

#define FLY_FORMAT_ARGS_LABEL_HELPER(                                                              \
    ARG_01,                                                                                        \
    ARG_02,                                                                                        \
    ARG_03,                                                                                        \
    ARG_04,                                                                                        \
    ARG_05,                                                                                        \
    ARG_06,                                                                                        \
    ARG_07,                                                                                        \
    ARG_08,                                                                                        \
    ARG_09,                                                                                        \
    ARG_10,                                                                                        \
    ARG_11,                                                                                        \
    ARG_12,                                                                                        \
    ARG_13,                                                                                        \
    ARG_14,                                                                                        \
    ARG_15,                                                                                        \
    ARG_16,                                                                                        \
    ARG_17,                                                                                        \
    ARG_18,                                                                                        \
    ARG_19,                                                                                        \
    ARG_20,                                                                                        \
    ARG_21,                                                                                        \
    ARG_22,                                                                                        \
    ARG_23,                                                                                        \
    ARG_24,                                                                                        \
    ARG_25,                                                                                        \
    ARG_26,                                                                                        \
    ARG_27,                                                                                        \
    ARG_28,                                                                                        \
    ARG_29,                                                                                        \
    ARG_30,                                                                                        \
    ARG_31,                                                                                        \
    ARG_32,                                                                                        \
    ARG_33,                                                                                        \
    ARG_34,                                                                                        \
    ARG_35,                                                                                        \
    ARG_36,                                                                                        \
    ARG_37,                                                                                        \
    ARG_38,                                                                                        \
    ARG_39,                                                                                        \
    ARG_40,                                                                                        \
    ARG_41,                                                                                        \
    ARG_42,                                                                                        \
    ARG_43,                                                                                        \
    ARG_44,                                                                                        \
    ARG_45,                                                                                        \
    ARG_46,                                                                                        \
    ARG_47,                                                                                        \
    ARG_48,                                                                                        \
    ARG_49,                                                                                        \
    ARG_50,                                                                                        \
    ARG_51,                                                                                        \
    ...)                                                                                           \
    ARG_51

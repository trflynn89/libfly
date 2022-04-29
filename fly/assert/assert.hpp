#pragma once

#if defined(NDEBUG)
#    include "fly/assert/detail/assert_disabled.hpp"
#else
#    include "fly/assert/detail/assert_enabled.hpp"
#endif

#include <cstdlib>

/**
 * Assert that a boolean expression is true, aborting the application if the assertion fails.
 *
 * This macro is disabled if NDEBUG is defined.
 *
 * This macro may be invoked in several ways:
 *
 *     1. FLY_ASSERT(Boolean expression);
 *
 *        In its simplest form, this will simply evaluate the boolean expression, and log the
 *        location of the assertion and its call stack if the expression evaluates to false before
 *        aborting the application.
 *
 *     2. FLY_ASSERT(Boolean expression, "Message for debugging");
 *
 *        In addition to (1), this will display a message to the user alongside the failed
 *        assertion.
 *
 *     3. FLY_ASSERT(Boolean expression, some_variable, other_variable);
 *
 *        In addition to (1), this will capture variables to be displayed alongside the failed
 *        assertion. Any variable that may be formatted with fly::string::format may be captured.
 *
 *     3. FLY_ASSERT(Boolean expression, "Message for debugging", some_variable, other_variable);
 *
 *        A combination of (2) and (3).
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version April 24, 2022
 */
#define FLY_ASSERT(expression, ...)                                                                \
    FLY_ASSERT_IMPL(std::abort, expression __VA_OPT__(, ) __VA_ARGS__)

/**
 * Assert that a source code location is never reached. Similar to FLY_ASSERT(), this macro may be
 * invoked with an optional message and variables to be captured for debugging.
 */
#define FLY_ASSERT_NOT_REACHED(...) FLY_ASSERT(false __VA_OPT__(, ) __VA_ARGS__)

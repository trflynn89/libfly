#pragma once

#if defined(NDEBUG)
#    include "fly/assert/detail/assert_disabled.hpp"
#else
#    include "fly/assert/detail/assert_enabled.hpp"
#endif

#include <cstdint>
#include <string_view>

/**
 * Assert that a boolean expression is true, invoking the application-wide assertion callback if the
 * assertion fails. If no callback was specified, std::abort() will be invoked.
 *
 * This macro is disabled if NDEBUG is defined.
 *
 * This macro may be invoked in several ways:
 *
 *     1. FLY_ASSERT(boolean expression);
 *
 *        In its simplest form, this will simply evaluate the boolean expression, and log the
 *        location of the assertion and its call stack if the expression evaluates to false before
 *        invoking the application-wide assertion callback.
 *
 *     2. FLY_ASSERT(boolean expression, "Message for debugging");
 *
 *        In addition to (1), this will display a message to the user alongside the failed
 *        assertion.
 *
 *     3. FLY_ASSERT(boolean expression, some_variable, other_variable);
 *
 *        In addition to (1), this will capture variables to be displayed alongside the failed
 *        assertion. Any variable that may be formatted with fly::string::format may be captured.
 *
 *     3. FLY_ASSERT(boolean expression, "Message for debugging", some_variable, other_variable);
 *
 *        A combination of (2) and (3).
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version April 24, 2022
 */
#define FLY_ASSERT(expression, ...) FLY_ASSERT_IMPL(expression __VA_OPT__(, ) __VA_ARGS__)

/**
 * Assert that a source code location is never reached. Similar to FLY_ASSERT(), this macro may be
 * invoked with an optional message and variables to be captured for debugging.
 */
#define FLY_ASSERT_NOT_REACHED(...) FLY_ASSERT(false __VA_OPT__(, ) __VA_ARGS__)

namespace fly::assert {

/**
 * Signature of the application-wide callback which is invoked when an assertion has failed.
 *
 * @param expression The stringified failed assertion expression.
 * @param file The file in which the assertion failed.
 * @param function The function in which the assertion failed.
 * @param line The line on which the assertion failed.
 */
using AssertionHandler = void (*)(
    std::string_view expression,
    std::string_view file,
    std::string_view function,
    std::uint32_t line);

/**
 * Set the application-wide callback to invoke when an assertion fails. If never invoked, then
 * std::abort() will be invoked instead.
 *
 * @param handler The callback to invoke with assertion information.
 *
 * @return A pointer to the previously set callback (or null).
 */
AssertionHandler set_assertion_handler(AssertionHandler handler);

/**
 * Retrieve the application-wide callback to invoke when an assertion fails.
 *
 * @return A pointer to the currently set callback (or null).
 */
AssertionHandler assertion_handler();

} // namespace fly::assert

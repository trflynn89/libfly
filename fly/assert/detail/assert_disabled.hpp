#pragma once

namespace fly::detail {

/**
 * Helper to ensure all captured values are referenced but not evaluated.
 */
template <typename... Args>
constexpr void ignore_arguments(Args &&...)
{
}

} // namespace fly::detail

#define FLY_ASSERT_IMPL(handler, ...)                                                              \
    true ? static_cast<void>(0) : fly::detail::ignore_arguments(__VA_ARGS__)

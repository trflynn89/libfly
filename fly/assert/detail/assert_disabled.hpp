#pragma once

namespace fly::assert::detail {

/**
 * Helper to ensure all captured values are referenced but not evaluated.
 */
template <typename... Args>
constexpr void ignore_arguments(Args &&...)
{
}

} // namespace fly::assert::detail

#define FLY_ASSERT_IMPL(...)                                                                       \
    true ? static_cast<void>(0) : fly::assert::detail::ignore_arguments(__VA_ARGS__)

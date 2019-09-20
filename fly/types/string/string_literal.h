#pragma once

#include <filesystem>

/**
 * Helper macros to choose the correct string literal to use for either a given
 * type or the char type used on the compiling system.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version March 23, 2019
 */
#define FLY_STR(type, str)                                                     \
    (fly::BasicStringLiteral<type>::Literal(str, L##str, u##str, U##str))

#define FLY_SYS_STR(str) FLY_STR(std::filesystem::path::value_type, str)

namespace fly {

//==============================================================================
template <typename CharType>
struct BasicStringLiteral;

//==============================================================================
template <>
struct BasicStringLiteral<char>
{
    static constexpr const char *Literal(
        const char *str,
        const wchar_t *,
        const char16_t *,
        const char32_t *)
    {
        return str;
    }
};

//==============================================================================
template <>
struct BasicStringLiteral<wchar_t>
{
    static constexpr const wchar_t *Literal(
        const char *,
        const wchar_t *str,
        const char16_t *,
        const char32_t *)
    {
        return str;
    }
};

//==============================================================================
template <>
struct BasicStringLiteral<char16_t>
{
    static constexpr const char16_t *Literal(
        const char *,
        const wchar_t *,
        const char16_t *str,
        const char32_t *)
    {
        return str;
    }
};

//==============================================================================
template <>
struct BasicStringLiteral<char32_t>
{
    static constexpr const char32_t *Literal(
        const char *,
        const wchar_t *,
        const char16_t *,
        const char32_t *str)
    {
        return str;
    }
};

} // namespace fly

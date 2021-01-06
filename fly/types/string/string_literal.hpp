#pragma once

#include <filesystem>

/**
 * Helper macros to choose the correct string literal prefix to use for either a given type or the
 * character type used on the compiling system.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version March 23, 2019
 */
#define FLY_CHR(type, ch)                                                                          \
    (fly::detail::BasicCharacterLiteral<type>::value(ch, L##ch, u8##ch, u##ch, U##ch))
#define FLY_STR(type, str)                                                                         \
    (fly::detail::BasicStringLiteral<type>::value(str, L##str, u8##str, u##str, U##str))
#define FLY_ARR(type, arr)                                                                         \
    (fly::detail::BasicStringArray<type>::value(arr, L##arr, u8##arr, u##arr, U##arr))

#define FLY_SYS_CHR(str) FLY_CH(std::filesystem::path::value_type, str)
#define FLY_SYS_STR(str) FLY_STR(std::filesystem::path::value_type, str)
#define FLY_SYS_ARR(arr) FLY_ARR(std::filesystem::path::value_type, arr)

namespace fly::detail {

//==================================================================================================
template <typename CharType>
struct BasicCharacterLiteral;

template <typename CharType>
struct BasicStringLiteral;

template <typename CharType>
struct BasicStringArray;

//==================================================================================================
template <>
struct BasicCharacterLiteral<char>
{
    static constexpr auto
    value(const char ch, const wchar_t, const char8_t, const char16_t, const char32_t)
    {
        return ch;
    }
};

//==================================================================================================
template <>
struct BasicCharacterLiteral<wchar_t>
{
    static constexpr auto
    value(const char, const wchar_t ch, const char8_t, const char16_t, const char32_t)
    {
        return ch;
    }
};

//==================================================================================================
template <>
struct BasicCharacterLiteral<char8_t>
{
    static constexpr auto
    value(const char, const wchar_t, const char8_t ch, const char16_t, const char32_t)
    {
        return ch;
    }
};

//==================================================================================================
template <>
struct BasicCharacterLiteral<char16_t>
{
    static constexpr auto
    value(const char, const wchar_t, const char8_t, const char16_t ch, const char32_t)
    {
        return ch;
    }
};

//==================================================================================================
template <>
struct BasicCharacterLiteral<char32_t>
{
    static constexpr auto
    value(const char, const wchar_t, const char8_t, const char16_t, const char32_t ch)
    {
        return ch;
    }
};

//==================================================================================================
template <>
struct BasicStringLiteral<char>
{
    static constexpr auto
    value(const char *str, const wchar_t *, const char8_t *, const char16_t *, const char32_t *)
    {
        return str;
    }
};

//==================================================================================================
template <>
struct BasicStringLiteral<wchar_t>
{
    static constexpr auto
    value(const char *, const wchar_t *str, const char8_t *, const char16_t *, const char32_t *)
    {
        return str;
    }
};

//==================================================================================================
template <>
struct BasicStringLiteral<char8_t>
{
    static constexpr auto
    value(const char *, const wchar_t *, const char8_t *str, const char16_t *, const char32_t *)
    {
        return str;
    }
};

//==================================================================================================
template <>
struct BasicStringLiteral<char16_t>
{
    static constexpr auto
    value(const char *, const wchar_t *, const char8_t *, const char16_t *str, const char32_t *)
    {
        return str;
    }
};

//==================================================================================================
template <>
struct BasicStringLiteral<char32_t>
{
    static constexpr auto
    value(const char *, const wchar_t *, const char8_t *, const char16_t *, const char32_t *str)
    {
        return str;
    }
};

//==================================================================================================
template <>
struct BasicStringArray<char>
{
    template <std::size_t N>
    static constexpr auto value(
        const char (&arr)[N],
        const wchar_t (&)[N],
        const char8_t (&)[N],
        const char16_t (&)[N],
        const char32_t (&)[N]) -> decltype(arr)
    {
        return arr;
    }
};

//==================================================================================================
template <>
struct BasicStringArray<wchar_t>
{
    template <std::size_t N>
    static constexpr auto value(
        const char (&)[N],
        const wchar_t (&arr)[N],
        const char8_t (&)[N],
        const char16_t (&)[N],
        const char32_t (&)[N]) -> decltype(arr)
    {
        return arr;
    }
};

//==================================================================================================
template <>
struct BasicStringArray<char8_t>
{
    template <std::size_t N>
    static constexpr auto value(
        const char (&)[N],
        const wchar_t (&)[N],
        const char8_t (&arr)[N],
        const char16_t (&)[N],
        const char32_t (&)[N]) -> decltype(arr)
    {
        return arr;
    }
};

//==================================================================================================
template <>
struct BasicStringArray<char16_t>
{
    template <std::size_t N>
    static constexpr auto value(
        const char (&)[N],
        const wchar_t (&)[N],
        const char8_t (&)[N],
        const char16_t (&arr)[N],
        const char32_t (&)[N]) -> decltype(arr)
    {
        return arr;
    }
};

//==================================================================================================
template <>
struct BasicStringArray<char32_t>
{
    template <std::size_t N>
    static constexpr auto value(
        const char (&)[N],
        const wchar_t (&)[N],
        const char8_t (&)[N],
        const char16_t (&)[N],
        const char32_t (&arr)[N]) -> decltype(arr)
    {
        return arr;
    }
};

} // namespace fly::detail

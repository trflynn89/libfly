#pragma once

#include "fly/fly.hpp"
#include "fly/types/string/concepts.hpp"

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

#if defined(FLY_LINUX) || defined(FLY_MACOS)
#    define FLY_SYS_CHR(str) FLY_CH(char, str)
#    define FLY_SYS_STR(str) FLY_STR(char, str)
#    define FLY_SYS_ARR(arr) FLY_ARR(char, arr)
#elif defined(FLY_WINDOWS)
#    define FLY_SYS_CHR(str) FLY_CH(wchar_t, str)
#    define FLY_SYS_STR(str) FLY_STR(wchar_t, str)
#    define FLY_SYS_ARR(arr) FLY_ARR(wchar_t, arr)
#else
#    error Unknown system character type.
#endif

namespace fly::detail {

//==================================================================================================
template <StandardCharacter CharType>
struct BasicCharacterLiteral;

template <StandardCharacter CharType>
struct BasicStringLiteral;

template <StandardCharacter CharType>
struct BasicStringArray;

//==================================================================================================
template <>
struct BasicCharacterLiteral<char>
{
    static constexpr auto
    value(char const ch, wchar_t const, char8_t const, char16_t const, char32_t const)
    {
        return ch;
    }
};

//==================================================================================================
template <>
struct BasicCharacterLiteral<wchar_t>
{
    static constexpr auto
    value(char const, wchar_t const ch, char8_t const, char16_t const, char32_t const)
    {
        return ch;
    }
};

//==================================================================================================
template <>
struct BasicCharacterLiteral<char8_t>
{
    static constexpr auto
    value(char const, wchar_t const, char8_t const ch, char16_t const, char32_t const)
    {
        return ch;
    }
};

//==================================================================================================
template <>
struct BasicCharacterLiteral<char16_t>
{
    static constexpr auto
    value(char const, wchar_t const, char8_t const, char16_t const ch, char32_t const)
    {
        return ch;
    }
};

//==================================================================================================
template <>
struct BasicCharacterLiteral<char32_t>
{
    static constexpr auto
    value(char const, wchar_t const, char8_t const, char16_t const, char32_t const ch)
    {
        return ch;
    }
};

//==================================================================================================
template <>
struct BasicStringLiteral<char>
{
    static constexpr auto
    value(char const *str, wchar_t const *, char8_t const *, char16_t const *, char32_t const *)
    {
        return str;
    }
};

//==================================================================================================
template <>
struct BasicStringLiteral<wchar_t>
{
    static constexpr auto
    value(char const *, wchar_t const *str, char8_t const *, char16_t const *, char32_t const *)
    {
        return str;
    }
};

//==================================================================================================
template <>
struct BasicStringLiteral<char8_t>
{
    static constexpr auto
    value(char const *, wchar_t const *, char8_t const *str, char16_t const *, char32_t const *)
    {
        return str;
    }
};

//==================================================================================================
template <>
struct BasicStringLiteral<char16_t>
{
    static constexpr auto
    value(char const *, wchar_t const *, char8_t const *, char16_t const *str, char32_t const *)
    {
        return str;
    }
};

//==================================================================================================
template <>
struct BasicStringLiteral<char32_t>
{
    static constexpr auto
    value(char const *, wchar_t const *, char8_t const *, char16_t const *, char32_t const *str)
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
        char const (&arr)[N],
        wchar_t const (&)[N],
        char8_t const (&)[N],
        char16_t const (&)[N],
        char32_t const (&)[N]) -> decltype(arr)
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
        char const (&)[N],
        wchar_t const (&arr)[N],
        char8_t const (&)[N],
        char16_t const (&)[N],
        char32_t const (&)[N]) -> decltype(arr)
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
        char const (&)[N],
        wchar_t const (&)[N],
        char8_t const (&arr)[N],
        char16_t const (&)[N],
        char32_t const (&)[N]) -> decltype(arr)
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
        char const (&)[N],
        wchar_t const (&)[N],
        char8_t const (&)[N],
        char16_t const (&arr)[N],
        char32_t const (&)[N]) -> decltype(arr)
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
        char const (&)[N],
        wchar_t const (&)[N],
        char8_t const (&)[N],
        char16_t const (&)[N],
        char32_t const (&arr)[N]) -> decltype(arr)
    {
        return arr;
    }
};

} // namespace fly::detail

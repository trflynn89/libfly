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
    (fly::BasicCharacterLiteral<type>::literal(ch, L##ch, u8##ch, u##ch, U##ch))
#define FLY_STR(type, str)                                                                         \
    (fly::BasicStringLiteral<type>::literal(str, L##str, u8##str, u##str, U##str))

#define FLY_SYS_CHR(str) FLY_CH(std::filesystem::path::value_type, str)
#define FLY_SYS_STR(str) FLY_STR(std::filesystem::path::value_type, str)

namespace fly {

//==================================================================================================
template <typename CharType>
struct BasicCharacterLiteral;

template <typename CharType>
struct BasicStringLiteral;

//==================================================================================================
template <>
struct BasicCharacterLiteral<char>
{
    static constexpr char
    literal(const char ch, const wchar_t, const char8_t, const char16_t, const char32_t)
    {
        return ch;
    }
};

//==================================================================================================
template <>
struct BasicCharacterLiteral<wchar_t>
{
    static constexpr wchar_t
    literal(const char, const wchar_t ch, const char8_t, const char16_t, const char32_t)
    {
        return ch;
    }
};

//==================================================================================================
template <>
struct BasicCharacterLiteral<char8_t>
{
    static constexpr char8_t
    literal(const char, const wchar_t, const char8_t ch, const char16_t, const char32_t)
    {
        return ch;
    }
};

//==================================================================================================
template <>
struct BasicCharacterLiteral<char16_t>
{
    static constexpr char16_t
    literal(const char, const wchar_t, const char8_t, const char16_t ch, const char32_t)
    {
        return ch;
    }
};

//==================================================================================================
template <>
struct BasicCharacterLiteral<char32_t>
{
    static constexpr char32_t
    literal(const char, const wchar_t, const char8_t, const char16_t, const char32_t ch)
    {
        return ch;
    }
};

//==================================================================================================
template <>
struct BasicStringLiteral<char>
{
    static constexpr const char *
    literal(const char *str, const wchar_t *, const char8_t *, const char16_t *, const char32_t *)
    {
        return str;
    }
};

//==================================================================================================
template <>
struct BasicStringLiteral<wchar_t>
{
    static constexpr const wchar_t *
    literal(const char *, const wchar_t *str, const char8_t *, const char16_t *, const char32_t *)
    {
        return str;
    }
};

//==================================================================================================
template <>
struct BasicStringLiteral<char8_t>
{
    static constexpr const char8_t *
    literal(const char *, const wchar_t *, const char8_t *str, const char16_t *, const char32_t *)
    {
        return str;
    }
};

//==================================================================================================
template <>
struct BasicStringLiteral<char16_t>
{
    static constexpr const char16_t *
    literal(const char *, const wchar_t *, const char8_t *, const char16_t *str, const char32_t *)
    {
        return str;
    }
};

//==================================================================================================
template <>
struct BasicStringLiteral<char32_t>
{
    static constexpr const char32_t *
    literal(const char *, const wchar_t *, const char8_t *, const char16_t *, const char32_t *str)
    {
        return str;
    }
};

} // namespace fly

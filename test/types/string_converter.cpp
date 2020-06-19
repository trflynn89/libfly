#include "fly/types/numeric/literals.hpp"
#include "fly/types/string/string.hpp"
#include "test/types/string_test.hpp"

#include <gtest/gtest.h>

#include <limits>
#include <string>
#include <type_traits>
#include <vector>

namespace {

//==================================================================================================
template <typename T, typename StringType>
void validate_pass(StringType test, T expected)
{
    auto actual = fly::BasicString<StringType>::template convert<T>(std::move(test));
    ASSERT_TRUE(actual.has_value());

    EXPECT_EQ(actual.value(), expected);
}

//==================================================================================================
template <typename T, typename StringType>
void validate_fail(StringType test)
{
    auto actual = fly::BasicString<StringType>::template convert<T>(std::move(test));
    EXPECT_FALSE(actual.has_value());
}

//==================================================================================================
template <typename StringType, typename T>
StringType minstr()
{
    static constexpr std::intmax_t s_min = std::numeric_limits<T>::min();

    if constexpr (std::is_same_v<StringType, std::string>)
    {
        return std::to_string(s_min - 1);
    }
    else if constexpr (std::is_same_v<StringType, std::wstring>)
    {
        return std::to_wstring(s_min - 1);
    }
}

//==================================================================================================
template <typename StringType, typename T>
StringType maxstr()
{
    static constexpr std::uintmax_t s_max = std::numeric_limits<T>::max();

    if constexpr (std::is_same_v<StringType, std::string>)
    {
        return std::to_string(s_max + 1);
    }
    else if constexpr (std::is_same_v<StringType, std::wstring>)
    {
        return std::to_wstring(s_max + 1);
    }
}

//==================================================================================================
template <typename StringType>
StringType out_of_range_codepoint()
{
    using char_type = typename StringType::value_type;

    static constexpr const std::uint32_t out_of_range = 0x110000;
    StringType result;

    if constexpr (sizeof(char_type) == 1)
    {
        result += static_cast<char_type>(0xf0 | (out_of_range >> 18));
        result += static_cast<char_type>(0x80 | ((out_of_range >> 12) & 0x3f));
        result += static_cast<char_type>(0x80 | ((out_of_range >> 6) & 0x3f));
        result += static_cast<char_type>(0x80 | (out_of_range & 0x3f));
    }
    else if constexpr (sizeof(char_type) == 2)
    {
        result += static_cast<char_type>(0xd800 | ((out_of_range - 0x10000) >> 10));
        result += static_cast<char_type>(0xdc00 | ((out_of_range - 0x10000) & 0x3ff));
    }
    else if constexpr (sizeof(char_type) == 4)
    {
        result = StringType(1, static_cast<char_type>(out_of_range));
    }

    return result;
}

} // namespace

//==================================================================================================
TYPED_TEST(BasicStringTest, ConvertString)
{
    DECLARE_ALIASES

    string_type s = FLY_STR(char_type, "abc");
    validate_pass<string_type>(string_type(s), s);

    const char_type *c = FLY_STR(char_type, "def");
    validate_pass<string_type>(string_type(c), c);

    char_type *d = const_cast<char_type *>(FLY_STR(char_type, "ghi"));
    validate_pass<string_type>(string_type(d), d);
}

//==================================================================================================
TYPED_TEST(BasicStringTest, ConvertToUTF8)
{
    DECLARE_ALIASES

    string_type test = FLY_STR(char_type, "\U0001f355 in the morning");
    {
        auto utf8 = FLY_STR(std::string::value_type, "\U0001f355 in the morning");
        validate_pass<std::string>(test, utf8);
    }
    {
        const auto utf8 = FLY_STR(std::string::value_type, "\U0001f355 in the morning");
        validate_pass<std::string>(test, utf8);
    }

    validate_fail<std::string>(out_of_range_codepoint<string_type>());
}

//==================================================================================================
TYPED_TEST(BasicStringTest, ConvertToUTF16)
{
    DECLARE_ALIASES

    string_type test = FLY_STR(char_type, "\U0001f355 in the morning");
    {
        auto utf16 = FLY_STR(std::u16string::value_type, "\U0001f355 in the morning");
        validate_pass<std::u16string>(test, utf16);
    }
    {
        const auto utf16 = FLY_STR(std::u16string::value_type, "\U0001f355 in the morning");
        validate_pass<std::u16string>(test, utf16);
    }

    validate_fail<std::u16string>(out_of_range_codepoint<string_type>());

    if constexpr (sizeof(std::wstring::value_type) == 2)
    {
        {
            auto utf16 = FLY_STR(std::wstring::value_type, "\U0001f355 in the morning");
            validate_pass<std::wstring>(test, utf16);
        }
        {
            const auto utf16 = FLY_STR(std::wstring::value_type, "\U0001f355 in the morning");
            validate_pass<std::wstring>(test, utf16);
        }

        validate_fail<std::wstring>(out_of_range_codepoint<string_type>());
    }
}

//==================================================================================================
TYPED_TEST(BasicStringTest, ConvertToUTF32)
{
    DECLARE_ALIASES

    string_type test = FLY_STR(char_type, "\U0001f355 in the morning");
    {
        auto utf32 = FLY_STR(std::u32string::value_type, "\U0001f355 in the morning");
        validate_pass<std::u32string>(test, utf32);
    }
    {
        const auto utf32 = FLY_STR(std::u32string::value_type, "\U0001f355 in the morning");
        validate_pass<std::u32string>(test, utf32);
    }

    validate_fail<std::u32string>(out_of_range_codepoint<string_type>());

    if constexpr (sizeof(std::wstring::value_type) == 4)
    {
        {
            auto utf32 = FLY_STR(std::wstring::value_type, "\U0001f355 in the morning");
            validate_pass<std::wstring>(test, utf32);
        }
        {
            const auto utf32 = FLY_STR(std::wstring::value_type, "\U0001f355 in the morning");
            validate_pass<std::wstring>(test, utf32);
        }

        validate_fail<std::wstring>(out_of_range_codepoint<string_type>());
    }
}

//==================================================================================================
TYPED_TEST(BasicStringTest, ConvertBool)
{
    DECLARE_ALIASES

    string_type s;

    s = FLY_STR(char_type, "0");
    validate_pass<bool>(s, false);

    s = FLY_STR(char_type, "1");
    validate_pass<bool>(s, true);

    s = FLY_STR(char_type, "-1");
    validate_fail<bool>(s);

    s = FLY_STR(char_type, "2");
    validate_fail<bool>(s);

    s = FLY_STR(char_type, "abc");
    validate_fail<bool>(s);

    s = FLY_STR(char_type, "2a");
    validate_fail<bool>(s);
}

//==================================================================================================
TYPED_TEST(BasicStringTest, ConvertChar)
{
    DECLARE_ALIASES

    string_type s;

    s = FLY_STR(char_type, "0");
    validate_pass<streamed_char>(s, '\0');
    validate_pass<ustreamed_char>(s, '\0');

    s = FLY_STR(char_type, "65");
    validate_pass<streamed_char>(s, 'A');
    validate_pass<ustreamed_char>(s, static_cast<ustreamed_char>(65));

    s = FLY_STR(char_type, "abc");
    validate_fail<streamed_char>(s);
    validate_fail<ustreamed_char>(s);

    s = FLY_STR(char_type, "2a");
    validate_fail<streamed_char>(s);
    validate_fail<ustreamed_char>(s);

    if constexpr (StringClass::traits::has_stoi_family_v)
    {
        validate_fail<streamed_char>(minstr<string_type, streamed_char>());
        validate_fail<streamed_char>(maxstr<string_type, streamed_char>());

        validate_fail<ustreamed_char>(minstr<string_type, ustreamed_char>());
        validate_fail<ustreamed_char>(maxstr<string_type, ustreamed_char>());
    }
}

//==================================================================================================
TYPED_TEST(BasicStringTest, ConvertInt8)
{
    DECLARE_ALIASES

    string_type s;

    s = FLY_STR(char_type, "0");
    validate_pass<std::int8_t>(s, 0_i8);
    validate_pass<std::uint8_t>(s, 0_u8);

    s = FLY_STR(char_type, "100");
    validate_pass<std::int8_t>(s, 100_i8);
    validate_pass<std::uint8_t>(s, 100_u8);

    s = FLY_STR(char_type, "-100");
    validate_pass<std::int8_t>(s, -100_i8);
    validate_fail<std::uint8_t>(s);

    s = FLY_STR(char_type, "abc");
    validate_fail<std::int8_t>(s);
    validate_fail<std::uint8_t>(s);

    s = FLY_STR(char_type, "2a");
    validate_fail<std::int8_t>(s);
    validate_fail<std::uint8_t>(s);

    if constexpr (StringClass::traits::has_stoi_family_v)
    {
        validate_fail<std::int8_t>(minstr<string_type, std::int8_t>());
        validate_fail<std::int8_t>(maxstr<string_type, std::int8_t>());

        validate_fail<std::uint8_t>(minstr<string_type, std::uint8_t>());
        validate_fail<std::uint8_t>(maxstr<string_type, std::uint8_t>());
    }
}

//==================================================================================================
TYPED_TEST(BasicStringTest, ConvertInt16)
{
    DECLARE_ALIASES

    string_type s;

    s = FLY_STR(char_type, "0");
    validate_pass<std::int16_t>(s, 0_i16);
    validate_pass<std::uint16_t>(s, 0_u16);

    s = FLY_STR(char_type, "100");
    validate_pass<std::int16_t>(s, 100_i16);
    validate_pass<std::uint16_t>(s, 100_u16);

    s = FLY_STR(char_type, "-100");
    validate_pass<std::int16_t>(s, -100_i16);
    validate_fail<std::uint16_t>(s);

    s = FLY_STR(char_type, "abc");
    validate_fail<std::int16_t>(s);
    validate_fail<std::uint16_t>(s);

    s = FLY_STR(char_type, "2a");
    validate_fail<std::int16_t>(s);
    validate_fail<std::uint16_t>(s);

    if constexpr (StringClass::traits::has_stoi_family_v)
    {
        validate_fail<std::int16_t>(minstr<string_type, std::int16_t>());
        validate_fail<std::int16_t>(maxstr<string_type, std::int16_t>());

        validate_fail<std::uint16_t>(minstr<string_type, std::uint16_t>());
        validate_fail<std::uint16_t>(maxstr<string_type, std::uint16_t>());
    }
}

//==================================================================================================
TYPED_TEST(BasicStringTest, ConvertInt32)
{
    DECLARE_ALIASES

    string_type s;

    s = FLY_STR(char_type, "0");
    validate_pass<std::int32_t>(s, 0_i32);
    validate_pass<std::uint32_t>(s, 0_u32);

    s = FLY_STR(char_type, "100");
    validate_pass<std::int32_t>(s, 100_i32);
    validate_pass<std::uint32_t>(s, 100_u32);

    s = FLY_STR(char_type, "-100");
    validate_pass<std::int32_t>(s, -100_i32);
    validate_fail<std::uint32_t>(s);

    s = FLY_STR(char_type, "abc");
    validate_fail<std::int32_t>(s);
    validate_fail<std::uint32_t>(s);

    s = FLY_STR(char_type, "2a");
    validate_fail<std::int32_t>(s);
    validate_fail<std::uint32_t>(s);

    if constexpr (StringClass::traits::has_stoi_family_v)
    {
        validate_fail<std::int32_t>(minstr<string_type, std::int32_t>());
        validate_fail<std::int32_t>(maxstr<string_type, std::int32_t>());

        validate_fail<std::uint32_t>(minstr<string_type, std::uint32_t>());
        validate_fail<std::uint32_t>(maxstr<string_type, std::uint32_t>());
    }
}

//==================================================================================================
TYPED_TEST(BasicStringTest, ConvertInt64)
{
    DECLARE_ALIASES

    string_type s;

    s = FLY_STR(char_type, "0");
    validate_pass<std::int64_t>(s, 0_i64);
    validate_pass<std::uint64_t>(s, 0_u64);

    s = FLY_STR(char_type, "100");
    validate_pass<std::int64_t>(s, 100_i64);
    validate_pass<std::uint64_t>(s, 100_u64);

    s = FLY_STR(char_type, "-100");
    validate_pass<std::int64_t>(s, -100_i64);

    s = FLY_STR(char_type, "abc");
    validate_fail<std::int64_t>(s);
    validate_fail<std::uint64_t>(s);

    s = FLY_STR(char_type, "2a");
    validate_fail<std::int64_t>(s);
    validate_fail<std::uint64_t>(s);
}

//==================================================================================================
TYPED_TEST(BasicStringTest, ConvertDecimal)
{
    DECLARE_ALIASES

    string_type s;

    s = FLY_STR(char_type, "-400.123");
    validate_pass<float>(s, -400.123f);
    validate_pass<double>(s, -400.123);
    validate_pass<long double>(s, -400.123L);

    s = FLY_STR(char_type, "400.456");
    validate_pass<float>(s, 400.456f);
    validate_pass<double>(s, 400.456);
    validate_pass<long double>(s, 400.456L);

    s = FLY_STR(char_type, "abc");
    validate_fail<float>(s);
    validate_fail<double>(s);
    validate_fail<long double>(s);

    s = FLY_STR(char_type, "2a");
    validate_fail<float>(s);
    validate_fail<double>(s);
    validate_fail<long double>(s);
}

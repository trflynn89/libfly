#include "fly/types/numeric/literals.hpp"
#include "fly/types/string/string.hpp"

#include "catch2/catch.hpp"

#include <limits>
#include <string>
#include <type_traits>

using namespace fly::literals::numeric_literals;

namespace {

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

} // namespace

CATCH_TEMPLATE_TEST_CASE(
    "BasicStringConverter",
    "[string]",
    std::string,
    std::wstring,
    std::u8string,
    std::u16string,
    std::u32string)
{
    using StringType = TestType;
    using BasicString = fly::BasicString<StringType>;
    using BasicStringTraits = fly::detail::BasicStringTraits<StringType>;
    using char_type = typename BasicString::char_type;
    using codepoint_type = typename BasicString::codepoint_type;
    using streamed_type = typename BasicString::streamed_type;
    using streamed_char = typename streamed_type::value_type;
    using ustreamed_char = std::make_unsigned_t<streamed_char>;

    auto out_of_range_codepoint = []() -> StringType
    {
        static constexpr const codepoint_type s_out_of_range = 0x110000;
        StringType result;

        if constexpr (sizeof(char_type) == 1)
        {
            result += static_cast<char_type>(0xf0 | (s_out_of_range >> 18));
            result += static_cast<char_type>(0x80 | ((s_out_of_range >> 12) & 0x3f));
            result += static_cast<char_type>(0x80 | ((s_out_of_range >> 6) & 0x3f));
            result += static_cast<char_type>(0x80 | (s_out_of_range & 0x3f));
        }
        else if constexpr (sizeof(char_type) == 2)
        {
            result += static_cast<char_type>(0xd800 | ((s_out_of_range - 0x10000) >> 10));
            result += static_cast<char_type>(0xdc00 | ((s_out_of_range - 0x10000) & 0x3ff));
        }
        else if constexpr (sizeof(char_type) == 4)
        {
            result = StringType(1, static_cast<char_type>(s_out_of_range));
        }

        return result;
    };

    CATCH_SECTION("Convert a string-like type to a standard string type")
    {
        StringType s = FLY_STR(char_type, "abc");
        CATCH_CHECK(BasicString::template convert<StringType>(s) == s);

        const char_type *c = FLY_STR(char_type, "def");
        CATCH_CHECK(BasicString::template convert<StringType>(c) == c);

        char_type *d = const_cast<char_type *>(FLY_STR(char_type, "ghi"));
        CATCH_CHECK(BasicString::template convert<StringType>(d) == d);
    }

    CATCH_SECTION("Convert a string to a UTF-8 encoded string")
    {
        StringType test = FLY_STR(char_type, "\U0001f355 in the morning");
        {
            auto utf8 = FLY_STR(std::string::value_type, "\U0001f355 in the morning");
            CATCH_CHECK(BasicString::template convert<std::string>(test) == utf8);
        }
        {
            const auto utf8 = FLY_STR(std::string::value_type, "\U0001f355 in the morning");
            CATCH_CHECK(BasicString::template convert<std::string>(test) == utf8);
        }
        {
            auto utf8 = FLY_STR(std::u8string::value_type, "\U0001f355 in the morning");
            CATCH_CHECK(BasicString::template convert<std::u8string>(test) == utf8);
        }
        {
            const auto utf8 = FLY_STR(std::u8string::value_type, "\U0001f355 in the morning");
            CATCH_CHECK(BasicString::template convert<std::u8string>(test) == utf8);
        }

        CATCH_CHECK_FALSE(BasicString::template convert<int>(out_of_range_codepoint()));
        CATCH_CHECK_FALSE(BasicString::template convert<std::string>(out_of_range_codepoint()));
        CATCH_CHECK_FALSE(BasicString::template convert<std::u8string>(out_of_range_codepoint()));
    }

    CATCH_SECTION("Convert a string to a UTF-16 encoded string")
    {
        StringType test = FLY_STR(char_type, "\U0001f355 in the morning");
        {
            auto utf16 = FLY_STR(std::u16string::value_type, "\U0001f355 in the morning");
            CATCH_CHECK(BasicString::template convert<std::u16string>(test) == utf16);
        }
        {
            const auto utf16 = FLY_STR(std::u16string::value_type, "\U0001f355 in the morning");
            CATCH_CHECK(BasicString::template convert<std::u16string>(test) == utf16);
        }

        CATCH_CHECK_FALSE(BasicString::template convert<int>(out_of_range_codepoint()));
        CATCH_CHECK_FALSE(BasicString::template convert<std::u16string>(out_of_range_codepoint()));

        if constexpr (sizeof(std::wstring::value_type) == 2)
        {
            {
                auto utf16 = FLY_STR(std::wstring::value_type, "\U0001f355 in the morning");
                CATCH_CHECK(BasicString::template convert<std::wstring>(test) == utf16);
            }
            {
                const auto utf16 = FLY_STR(std::wstring::value_type, "\U0001f355 in the morning");
                CATCH_CHECK(BasicString::template convert<std::wstring>(test) == utf16);
            }

            CATCH_CHECK_FALSE(
                BasicString::template convert<std::wstring>(out_of_range_codepoint()));
        }
    }

    CATCH_SECTION("Convert a string to a UTF-32 encoded string")
    {
        StringType test = FLY_STR(char_type, "\U0001f355 in the morning");
        {
            auto utf32 = FLY_STR(std::u32string::value_type, "\U0001f355 in the morning");
            CATCH_CHECK(BasicString::template convert<std::u32string>(test) == utf32);
        }
        {
            const auto utf32 = FLY_STR(std::u32string::value_type, "\U0001f355 in the morning");
            CATCH_CHECK(BasicString::template convert<std::u32string>(test) == utf32);
        }

        CATCH_CHECK_FALSE(BasicString::template convert<int>(out_of_range_codepoint()));
        CATCH_CHECK_FALSE(BasicString::template convert<std::u32string>(out_of_range_codepoint()));

        if constexpr (sizeof(std::wstring::value_type) == 4)
        {
            {
                auto utf32 = FLY_STR(std::wstring::value_type, "\U0001f355 in the morning");
                CATCH_CHECK(BasicString::template convert<std::wstring>(test) == utf32);
            }
            {
                const auto utf32 = FLY_STR(std::wstring::value_type, "\U0001f355 in the morning");
                CATCH_CHECK(BasicString::template convert<std::wstring>(test) == utf32);
            }

            CATCH_CHECK_FALSE(
                BasicString::template convert<std::wstring>(out_of_range_codepoint()));
        }
    }

    CATCH_SECTION("Convert a string to a Boolean")
    {
        StringType s;

        s = FLY_STR(char_type, "0");
        CATCH_CHECK(BasicString::template convert<bool>(s) == false);

        s = FLY_STR(char_type, "1");
        CATCH_CHECK(BasicString::template convert<bool>(s) == true);

        s = FLY_STR(char_type, "-1");
        CATCH_CHECK_FALSE(BasicString::template convert<bool>(s));

        s = FLY_STR(char_type, "2");
        CATCH_CHECK_FALSE(BasicString::template convert<bool>(s));

        s = FLY_STR(char_type, "abc");
        CATCH_CHECK_FALSE(BasicString::template convert<bool>(s));

        s = FLY_STR(char_type, "2a");
        CATCH_CHECK_FALSE(BasicString::template convert<bool>(s));
    }

    CATCH_SECTION("Convert a string to a streamable character type")
    {
        StringType s;

        s = FLY_STR(char_type, "0");
        CATCH_CHECK(BasicString::template convert<streamed_char>(s) == '\0');
        CATCH_CHECK(BasicString::template convert<ustreamed_char>(s) == '\0');

        s = FLY_STR(char_type, "65");
        CATCH_CHECK(BasicString::template convert<streamed_char>(s) == 'A');
        CATCH_CHECK(
            BasicString::template convert<ustreamed_char>(s) == static_cast<ustreamed_char>(65));

        s = FLY_STR(char_type, "abc");
        CATCH_CHECK_FALSE(BasicString::template convert<streamed_char>(s));
        CATCH_CHECK_FALSE(BasicString::template convert<ustreamed_char>(s));

        s = FLY_STR(char_type, "2a");
        CATCH_CHECK_FALSE(BasicString::template convert<streamed_char>(s));
        CATCH_CHECK_FALSE(BasicString::template convert<ustreamed_char>(s));

        if constexpr (BasicStringTraits::has_stoi_family_v)
        {
            CATCH_CHECK_FALSE(
                BasicString::template convert<streamed_char>(minstr<StringType, streamed_char>()));
            CATCH_CHECK_FALSE(
                BasicString::template convert<streamed_char>(maxstr<StringType, streamed_char>()));

            CATCH_CHECK_FALSE(BasicString::template convert<ustreamed_char>(
                minstr<StringType, ustreamed_char>()));
            CATCH_CHECK_FALSE(BasicString::template convert<ustreamed_char>(
                maxstr<StringType, ustreamed_char>()));
        }
    }

    CATCH_SECTION("Convert a string to an 8-bit integer")
    {
        StringType s;

        s = FLY_STR(char_type, "0");
        CATCH_CHECK(BasicString::template convert<std::int8_t>(s) == 0_i8);
        CATCH_CHECK(BasicString::template convert<std::uint8_t>(s) == 0_u8);

        s = FLY_STR(char_type, "100");
        CATCH_CHECK(BasicString::template convert<std::int8_t>(s) == 100_i8);
        CATCH_CHECK(BasicString::template convert<std::uint8_t>(s) == 100_u8);

        s = FLY_STR(char_type, "-100");
        CATCH_CHECK(BasicString::template convert<std::int8_t>(s) == -100_i8);
        CATCH_CHECK_FALSE(BasicString::template convert<std::uint8_t>(s));

        s = FLY_STR(char_type, "abc");
        CATCH_CHECK_FALSE(BasicString::template convert<std::int8_t>(s));
        CATCH_CHECK_FALSE(BasicString::template convert<std::uint8_t>(s));

        s = FLY_STR(char_type, "2a");
        CATCH_CHECK_FALSE(BasicString::template convert<std::int8_t>(s));
        CATCH_CHECK_FALSE(BasicString::template convert<std::uint8_t>(s));

        if constexpr (BasicStringTraits::has_stoi_family_v)
        {
            CATCH_CHECK_FALSE(
                BasicString::template convert<std::int8_t>(minstr<StringType, std::int8_t>()));
            CATCH_CHECK_FALSE(
                BasicString::template convert<std::int8_t>(maxstr<StringType, std::int8_t>()));

            CATCH_CHECK_FALSE(
                BasicString::template convert<std::uint8_t>(minstr<StringType, std::uint8_t>()));
            CATCH_CHECK_FALSE(
                BasicString::template convert<std::uint8_t>(maxstr<StringType, std::uint8_t>()));
        }
    }

    CATCH_SECTION("Convert a string to a 16-bit integer")
    {
        StringType s;

        s = FLY_STR(char_type, "0");
        CATCH_CHECK(BasicString::template convert<std::int16_t>(s) == 0_i16);
        CATCH_CHECK(BasicString::template convert<std::uint16_t>(s) == 0_u16);

        s = FLY_STR(char_type, "100");
        CATCH_CHECK(BasicString::template convert<std::int16_t>(s) == 100_i16);
        CATCH_CHECK(BasicString::template convert<std::uint16_t>(s) == 100_u16);

        s = FLY_STR(char_type, "-100");
        CATCH_CHECK(BasicString::template convert<std::int16_t>(s) == -100_i16);
        CATCH_CHECK_FALSE(BasicString::template convert<std::uint16_t>(s));

        s = FLY_STR(char_type, "abc");
        CATCH_CHECK_FALSE(BasicString::template convert<std::int16_t>(s));
        CATCH_CHECK_FALSE(BasicString::template convert<std::uint16_t>(s));

        s = FLY_STR(char_type, "2a");
        CATCH_CHECK_FALSE(BasicString::template convert<std::int16_t>(s));
        CATCH_CHECK_FALSE(BasicString::template convert<std::uint16_t>(s));

        if constexpr (BasicStringTraits::has_stoi_family_v)
        {
            CATCH_CHECK_FALSE(
                BasicString::template convert<std::int16_t>(minstr<StringType, std::int16_t>()));
            CATCH_CHECK_FALSE(
                BasicString::template convert<std::int16_t>(maxstr<StringType, std::int16_t>()));

            CATCH_CHECK_FALSE(
                BasicString::template convert<std::uint16_t>(minstr<StringType, std::uint16_t>()));
            CATCH_CHECK_FALSE(
                BasicString::template convert<std::uint16_t>(maxstr<StringType, std::uint16_t>()));
        }
    }

    CATCH_SECTION("Convert a string to a 32-bit integer")
    {
        StringType s;

        s = FLY_STR(char_type, "0");
        CATCH_CHECK(BasicString::template convert<std::int32_t>(s) == 0_i32);
        CATCH_CHECK(BasicString::template convert<std::uint32_t>(s) == 0_u32);

        s = FLY_STR(char_type, "100");
        CATCH_CHECK(BasicString::template convert<std::int32_t>(s) == 100_i32);
        CATCH_CHECK(BasicString::template convert<std::uint32_t>(s) == 100_u32);

        s = FLY_STR(char_type, "-100");
        CATCH_CHECK(BasicString::template convert<std::int32_t>(s) == -100_i32);
        CATCH_CHECK_FALSE(BasicString::template convert<std::uint32_t>(s));

        s = FLY_STR(char_type, "abc");
        CATCH_CHECK_FALSE(BasicString::template convert<std::int32_t>(s));
        CATCH_CHECK_FALSE(BasicString::template convert<std::uint32_t>(s));

        s = FLY_STR(char_type, "2a");
        CATCH_CHECK_FALSE(BasicString::template convert<std::int32_t>(s));
        CATCH_CHECK_FALSE(BasicString::template convert<std::uint32_t>(s));

        if constexpr (BasicStringTraits::has_stoi_family_v)
        {
            CATCH_CHECK_FALSE(
                BasicString::template convert<std::int32_t>(minstr<StringType, std::int32_t>()));
            CATCH_CHECK_FALSE(
                BasicString::template convert<std::int32_t>(maxstr<StringType, std::int32_t>()));

            CATCH_CHECK_FALSE(
                BasicString::template convert<std::uint32_t>(minstr<StringType, std::uint32_t>()));
            CATCH_CHECK_FALSE(
                BasicString::template convert<std::uint32_t>(maxstr<StringType, std::uint32_t>()));
        }
    }

    CATCH_SECTION("Convert a string to a 64-bit integer")
    {
        StringType s;

        s = FLY_STR(char_type, "0");
        CATCH_CHECK(BasicString::template convert<std::int64_t>(s) == 0_i64);
        CATCH_CHECK(BasicString::template convert<std::uint64_t>(s) == 0_u64);

        s = FLY_STR(char_type, "100");
        CATCH_CHECK(BasicString::template convert<std::int64_t>(s) == 100_i64);
        CATCH_CHECK(BasicString::template convert<std::uint64_t>(s) == 100_u64);

        s = FLY_STR(char_type, "-100");
        CATCH_CHECK(BasicString::template convert<std::int64_t>(s) == -100_i64);

        s = FLY_STR(char_type, "abc");
        CATCH_CHECK_FALSE(BasicString::template convert<std::int64_t>(s));
        CATCH_CHECK_FALSE(BasicString::template convert<std::uint64_t>(s));

        s = FLY_STR(char_type, "2a");
        CATCH_CHECK_FALSE(BasicString::template convert<std::int64_t>(s));
        CATCH_CHECK_FALSE(BasicString::template convert<std::uint64_t>(s));
    }

    CATCH_SECTION("Convert a string to a floating point decimal")
    {
        StringType s;

        s = FLY_STR(char_type, "-400.123");
        CATCH_CHECK(BasicString::template convert<float>(s) == -400.123f);
        CATCH_CHECK(BasicString::template convert<double>(s) == -400.123);
        CATCH_CHECK(BasicString::template convert<long double>(s) == -400.123L);

        s = FLY_STR(char_type, "400.456");
        CATCH_CHECK(BasicString::template convert<float>(s) == 400.456f);
        CATCH_CHECK(BasicString::template convert<double>(s) == 400.456);
        CATCH_CHECK(BasicString::template convert<long double>(s) == 400.456L);

        s = FLY_STR(char_type, "abc");
        CATCH_CHECK_FALSE(BasicString::template convert<float>(s));
        CATCH_CHECK_FALSE(BasicString::template convert<double>(s));
        CATCH_CHECK_FALSE(BasicString::template convert<long double>(s));

        s = FLY_STR(char_type, "2a");
        CATCH_CHECK_FALSE(BasicString::template convert<float>(s));
        CATCH_CHECK_FALSE(BasicString::template convert<double>(s));
        CATCH_CHECK_FALSE(BasicString::template convert<long double>(s));
    }
}

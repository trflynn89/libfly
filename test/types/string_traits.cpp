#include "fly/types/string/detail/string_traits.hpp"

#include "fly/traits/traits.hpp"
#include "fly/types/string/string_literal.hpp"

#include "catch2/catch.hpp"

#include <string>
#include <type_traits>

namespace {

template <typename StringType>
struct Streamable
{
    using ostream_type = typename fly::detail::BasicStringTraits<StringType>::ostream_type;

    friend ostream_type &operator<<(ostream_type &stream, const Streamable &)
    {
        return stream;
    }
};

struct NotStreamable
{
};

template <typename T, fly::enable_if_any<fly::detail::is_supported_string<T>> = 0>
constexpr bool is_supported_string(const T &)
{
    return true;
}

template <typename T, fly::enable_if_none<fly::detail::is_supported_string<T>> = 0>
constexpr bool is_supported_string(const T &)
{
    return false;
}

template <typename T, fly::enable_if_any<fly::detail::is_supported_character<T>> = 0>
constexpr bool is_supported_character(const T &)
{
    return true;
}

template <typename T, fly::enable_if_none<fly::detail::is_supported_character<T>> = 0>
constexpr bool is_supported_character(const T &)
{
    return false;
}

template <typename T, fly::enable_if_any<fly::detail::is_like_supported_string<T>> = 0>
constexpr bool is_like_supported_string(const T &)
{
    return true;
}

template <typename T, fly::enable_if_none<fly::detail::is_like_supported_string<T>> = 0>
constexpr bool is_like_supported_string(const T &)
{
    return false;
}

template <
    typename T,
    fly::enable_if_any<
        fly::detail::BasicStringTraits<std::string>::is_string_like<T>,
        fly::detail::BasicStringTraits<std::wstring>::is_string_like<T>,
        fly::detail::BasicStringTraits<std::u8string>::is_string_like<T>,
        fly::detail::BasicStringTraits<std::u16string>::is_string_like<T>,
        fly::detail::BasicStringTraits<std::u32string>::is_string_like<T>> = 0>
constexpr bool is_string_like(const T &)
{
    return true;
}

template <
    typename T,
    fly::enable_if_none<
        fly::detail::BasicStringTraits<std::string>::is_string_like<T>,
        fly::detail::BasicStringTraits<std::wstring>::is_string_like<T>,
        fly::detail::BasicStringTraits<std::u8string>::is_string_like<T>,
        fly::detail::BasicStringTraits<std::u16string>::is_string_like<T>,
        fly::detail::BasicStringTraits<std::u32string>::is_string_like<T>> = 0>
constexpr bool is_string_like(const T &)
{
    return false;
}

template <
    typename StringType,
    fly::enable_if_all<typename fly::detail::BasicStringTraits<StringType>::has_stoi_family> = 0>
constexpr int call_stoi(const StringType &str)
{
    return std::stoi(str);
}

template <
    typename StringType,
    fly::enable_if_not_all<typename fly::detail::BasicStringTraits<StringType>::has_stoi_family> =
        0>
constexpr int call_stoi(const StringType &)
{
    return -1;
}

} // namespace

CATCH_TEMPLATE_TEST_CASE(
    "BasicStringTraits",
    "[string]",
    std::string,
    std::wstring,
    std::u8string,
    std::u16string,
    std::u32string)
{
    using StringType = TestType;
    using traits = typename fly::detail::BasicStringTraits<StringType>;
    using char_type = typename traits::char_type;
    using char_pointer_type = typename std::add_pointer<char_type>::type;
    using streamed_type = typename traits::streamed_type;
    using view_type = typename traits::view_type;

    constexpr bool is_string = std::is_same_v<StringType, std::string>;
    constexpr bool is_wstring = std::is_same_v<StringType, std::wstring>;
    constexpr bool is_string8 = std::is_same_v<StringType, std::u8string>;
    constexpr bool is_string16 = std::is_same_v<StringType, std::u16string>;
    constexpr bool is_string32 = std::is_same_v<StringType, std::u32string>;

    CATCH_SECTION("Check whether the STL defines the std::stoi family of functions via traits")
    {
        CATCH_CHECK(traits::has_stoi_family_v == (is_string || is_wstring));
    }

    CATCH_SECTION(
        "Check whether the STL defines the std::stoi family of functions via SFINAE overloads")
    {
        const StringType s = FLY_STR(char_type, "123");
        const int i = call_stoi(s);

        if constexpr (is_string || is_wstring)
        {
            CATCH_CHECK(i == 123);
        }
        else
        {
            CATCH_CHECK(i == -1);
        }
    }

    CATCH_SECTION("Check whether types are supported strings via traits")
    {
        CATCH_SECTION("Plain data types")
        {
            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<int>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<const int>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<int const>);

            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<char_type>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<const char_type>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<char_type const>);

            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<char_type &>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<const char_type &>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<char_type const &>);
        }

        CATCH_SECTION("C-string types")
        {
            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<char_type *>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<const char_type *>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<char_type const *>);
        }

        CATCH_SECTION("C++-string types")
        {
            CATCH_CHECK(fly::detail::is_supported_string_v<StringType>);
            CATCH_CHECK(fly::detail::is_supported_string_v<const StringType>);
            CATCH_CHECK(fly::detail::is_supported_string_v<StringType const>);

            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<view_type>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<const view_type>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<view_type const>);
        }

        CATCH_SECTION("C++-string type references")
        {
            CATCH_CHECK(fly::detail::is_supported_string_v<StringType &>);
            CATCH_CHECK(fly::detail::is_supported_string_v<const StringType &>);
            CATCH_CHECK(fly::detail::is_supported_string_v<StringType const &>);

            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<view_type &>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<const view_type &>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<view_type const &>);
        }

        CATCH_SECTION("C++-string type pointers")
        {
            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<StringType *>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<const StringType *>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<StringType const *>);

            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<view_type *>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<const view_type *>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<view_type const *>);
        }
    }

    CATCH_SECTION("Check whether types are supported characters via traits")
    {
        CATCH_SECTION("Plain data types")
        {
            CATCH_CHECK_FALSE(fly::detail::is_supported_character_v<int>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_character_v<const int>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_character_v<int const>);

            CATCH_CHECK(fly::detail::is_supported_character_v<char_type>);
            CATCH_CHECK(fly::detail::is_supported_character_v<const char_type>);
            CATCH_CHECK(fly::detail::is_supported_character_v<char_type const>);

            CATCH_CHECK(fly::detail::is_supported_character_v<char_type &>);
            CATCH_CHECK(fly::detail::is_supported_character_v<const char_type &>);
            CATCH_CHECK(fly::detail::is_supported_character_v<char_type const &>);
        }

        CATCH_SECTION("C-string types")
        {
            CATCH_CHECK_FALSE(fly::detail::is_supported_character_v<char_type *>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_character_v<const char_type *>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_character_v<char_type const *>);
        }

        CATCH_SECTION("C++-string types")
        {
            CATCH_CHECK_FALSE(fly::detail::is_supported_character_v<StringType>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_character_v<const StringType>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_character_v<StringType const>);

            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<view_type>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<const view_type>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<view_type const>);
        }

        CATCH_SECTION("C++-string type references")
        {
            CATCH_CHECK_FALSE(fly::detail::is_supported_character_v<StringType &>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_character_v<const StringType &>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_character_v<StringType const &>);

            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<view_type &>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<const view_type &>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<view_type const &>);
        }

        CATCH_SECTION("C++-string type pointers")
        {
            CATCH_CHECK_FALSE(fly::detail::is_supported_character_v<StringType *>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_character_v<const StringType *>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_character_v<StringType const *>);

            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<view_type *>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<const view_type *>);
            CATCH_CHECK_FALSE(fly::detail::is_supported_string_v<view_type const *>);
        }
    }

    CATCH_SECTION("Check whether types are like supported strings via traits")
    {
        CATCH_SECTION("Plain data types")
        {
            CATCH_CHECK_FALSE(fly::detail::is_like_supported_string_v<int>);
            CATCH_CHECK_FALSE(fly::detail::is_like_supported_string_v<const int>);
            CATCH_CHECK_FALSE(fly::detail::is_like_supported_string_v<int const>);

            CATCH_CHECK_FALSE(fly::detail::is_like_supported_string_v<char_type>);
            CATCH_CHECK_FALSE(fly::detail::is_like_supported_string_v<const char_type>);
            CATCH_CHECK_FALSE(fly::detail::is_like_supported_string_v<char_type const>);

            CATCH_CHECK_FALSE(fly::detail::is_like_supported_string_v<char_type &>);
            CATCH_CHECK_FALSE(fly::detail::is_like_supported_string_v<const char_type &>);
            CATCH_CHECK_FALSE(fly::detail::is_like_supported_string_v<char_type const &>);
        }

        CATCH_SECTION("C-string types")
        {
            CATCH_CHECK(fly::detail::is_like_supported_string_v<char_type *>);
            CATCH_CHECK(fly::detail::is_like_supported_string_v<const char_type *>);
            CATCH_CHECK(fly::detail::is_like_supported_string_v<char_type const *>);

            CATCH_CHECK(
                std::is_same_v<fly::detail::is_like_supported_string_t<char_type *>, StringType>);
            CATCH_CHECK(std::is_same_v<
                        fly::detail::is_like_supported_string_t<const char_type *>,
                        StringType>);
            CATCH_CHECK(std::is_same_v<
                        fly::detail::is_like_supported_string_t<char_type const *>,
                        StringType>);
        }

        CATCH_SECTION("C++-string types")
        {
            CATCH_CHECK(fly::detail::is_like_supported_string_v<StringType>);
            CATCH_CHECK(fly::detail::is_like_supported_string_v<const StringType>);
            CATCH_CHECK(fly::detail::is_like_supported_string_v<StringType const>);

            CATCH_CHECK(
                std::is_same_v<fly::detail::is_like_supported_string_t<StringType>, StringType>);
            CATCH_CHECK(std::is_same_v<
                        fly::detail::is_like_supported_string_t<const StringType>,
                        StringType>);
            CATCH_CHECK(std::is_same_v<
                        fly::detail::is_like_supported_string_t<StringType const>,
                        StringType>);

            CATCH_CHECK(fly::detail::is_like_supported_string_v<view_type>);
            CATCH_CHECK(fly::detail::is_like_supported_string_v<const view_type>);
            CATCH_CHECK(fly::detail::is_like_supported_string_v<view_type const>);
        }

        CATCH_SECTION("C++-string type references")
        {
            CATCH_CHECK(fly::detail::is_like_supported_string_v<StringType &>);
            CATCH_CHECK(fly::detail::is_like_supported_string_v<const StringType &>);
            CATCH_CHECK(fly::detail::is_like_supported_string_v<StringType const &>);

            CATCH_CHECK(
                std::is_same_v<fly::detail::is_like_supported_string_t<StringType &>, StringType>);
            CATCH_CHECK(std::is_same_v<
                        fly::detail::is_like_supported_string_t<const StringType &>,
                        StringType>);
            CATCH_CHECK(std::is_same_v<
                        fly::detail::is_like_supported_string_t<StringType const &>,
                        StringType>);

            CATCH_CHECK(fly::detail::is_like_supported_string_v<view_type &>);
            CATCH_CHECK(fly::detail::is_like_supported_string_v<const view_type &>);
            CATCH_CHECK(fly::detail::is_like_supported_string_v<view_type const &>);
        }

        CATCH_SECTION("C++-string type pointers")
        {
            CATCH_CHECK_FALSE(fly::detail::is_like_supported_string_v<StringType *>);
            CATCH_CHECK_FALSE(fly::detail::is_like_supported_string_v<const StringType *>);
            CATCH_CHECK_FALSE(fly::detail::is_like_supported_string_v<StringType const *>);

            CATCH_CHECK_FALSE(fly::detail::is_like_supported_string_v<view_type *>);
            CATCH_CHECK_FALSE(fly::detail::is_like_supported_string_v<const view_type *>);
            CATCH_CHECK_FALSE(fly::detail::is_like_supported_string_v<view_type const *>);
        }
    }

    CATCH_SECTION("Check whether types are string-like via traits")
    {
        CATCH_SECTION("Plain data types")
        {
            CATCH_CHECK_FALSE(traits::template is_string_like_v<int>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<const int>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<int const>);

            CATCH_CHECK_FALSE(traits::template is_string_like_v<char>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<const char>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<char const>);

            CATCH_CHECK_FALSE(traits::template is_string_like_v<wchar_t>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<const wchar_t>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<wchar_t const>);

            CATCH_CHECK_FALSE(traits::template is_string_like_v<char16_t>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<const char16_t>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<char16_t const>);

            CATCH_CHECK_FALSE(traits::template is_string_like_v<char32_t>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<const char32_t>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<char32_t const>);

            CATCH_CHECK_FALSE(traits::template is_string_like_v<char &>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<const char &>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<char const &>);

            CATCH_CHECK_FALSE(traits::template is_string_like_v<wchar_t &>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<const wchar_t &>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<wchar_t const &>);

            CATCH_CHECK_FALSE(traits::template is_string_like_v<char16_t &>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<const char16_t &>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<char16_t const &>);

            CATCH_CHECK_FALSE(traits::template is_string_like_v<char32_t &>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<const char32_t &>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<char32_t const &>);
        }

        CATCH_SECTION("C-string types")
        {
            CATCH_CHECK(traits::template is_string_like_v<char *> == is_string);
            CATCH_CHECK(traits::template is_string_like_v<const char *> == is_string);
            CATCH_CHECK(traits::template is_string_like_v<char const *> == is_string);

            CATCH_CHECK(traits::template is_string_like_v<wchar_t *> == is_wstring);
            CATCH_CHECK(traits::template is_string_like_v<const wchar_t *> == is_wstring);
            CATCH_CHECK(traits::template is_string_like_v<wchar_t const *> == is_wstring);

            CATCH_CHECK(traits::template is_string_like_v<char8_t *> == is_string8);
            CATCH_CHECK(traits::template is_string_like_v<const char8_t *> == is_string8);
            CATCH_CHECK(traits::template is_string_like_v<char8_t const *> == is_string8);

            CATCH_CHECK(traits::template is_string_like_v<char16_t *> == is_string16);
            CATCH_CHECK(traits::template is_string_like_v<const char16_t *> == is_string16);
            CATCH_CHECK(traits::template is_string_like_v<char16_t const *> == is_string16);

            CATCH_CHECK(traits::template is_string_like_v<char32_t *> == is_string32);
            CATCH_CHECK(traits::template is_string_like_v<const char32_t *> == is_string32);
            CATCH_CHECK(traits::template is_string_like_v<char32_t const *> == is_string32);
        }

        CATCH_SECTION("C++-string types")
        {
            CATCH_CHECK(traits::template is_string_like_v<std::string> == is_string);
            CATCH_CHECK(traits::template is_string_like_v<const std::string> == is_string);
            CATCH_CHECK(traits::template is_string_like_v<std::string const> == is_string);

            CATCH_CHECK(traits::template is_string_like_v<std::wstring> == is_wstring);
            CATCH_CHECK(traits::template is_string_like_v<const std::wstring> == is_wstring);
            CATCH_CHECK(traits::template is_string_like_v<std::wstring const> == is_wstring);

            CATCH_CHECK(traits::template is_string_like_v<std::u8string> == is_string8);
            CATCH_CHECK(traits::template is_string_like_v<const std::u8string> == is_string8);
            CATCH_CHECK(traits::template is_string_like_v<std::u8string const> == is_string8);

            CATCH_CHECK(traits::template is_string_like_v<std::u16string> == is_string16);
            CATCH_CHECK(traits::template is_string_like_v<const std::u16string> == is_string16);
            CATCH_CHECK(traits::template is_string_like_v<std::u16string const> == is_string16);

            CATCH_CHECK(traits::template is_string_like_v<std::u32string> == is_string32);
            CATCH_CHECK(traits::template is_string_like_v<const std::u32string> == is_string32);
            CATCH_CHECK(traits::template is_string_like_v<std::u32string const> == is_string32);
        }

        CATCH_SECTION("C++-string type references")
        {
            CATCH_CHECK(traits::template is_string_like_v<std::string &> == is_string);
            CATCH_CHECK(traits::template is_string_like_v<const std::string &> == is_string);
            CATCH_CHECK(traits::template is_string_like_v<std::string const &> == is_string);

            CATCH_CHECK(traits::template is_string_like_v<std::wstring &> == is_wstring);
            CATCH_CHECK(traits::template is_string_like_v<const std::wstring &> == is_wstring);
            CATCH_CHECK(traits::template is_string_like_v<std::wstring const &> == is_wstring);

            CATCH_CHECK(traits::template is_string_like_v<std::u8string &> == is_string8);
            CATCH_CHECK(traits::template is_string_like_v<const std::u8string &> == is_string8);
            CATCH_CHECK(traits::template is_string_like_v<std::u8string const &> == is_string8);

            CATCH_CHECK(traits::template is_string_like_v<std::u16string &> == is_string16);
            CATCH_CHECK(traits::template is_string_like_v<const std::u16string &> == is_string16);
            CATCH_CHECK(traits::template is_string_like_v<std::u16string const &> == is_string16);

            CATCH_CHECK(traits::template is_string_like_v<std::u32string &> == is_string32);
            CATCH_CHECK(traits::template is_string_like_v<const std::u32string &> == is_string32);
            CATCH_CHECK(traits::template is_string_like_v<std::u32string const &> == is_string32);
        }

        CATCH_SECTION("C++-string type pointers")
        {
            CATCH_CHECK_FALSE(traits::template is_string_like_v<std::string *>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<const std::string *>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<std::string const *>);

            CATCH_CHECK_FALSE(traits::template is_string_like_v<std::wstring *>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<const std::wstring *>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<std::wstring const *>);

            CATCH_CHECK_FALSE(traits::template is_string_like_v<std::u8string *>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<const std::u8string *>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<std::u8string const *>);

            CATCH_CHECK_FALSE(traits::template is_string_like_v<std::u16string *>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<const std::u16string *>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<std::u16string const *>);

            CATCH_CHECK_FALSE(traits::template is_string_like_v<std::u32string *>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<const std::u32string *>);
            CATCH_CHECK_FALSE(traits::template is_string_like_v<std::u32string const *>);
        }
    }

    CATCH_SECTION("Check whether types are supported strings via SFINAE overloads")
    {
        CATCH_CHECK(is_supported_string(StringType()));

        CATCH_CHECK_FALSE(is_supported_string(int()));
        CATCH_CHECK_FALSE(is_supported_string(char_type()));
        CATCH_CHECK_FALSE(is_supported_string(char_pointer_type()));
    }

    CATCH_SECTION("Check whether types are supported characters via SFINAE overloads")
    {
        CATCH_CHECK(is_supported_character(char_type()));

        CATCH_CHECK_FALSE(is_supported_character(StringType()));
        CATCH_CHECK_FALSE(is_supported_character(int()));
        CATCH_CHECK_FALSE(is_supported_character(char_pointer_type()));
    }

    CATCH_SECTION("Check whether types are like supported strings via SFINAE overloads")
    {
        CATCH_CHECK(is_like_supported_string(StringType()));
        CATCH_CHECK(is_like_supported_string(char_pointer_type()));

        CATCH_CHECK_FALSE(is_like_supported_string(int()));
        CATCH_CHECK_FALSE(is_like_supported_string(char_type()));
    }

    CATCH_SECTION("Check whether types are string-like via SFINAE overloads")
    {
        CATCH_CHECK(is_string_like(StringType()));
        CATCH_CHECK(is_string_like(char_pointer_type()));

        CATCH_CHECK_FALSE(is_string_like(int()));
        CATCH_CHECK_FALSE(is_string_like(char_type()));
    }

    CATCH_SECTION("Check whether types are streamable")
    {
        const Streamable<streamed_type> obj1;
        const NotStreamable obj2;

        CATCH_CHECK(traits::OstreamTraits::template is_declared_v<int>);
        CATCH_CHECK(traits::OstreamTraits::template is_declared_v<bool>);
        CATCH_CHECK(traits::OstreamTraits::template is_declared_v<streamed_type>);
        CATCH_CHECK(traits::OstreamTraits::template is_declared_v<decltype(obj1)>);

        CATCH_CHECK_FALSE(traits::OstreamTraits::template is_declared_v<decltype(obj2)>);
    }
}

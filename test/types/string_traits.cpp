#include "fly/types/string/detail/string_traits.hpp"

#include "fly/traits/traits.hpp"
#include "fly/types/string/string_literal.hpp"

#include <catch2/catch.hpp>

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

template <
    typename T,
    fly::enable_if_any<
        fly::detail::BasicStringTraits<std::string>::is_string_like<T>,
        fly::detail::BasicStringTraits<std::wstring>::is_string_like<T>,
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

TEMPLATE_TEST_CASE(
    "BasicStringTraits",
    "[string]",
    std::string,
    std::wstring,
    std::u16string,
    std::u32string)
{
    using StringType = TestType;
    using traits = typename fly::detail::BasicStringTraits<StringType>;
    using char_type = typename traits::char_type;
    using char_pointer_type = typename std::add_pointer<char_type>::type;
    using streamed_type = typename traits::streamed_type;

    constexpr bool is_string = std::is_same_v<StringType, std::string>;
    constexpr bool is_wstring = std::is_same_v<StringType, std::wstring>;
    constexpr bool is_string16 = std::is_same_v<StringType, std::u16string>;
    constexpr bool is_string32 = std::is_same_v<StringType, std::u32string>;

    SECTION("Check whether the STL defines the std::stoi family of functions via traits")
    {
        CHECK(traits::has_stoi_family_v == (is_string || is_wstring));
    }

    SECTION("Check whether the STL defines the std::stoi family of functions via SFINAE overloads")
    {
        const StringType s = FLY_STR(char_type, "123");
        const int i = call_stoi(s);

        if constexpr (is_string || is_wstring)
        {
            CHECK(i == 123);
        }
        else
        {
            CHECK(i == -1);
        }
    }

    SECTION("Check whether types are string-like via traits")
    {
        SECTION("Plain data types")
        {
            CHECK_FALSE(traits::template is_string_like_v<int>);
            CHECK_FALSE(traits::template is_string_like_v<const int>);
            CHECK_FALSE(traits::template is_string_like_v<int const>);

            CHECK_FALSE(traits::template is_string_like_v<char>);
            CHECK_FALSE(traits::template is_string_like_v<const char>);
            CHECK_FALSE(traits::template is_string_like_v<char const>);

            CHECK_FALSE(traits::template is_string_like_v<wchar_t>);
            CHECK_FALSE(traits::template is_string_like_v<const wchar_t>);
            CHECK_FALSE(traits::template is_string_like_v<wchar_t const>);

            CHECK_FALSE(traits::template is_string_like_v<char16_t>);
            CHECK_FALSE(traits::template is_string_like_v<const char16_t>);
            CHECK_FALSE(traits::template is_string_like_v<char16_t const>);

            CHECK_FALSE(traits::template is_string_like_v<char32_t>);
            CHECK_FALSE(traits::template is_string_like_v<const char32_t>);
            CHECK_FALSE(traits::template is_string_like_v<char32_t const>);

            CHECK_FALSE(traits::template is_string_like_v<char &>);
            CHECK_FALSE(traits::template is_string_like_v<const char &>);
            CHECK_FALSE(traits::template is_string_like_v<char const &>);

            CHECK_FALSE(traits::template is_string_like_v<wchar_t &>);
            CHECK_FALSE(traits::template is_string_like_v<const wchar_t &>);
            CHECK_FALSE(traits::template is_string_like_v<wchar_t const &>);

            CHECK_FALSE(traits::template is_string_like_v<char16_t &>);
            CHECK_FALSE(traits::template is_string_like_v<const char16_t &>);
            CHECK_FALSE(traits::template is_string_like_v<char16_t const &>);

            CHECK_FALSE(traits::template is_string_like_v<char32_t &>);
            CHECK_FALSE(traits::template is_string_like_v<const char32_t &>);
            CHECK_FALSE(traits::template is_string_like_v<char32_t const &>);
        }

        SECTION("C-string types")
        {
            CHECK(traits::template is_string_like_v<char *> == is_string);
            CHECK(traits::template is_string_like_v<const char *> == is_string);
            CHECK(traits::template is_string_like_v<char const *> == is_string);

            CHECK(traits::template is_string_like_v<wchar_t *> == is_wstring);
            CHECK(traits::template is_string_like_v<const wchar_t *> == is_wstring);
            CHECK(traits::template is_string_like_v<wchar_t const *> == is_wstring);

            CHECK(traits::template is_string_like_v<char16_t *> == is_string16);
            CHECK(traits::template is_string_like_v<const char16_t *> == is_string16);
            CHECK(traits::template is_string_like_v<char16_t const *> == is_string16);

            CHECK(traits::template is_string_like_v<char32_t *> == is_string32);
            CHECK(traits::template is_string_like_v<const char32_t *> == is_string32);
            CHECK(traits::template is_string_like_v<char32_t const *> == is_string32);
        }

        SECTION("C++-string types")
        {
            CHECK(traits::template is_string_like_v<std::string> == is_string);
            CHECK(traits::template is_string_like_v<const std::string> == is_string);
            CHECK(traits::template is_string_like_v<std::string const> == is_string);

            CHECK(traits::template is_string_like_v<std::wstring> == is_wstring);
            CHECK(traits::template is_string_like_v<const std::wstring> == is_wstring);
            CHECK(traits::template is_string_like_v<std::wstring const> == is_wstring);

            CHECK(traits::template is_string_like_v<std::u16string> == is_string16);
            CHECK(traits::template is_string_like_v<const std::u16string> == is_string16);
            CHECK(traits::template is_string_like_v<std::u16string const> == is_string16);

            CHECK(traits::template is_string_like_v<std::u32string> == is_string32);
            CHECK(traits::template is_string_like_v<const std::u32string> == is_string32);
            CHECK(traits::template is_string_like_v<std::u32string const> == is_string32);
        }

        SECTION("C++-string type references")
        {
            CHECK(traits::template is_string_like_v<std::string &> == is_string);
            CHECK(traits::template is_string_like_v<const std::string &> == is_string);
            CHECK(traits::template is_string_like_v<std::string const &> == is_string);

            CHECK(traits::template is_string_like_v<std::wstring &> == is_wstring);
            CHECK(traits::template is_string_like_v<const std::wstring &> == is_wstring);
            CHECK(traits::template is_string_like_v<std::wstring const &> == is_wstring);

            CHECK(traits::template is_string_like_v<std::u16string &> == is_string16);
            CHECK(traits::template is_string_like_v<const std::u16string &> == is_string16);
            CHECK(traits::template is_string_like_v<std::u16string const &> == is_string16);

            CHECK(traits::template is_string_like_v<std::u32string &> == is_string32);
            CHECK(traits::template is_string_like_v<const std::u32string &> == is_string32);
            CHECK(traits::template is_string_like_v<std::u32string const &> == is_string32);
        }

        SECTION("C++-string type pointers")
        {
            CHECK_FALSE(traits::template is_string_like_v<std::string *>);
            CHECK_FALSE(traits::template is_string_like_v<const std::string *>);
            CHECK_FALSE(traits::template is_string_like_v<std::string const *>);

            CHECK_FALSE(traits::template is_string_like_v<std::wstring *>);
            CHECK_FALSE(traits::template is_string_like_v<const std::wstring *>);
            CHECK_FALSE(traits::template is_string_like_v<std::wstring const *>);

            CHECK_FALSE(traits::template is_string_like_v<std::u16string *>);
            CHECK_FALSE(traits::template is_string_like_v<const std::u16string *>);
            CHECK_FALSE(traits::template is_string_like_v<std::u16string const *>);

            CHECK_FALSE(traits::template is_string_like_v<std::u32string *>);
            CHECK_FALSE(traits::template is_string_like_v<const std::u32string *>);
            CHECK_FALSE(traits::template is_string_like_v<std::u32string const *>);
        }
    }

    SECTION("Check whether types are string-like via SFINAE overloads")
    {
        CHECK(is_string_like(StringType()));
        CHECK(is_string_like(char_pointer_type()));

        CHECK_FALSE(is_string_like(int()));
        CHECK_FALSE(is_string_like(char_type()));
    }

    SECTION("Check whether types are streamable")
    {
        const Streamable<streamed_type> obj1;
        const NotStreamable obj2;

        CHECK(traits::OstreamTraits::template is_declared_v<int>);
        CHECK(traits::OstreamTraits::template is_declared_v<bool>);
        CHECK(traits::OstreamTraits::template is_declared_v<streamed_type>);
        CHECK(traits::OstreamTraits::template is_declared_v<decltype(obj1)>);

        CHECK_FALSE(traits::OstreamTraits::template is_declared_v<decltype(obj2)>);
    }
}

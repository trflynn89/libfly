#include "fly/types/string/detail/string_traits.hpp"

#include "fly/traits/traits.hpp"
#include "fly/types/string/string_literal.hpp"
#include "test/types/string_test.hpp"

#include <gtest/gtest.h>

#include <string>
#include <type_traits>

namespace {

//==================================================================================================
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

//==================================================================================================
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

//==================================================================================================
template <
    typename StringType,
    fly::enable_if_all<typename fly::detail::BasicStringTraits<StringType>::has_stoi_family> = 0>
constexpr int call_stoi(const StringType &str)
{
    return std::stoi(str);
}

//==================================================================================================
template <
    typename StringType,
    fly::enable_if_not_all<typename fly::detail::BasicStringTraits<StringType>::has_stoi_family> =
        0>
constexpr int call_stoi(const StringType &)
{
    return -1;
}

} // namespace

//==================================================================================================
TYPED_TEST(BasicStringTest, StoiFamily)
{
    DECLARE_ALIASES

    constexpr bool is_string = std::is_same_v<string_type, std::string>;
    constexpr bool is_wstring = std::is_same_v<string_type, std::wstring>;

    EXPECT_EQ(traits::has_stoi_family_v, is_string || is_wstring);
}

//==================================================================================================
TYPED_TEST(BasicStringTest, StoiFamilySFINAE)
{
    DECLARE_ALIASES

    constexpr bool is_string = std::is_same_v<string_type, std::string>;
    constexpr bool is_wstring = std::is_same_v<string_type, std::wstring>;

    const string_type s = FLY_STR(char_type, "123");
    const int i = call_stoi(s);

    if constexpr (is_string || is_wstring)
    {
        EXPECT_EQ(i, 123);
    }
    else
    {
        EXPECT_EQ(i, -1);
    }
}

//==================================================================================================
TYPED_TEST(BasicStringTest, StringLike)
{
    DECLARE_ALIASES

    constexpr bool is_string = std::is_same_v<string_type, std::string>;
    constexpr bool is_wstring = std::is_same_v<string_type, std::wstring>;
    constexpr bool is_string16 = std::is_same_v<string_type, std::u16string>;
    constexpr bool is_string32 = std::is_same_v<string_type, std::u32string>;

    EXPECT_FALSE(traits::template is_string_like_v<int>);
    EXPECT_FALSE(traits::template is_string_like_v<const int>);
    EXPECT_FALSE(traits::template is_string_like_v<int const>);

    EXPECT_FALSE(traits::template is_string_like_v<char>);
    EXPECT_FALSE(traits::template is_string_like_v<const char>);
    EXPECT_FALSE(traits::template is_string_like_v<char const>);

    EXPECT_FALSE(traits::template is_string_like_v<wchar_t>);
    EXPECT_FALSE(traits::template is_string_like_v<const wchar_t>);
    EXPECT_FALSE(traits::template is_string_like_v<wchar_t const>);

    EXPECT_FALSE(traits::template is_string_like_v<char16_t>);
    EXPECT_FALSE(traits::template is_string_like_v<const char16_t>);
    EXPECT_FALSE(traits::template is_string_like_v<char16_t const>);

    EXPECT_FALSE(traits::template is_string_like_v<char32_t>);
    EXPECT_FALSE(traits::template is_string_like_v<const char32_t>);
    EXPECT_FALSE(traits::template is_string_like_v<char32_t const>);

    EXPECT_FALSE(traits::template is_string_like_v<char &>);
    EXPECT_FALSE(traits::template is_string_like_v<const char &>);
    EXPECT_FALSE(traits::template is_string_like_v<char const &>);

    EXPECT_FALSE(traits::template is_string_like_v<wchar_t &>);
    EXPECT_FALSE(traits::template is_string_like_v<const wchar_t &>);
    EXPECT_FALSE(traits::template is_string_like_v<wchar_t const &>);

    EXPECT_FALSE(traits::template is_string_like_v<char16_t &>);
    EXPECT_FALSE(traits::template is_string_like_v<const char16_t &>);
    EXPECT_FALSE(traits::template is_string_like_v<char16_t const &>);

    EXPECT_FALSE(traits::template is_string_like_v<char32_t &>);
    EXPECT_FALSE(traits::template is_string_like_v<const char32_t &>);
    EXPECT_FALSE(traits::template is_string_like_v<char32_t const &>);

    EXPECT_EQ(traits::template is_string_like_v<char *>, is_string);
    EXPECT_EQ(traits::template is_string_like_v<const char *>, is_string);
    EXPECT_EQ(traits::template is_string_like_v<char const *>, is_string);

    EXPECT_EQ(traits::template is_string_like_v<wchar_t *>, is_wstring);
    EXPECT_EQ(traits::template is_string_like_v<const wchar_t *>, is_wstring);
    EXPECT_EQ(traits::template is_string_like_v<wchar_t const *>, is_wstring);

    EXPECT_EQ(traits::template is_string_like_v<char16_t *>, is_string16);
    EXPECT_EQ(traits::template is_string_like_v<const char16_t *>, is_string16);
    EXPECT_EQ(traits::template is_string_like_v<char16_t const *>, is_string16);

    EXPECT_EQ(traits::template is_string_like_v<char32_t *>, is_string32);
    EXPECT_EQ(traits::template is_string_like_v<const char32_t *>, is_string32);
    EXPECT_EQ(traits::template is_string_like_v<char32_t const *>, is_string32);

    EXPECT_EQ(traits::template is_string_like_v<std::string>, is_string);
    EXPECT_EQ(traits::template is_string_like_v<const std::string>, is_string);
    EXPECT_EQ(traits::template is_string_like_v<std::string const>, is_string);

    EXPECT_EQ(traits::template is_string_like_v<std::wstring>, is_wstring);
    EXPECT_EQ(traits::template is_string_like_v<const std::wstring>, is_wstring);
    EXPECT_EQ(traits::template is_string_like_v<std::wstring const>, is_wstring);

    EXPECT_EQ(traits::template is_string_like_v<std::u16string>, is_string16);
    EXPECT_EQ(traits::template is_string_like_v<const std::u16string>, is_string16);
    EXPECT_EQ(traits::template is_string_like_v<std::u16string const>, is_string16);

    EXPECT_EQ(traits::template is_string_like_v<std::u32string>, is_string32);
    EXPECT_EQ(traits::template is_string_like_v<const std::u32string>, is_string32);
    EXPECT_EQ(traits::template is_string_like_v<std::u32string const>, is_string32);

    EXPECT_EQ(traits::template is_string_like_v<std::string &>, is_string);
    EXPECT_EQ(traits::template is_string_like_v<const std::string &>, is_string);
    EXPECT_EQ(traits::template is_string_like_v<std::string const &>, is_string);

    EXPECT_EQ(traits::template is_string_like_v<std::wstring &>, is_wstring);
    EXPECT_EQ(traits::template is_string_like_v<const std::wstring &>, is_wstring);
    EXPECT_EQ(traits::template is_string_like_v<std::wstring const &>, is_wstring);

    EXPECT_EQ(traits::template is_string_like_v<std::u16string &>, is_string16);
    EXPECT_EQ(traits::template is_string_like_v<const std::u16string &>, is_string16);
    EXPECT_EQ(traits::template is_string_like_v<std::u16string const &>, is_string16);

    EXPECT_EQ(traits::template is_string_like_v<std::u32string &>, is_string32);
    EXPECT_EQ(traits::template is_string_like_v<const std::u32string &>, is_string32);
    EXPECT_EQ(traits::template is_string_like_v<std::u32string const &>, is_string32);

    EXPECT_FALSE(traits::template is_string_like_v<std::string *>);
    EXPECT_FALSE(traits::template is_string_like_v<const std::string *>);
    EXPECT_FALSE(traits::template is_string_like_v<std::string const *>);

    EXPECT_FALSE(traits::template is_string_like_v<std::wstring *>);
    EXPECT_FALSE(traits::template is_string_like_v<const std::wstring *>);
    EXPECT_FALSE(traits::template is_string_like_v<std::wstring const *>);

    EXPECT_FALSE(traits::template is_string_like_v<std::u16string *>);
    EXPECT_FALSE(traits::template is_string_like_v<const std::u16string *>);
    EXPECT_FALSE(traits::template is_string_like_v<std::u16string const *>);

    EXPECT_FALSE(traits::template is_string_like_v<std::u32string *>);
    EXPECT_FALSE(traits::template is_string_like_v<const std::u32string *>);
    EXPECT_FALSE(traits::template is_string_like_v<std::u32string const *>);
}

//==================================================================================================
TYPED_TEST(BasicStringTest, StringLikeSFINAE)
{
    DECLARE_ALIASES

    EXPECT_TRUE(is_string_like(string_type()));
    EXPECT_TRUE(is_string_like(char_pointer_type()));

    EXPECT_FALSE(is_string_like(int()));
    EXPECT_FALSE(is_string_like(char_type()));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, OstreamTraits)
{
    DECLARE_ALIASES

    const Streamable<streamed_type> obj1(FLY_STR(streamed_char, "hi"), 0xbeef);
    const NotStreamable obj2;

    EXPECT_TRUE(traits::OstreamTraits::template is_declared_v<int>);
    EXPECT_TRUE(traits::OstreamTraits::template is_declared_v<bool>);
    EXPECT_TRUE(traits::OstreamTraits::template is_declared_v<streamed_type>);
    EXPECT_TRUE(traits::OstreamTraits::template is_declared_v<decltype(obj1)>);
    EXPECT_FALSE(traits::OstreamTraits::template is_declared_v<decltype(obj2)>);
}

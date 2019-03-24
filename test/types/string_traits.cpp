#include "fly/types/string_traits.h"

#include "fly/traits/traits.h"

#include <gtest/gtest.h>

#include <string>

namespace {

//==============================================================================
template <
    typename T,
    fly::enable_if_any<
        fly::BasicStringTraits<std::string>::is_string_like<T>,
        fly::BasicStringTraits<std::wstring>::is_string_like<T>,
        fly::BasicStringTraits<std::u16string>::is_string_like<T>,
        fly::BasicStringTraits<std::u32string>::is_string_like<T>> = 0>
constexpr bool isStringLike(const T &) noexcept
{
    return true;
}

//==============================================================================
template <
    typename T,
    fly::enable_if_none<
        fly::BasicStringTraits<std::string>::is_string_like<T>,
        fly::BasicStringTraits<std::wstring>::is_string_like<T>,
        fly::BasicStringTraits<std::u16string>::is_string_like<T>,
        fly::BasicStringTraits<std::u32string>::is_string_like<T>> = 0>
constexpr bool isStringLike(const T &) noexcept
{
    return false;
}

} // namespace

//==============================================================================
template <typename T>
struct BasicStringTraitsTest : public ::testing::Test
{
    using string_type = T;
};

using StringTypes =
    ::testing::Types<std::string, std::wstring, std::u16string, std::u32string>;

TYPED_TEST_CASE(BasicStringTraitsTest, StringTypes);

//==============================================================================
TYPED_TEST(BasicStringTraitsTest, StoiFamilyTest)
{
    using string_type = typename TestFixture::string_type;
    using traits = typename fly::BasicStringTraits<string_type>;

    constexpr bool is_string = std::is_same_v<string_type, std::string>;
    constexpr bool is_wstring = std::is_same_v<string_type, std::wstring>;

    EXPECT_EQ(traits::has_stoi_family_v, is_string || is_wstring);
}

//==============================================================================
TYPED_TEST(BasicStringTraitsTest, StringLikeTest)
{
    using string_type = typename TestFixture::string_type;
    using traits = typename fly::BasicStringTraits<string_type>;

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
    EXPECT_EQ(
        traits::template is_string_like_v<const std::wstring>, is_wstring);
    EXPECT_EQ(
        traits::template is_string_like_v<std::wstring const>, is_wstring);

    EXPECT_EQ(traits::template is_string_like_v<std::u16string>, is_string16);
    EXPECT_EQ(
        traits::template is_string_like_v<const std::u16string>, is_string16);
    EXPECT_EQ(
        traits::template is_string_like_v<std::u16string const>, is_string16);

    EXPECT_EQ(traits::template is_string_like_v<std::u32string>, is_string32);
    EXPECT_EQ(
        traits::template is_string_like_v<const std::u32string>, is_string32);
    EXPECT_EQ(
        traits::template is_string_like_v<std::u32string const>, is_string32);

    EXPECT_EQ(traits::template is_string_like_v<std::string &>, is_string);
    EXPECT_EQ(
        traits::template is_string_like_v<const std::string &>, is_string);
    EXPECT_EQ(
        traits::template is_string_like_v<std::string const &>, is_string);

    EXPECT_EQ(traits::template is_string_like_v<std::wstring &>, is_wstring);
    EXPECT_EQ(
        traits::template is_string_like_v<const std::wstring &>, is_wstring);
    EXPECT_EQ(
        traits::template is_string_like_v<std::wstring const &>, is_wstring);

    EXPECT_EQ(traits::template is_string_like_v<std::u16string &>, is_string16);
    EXPECT_EQ(
        traits::template is_string_like_v<const std::u16string &>, is_string16);
    EXPECT_EQ(
        traits::template is_string_like_v<std::u16string const &>, is_string16);

    EXPECT_EQ(traits::template is_string_like_v<std::u32string &>, is_string32);
    EXPECT_EQ(
        traits::template is_string_like_v<const std::u32string &>, is_string32);
    EXPECT_EQ(
        traits::template is_string_like_v<std::u32string const &>, is_string32);

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

//==============================================================================
TYPED_TEST(BasicStringTraitsTest, StringLikeSFINAETest)
{
    using string_type = typename TestFixture::string_type;
    using char_type = typename string_type::value_type;
    using char_pointer_type = typename std::add_pointer<char_type>::type;

    EXPECT_TRUE(isStringLike(string_type()));
    EXPECT_TRUE(isStringLike(char_pointer_type()));

    EXPECT_FALSE(isStringLike(int()));
    EXPECT_FALSE(isStringLike(char_type()));
}

#include "fly/types/string/detail/string_traits.h"

#include "fly/traits/traits.h"
#include "fly/types/string/string_literal.h"

#include <gtest/gtest.h>

#include <string>

namespace {

//==============================================================================
template <typename StringType>
class Streamable
{
public:
    using ostream_type = typename fly::detail::BasicStringTraits<
        StringType>::streamer_type::ostream_type;

    friend ostream_type &operator<<(ostream_type &, const Streamable &)
    {
    }
};

//==============================================================================
class NotStreamable
{
};

//==============================================================================
template <
    typename T,
    fly::enable_if_any<
        fly::detail::BasicStringTraits<std::string>::is_string_like<T>,
        fly::detail::BasicStringTraits<std::wstring>::is_string_like<T>,
        fly::detail::BasicStringTraits<std::u16string>::is_string_like<T>,
        fly::detail::BasicStringTraits<std::u32string>::is_string_like<T>> = 0>
constexpr bool isStringLike(const T &) noexcept
{
    return true;
}

//==============================================================================
template <
    typename T,
    fly::enable_if_none<
        fly::detail::BasicStringTraits<std::string>::is_string_like<T>,
        fly::detail::BasicStringTraits<std::wstring>::is_string_like<T>,
        fly::detail::BasicStringTraits<std::u16string>::is_string_like<T>,
        fly::detail::BasicStringTraits<std::u32string>::is_string_like<T>> = 0>
constexpr bool isStringLike(const T &) noexcept
{
    return false;
}

//==============================================================================
template <
    typename StringType,
    fly::enable_if_all<typename fly::detail::BasicStringTraits<
        StringType>::has_stoi_family> = 0>
constexpr int callStoi(const StringType &str) noexcept
{
    return std::stoi(str);
}

//==============================================================================
template <
    typename StringType,
    fly::enable_if_not_all<typename fly::detail::BasicStringTraits<
        StringType>::has_stoi_family> = 0>
constexpr int callStoi(const StringType &) noexcept
{
    return -1;
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
    using traits = typename fly::detail::BasicStringTraits<string_type>;

    constexpr bool is_string = std::is_same_v<string_type, std::string>;
    constexpr bool is_wstring = std::is_same_v<string_type, std::wstring>;

    EXPECT_EQ(traits::has_stoi_family_v, is_string || is_wstring);
}

//==============================================================================
TYPED_TEST(BasicStringTraitsTest, StoiFamilySFINAETest)
{
    using string_type = typename TestFixture::string_type;
    using char_type = typename string_type::value_type;

    constexpr bool is_string = std::is_same_v<string_type, std::string>;
    constexpr bool is_wstring = std::is_same_v<string_type, std::wstring>;

    const string_type s = FLY_STR(char_type, "123");
    const int i = callStoi(s);

    if constexpr (is_string || is_wstring)
    {
        EXPECT_EQ(i, 123);
    }
    else
    {
        EXPECT_EQ(i, -1);
    }
}

//==============================================================================
TYPED_TEST(BasicStringTraitsTest, StringLikeTest)
{
    using string_type = typename TestFixture::string_type;
    using traits = typename fly::detail::BasicStringTraits<string_type>;

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
        traits::template is_string_like_v<const std::wstring>,
        is_wstring);
    EXPECT_EQ(
        traits::template is_string_like_v<std::wstring const>,
        is_wstring);

    EXPECT_EQ(traits::template is_string_like_v<std::u16string>, is_string16);
    EXPECT_EQ(
        traits::template is_string_like_v<const std::u16string>,
        is_string16);
    EXPECT_EQ(
        traits::template is_string_like_v<std::u16string const>,
        is_string16);

    EXPECT_EQ(traits::template is_string_like_v<std::u32string>, is_string32);
    EXPECT_EQ(
        traits::template is_string_like_v<const std::u32string>,
        is_string32);
    EXPECT_EQ(
        traits::template is_string_like_v<std::u32string const>,
        is_string32);

    EXPECT_EQ(traits::template is_string_like_v<std::string &>, is_string);
    EXPECT_EQ(
        traits::template is_string_like_v<const std::string &>,
        is_string);
    EXPECT_EQ(
        traits::template is_string_like_v<std::string const &>,
        is_string);

    EXPECT_EQ(traits::template is_string_like_v<std::wstring &>, is_wstring);
    EXPECT_EQ(
        traits::template is_string_like_v<const std::wstring &>,
        is_wstring);
    EXPECT_EQ(
        traits::template is_string_like_v<std::wstring const &>,
        is_wstring);

    EXPECT_EQ(traits::template is_string_like_v<std::u16string &>, is_string16);
    EXPECT_EQ(
        traits::template is_string_like_v<const std::u16string &>,
        is_string16);
    EXPECT_EQ(
        traits::template is_string_like_v<std::u16string const &>,
        is_string16);

    EXPECT_EQ(traits::template is_string_like_v<std::u32string &>, is_string32);
    EXPECT_EQ(
        traits::template is_string_like_v<const std::u32string &>,
        is_string32);
    EXPECT_EQ(
        traits::template is_string_like_v<std::u32string const &>,
        is_string32);

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

//==============================================================================
TYPED_TEST(BasicStringTraitsTest, OstreamTraitsTest)
{
    using string_type = typename TestFixture::string_type;
    using traits = typename fly::detail::BasicStringTraits<string_type>;
    using streamed_type = typename traits::streamer_type::streamed_type;

    const Streamable<streamed_type> obj1;
    const NotStreamable obj2;

    EXPECT_TRUE(traits::OstreamTraits::template is_declared_v<int>);
    EXPECT_TRUE(traits::OstreamTraits::template is_declared_v<bool>);
    EXPECT_TRUE(traits::OstreamTraits::template is_declared_v<streamed_type>);
    EXPECT_TRUE(traits::OstreamTraits::template is_declared_v<decltype(obj1)>);
    EXPECT_FALSE(traits::OstreamTraits::template is_declared_v<decltype(obj2)>);
}

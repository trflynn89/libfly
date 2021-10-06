#include "fly/types/string/detail/string_concepts.hpp"

#include "fly/concepts/concepts.hpp"
#include "fly/types/string/detail/string_traits.hpp"

#include "catch2/catch_template_test_macros.hpp"
#include "catch2/catch_test_macros.hpp"

namespace {

template <fly::detail::StandardString T>
constexpr bool is_supported_string(const T &)
{
    return true;
}

template <typename T>
constexpr bool is_supported_string(const T &) requires(!fly::detail::StandardString<T>)
{
    return false;
}

template <fly::detail::StandardCharacter T>
constexpr bool is_supported_character(const T &)
{
    return true;
}

template <typename T>
constexpr bool is_supported_character(const T &) requires(!fly::detail::StandardCharacter<T>)
{
    return false;
}

template <fly::detail::StandardStringLike T>
constexpr bool is_like_supported_string(const T &)
{
    return true;
}

template <typename T>
constexpr bool is_like_supported_string(const T &) requires(!fly::detail::StandardStringLike<T>)
{
    return false;
}

} // namespace

CATCH_TEMPLATE_TEST_CASE(
    "BasicStringConcepts",
    "[string]",
    char,
    wchar_t,
    char8_t,
    char16_t,
    char32_t)
{
    using traits = typename fly::detail::BasicStringTraits<TestType>;

    using string_type = typename traits::string_type;
    using char_type = typename traits::char_type;
    using char_pointer_type = typename std::add_pointer<char_type>::type;
    using view_type = typename traits::view_type;

    CATCH_SECTION("Check whether types are supported strings via traits")
    {
        CATCH_SECTION("Plain data types")
        {
            CATCH_CHECK_FALSE(fly::detail::StandardString<int>);
            CATCH_CHECK_FALSE(fly::detail::StandardString<const int>);
            CATCH_CHECK_FALSE(fly::detail::StandardString<int const>);

            CATCH_CHECK_FALSE(fly::detail::StandardString<char_type>);
            CATCH_CHECK_FALSE(fly::detail::StandardString<const char_type>);
            CATCH_CHECK_FALSE(fly::detail::StandardString<char_type const>);

            CATCH_CHECK_FALSE(fly::detail::StandardString<char_type &>);
            CATCH_CHECK_FALSE(fly::detail::StandardString<const char_type &>);
            CATCH_CHECK_FALSE(fly::detail::StandardString<char_type const &>);
        }

        CATCH_SECTION("C-string types")
        {
            CATCH_CHECK_FALSE(fly::detail::StandardString<char_type *>);
            CATCH_CHECK_FALSE(fly::detail::StandardString<const char_type *>);
            CATCH_CHECK_FALSE(fly::detail::StandardString<char_type const *>);
        }

        CATCH_SECTION("C++-string types")
        {
            CATCH_CHECK(fly::detail::StandardString<string_type>);
            CATCH_CHECK(fly::detail::StandardString<const string_type>);
            CATCH_CHECK(fly::detail::StandardString<string_type const>);

            CATCH_CHECK_FALSE(fly::detail::StandardString<view_type>);
            CATCH_CHECK_FALSE(fly::detail::StandardString<const view_type>);
            CATCH_CHECK_FALSE(fly::detail::StandardString<view_type const>);
        }

        CATCH_SECTION("C++-string type references")
        {
            CATCH_CHECK(fly::detail::StandardString<string_type &>);
            CATCH_CHECK(fly::detail::StandardString<const string_type &>);
            CATCH_CHECK(fly::detail::StandardString<string_type const &>);

            CATCH_CHECK_FALSE(fly::detail::StandardString<view_type &>);
            CATCH_CHECK_FALSE(fly::detail::StandardString<const view_type &>);
            CATCH_CHECK_FALSE(fly::detail::StandardString<view_type const &>);
        }

        CATCH_SECTION("C++-string type pointers")
        {
            CATCH_CHECK_FALSE(fly::detail::StandardString<string_type *>);
            CATCH_CHECK_FALSE(fly::detail::StandardString<const string_type *>);
            CATCH_CHECK_FALSE(fly::detail::StandardString<string_type const *>);

            CATCH_CHECK_FALSE(fly::detail::StandardString<view_type *>);
            CATCH_CHECK_FALSE(fly::detail::StandardString<const view_type *>);
            CATCH_CHECK_FALSE(fly::detail::StandardString<view_type const *>);
        }
    }

    CATCH_SECTION("Check whether types are supported characters via traits")
    {
        CATCH_SECTION("Plain data types")
        {
            CATCH_CHECK_FALSE(fly::detail::StandardCharacter<int>);
            CATCH_CHECK_FALSE(fly::detail::StandardCharacter<const int>);
            CATCH_CHECK_FALSE(fly::detail::StandardCharacter<int const>);

            CATCH_CHECK(fly::detail::StandardCharacter<char_type>);
            CATCH_CHECK(fly::detail::StandardCharacter<const char_type>);
            CATCH_CHECK(fly::detail::StandardCharacter<char_type const>);

            CATCH_CHECK(fly::detail::StandardCharacter<char_type &>);
            CATCH_CHECK(fly::detail::StandardCharacter<const char_type &>);
            CATCH_CHECK(fly::detail::StandardCharacter<char_type const &>);
        }

        CATCH_SECTION("C-string types")
        {
            CATCH_CHECK_FALSE(fly::detail::StandardCharacter<char_type *>);
            CATCH_CHECK_FALSE(fly::detail::StandardCharacter<const char_type *>);
            CATCH_CHECK_FALSE(fly::detail::StandardCharacter<char_type const *>);
        }

        CATCH_SECTION("C++-string types")
        {
            CATCH_CHECK_FALSE(fly::detail::StandardCharacter<string_type>);
            CATCH_CHECK_FALSE(fly::detail::StandardCharacter<const string_type>);
            CATCH_CHECK_FALSE(fly::detail::StandardCharacter<string_type const>);

            CATCH_CHECK_FALSE(fly::detail::StandardString<view_type>);
            CATCH_CHECK_FALSE(fly::detail::StandardString<const view_type>);
            CATCH_CHECK_FALSE(fly::detail::StandardString<view_type const>);
        }

        CATCH_SECTION("C++-string type references")
        {
            CATCH_CHECK_FALSE(fly::detail::StandardCharacter<string_type &>);
            CATCH_CHECK_FALSE(fly::detail::StandardCharacter<const string_type &>);
            CATCH_CHECK_FALSE(fly::detail::StandardCharacter<string_type const &>);

            CATCH_CHECK_FALSE(fly::detail::StandardString<view_type &>);
            CATCH_CHECK_FALSE(fly::detail::StandardString<const view_type &>);
            CATCH_CHECK_FALSE(fly::detail::StandardString<view_type const &>);
        }

        CATCH_SECTION("C++-string type pointers")
        {
            CATCH_CHECK_FALSE(fly::detail::StandardCharacter<string_type *>);
            CATCH_CHECK_FALSE(fly::detail::StandardCharacter<const string_type *>);
            CATCH_CHECK_FALSE(fly::detail::StandardCharacter<string_type const *>);

            CATCH_CHECK_FALSE(fly::detail::StandardString<view_type *>);
            CATCH_CHECK_FALSE(fly::detail::StandardString<const view_type *>);
            CATCH_CHECK_FALSE(fly::detail::StandardString<view_type const *>);
        }
    }

    CATCH_SECTION("Check whether types are like supported strings via traits")
    {
        CATCH_SECTION("Plain data types")
        {
            CATCH_CHECK_FALSE(fly::detail::StandardStringLike<int>);
            CATCH_CHECK_FALSE(fly::detail::StandardStringLike<const int>);
            CATCH_CHECK_FALSE(fly::detail::StandardStringLike<int const>);

            CATCH_CHECK_FALSE(fly::detail::StandardStringLike<char_type>);
            CATCH_CHECK_FALSE(fly::detail::StandardStringLike<const char_type>);
            CATCH_CHECK_FALSE(fly::detail::StandardStringLike<char_type const>);

            CATCH_CHECK_FALSE(fly::detail::StandardStringLike<char_type &>);
            CATCH_CHECK_FALSE(fly::detail::StandardStringLike<const char_type &>);
            CATCH_CHECK_FALSE(fly::detail::StandardStringLike<char_type const &>);
        }

        CATCH_SECTION("C-string types")
        {
            CATCH_CHECK(fly::detail::StandardStringLike<char_type *>);
            CATCH_CHECK(fly::detail::StandardStringLike<const char_type *>);
            CATCH_CHECK(fly::detail::StandardStringLike<char_type const *>);

            CATCH_CHECK(fly::SameAs<fly::detail::StandardStringType<char_type *>, string_type>);
            CATCH_CHECK(
                fly::SameAs<fly::detail::StandardStringType<const char_type *>, string_type>);
            CATCH_CHECK(
                fly::SameAs<fly::detail::StandardStringType<char_type const *>, string_type>);
        }

        CATCH_SECTION("C++-string types")
        {
            CATCH_CHECK(fly::detail::StandardStringLike<string_type>);
            CATCH_CHECK(fly::detail::StandardStringLike<const string_type>);
            CATCH_CHECK(fly::detail::StandardStringLike<string_type const>);

            CATCH_CHECK(fly::SameAs<fly::detail::StandardStringType<string_type>, string_type>);
            CATCH_CHECK(
                fly::SameAs<fly::detail::StandardStringType<const string_type>, string_type>);
            CATCH_CHECK(
                fly::SameAs<fly::detail::StandardStringType<string_type const>, string_type>);

            CATCH_CHECK(fly::detail::StandardStringLike<view_type>);
            CATCH_CHECK(fly::detail::StandardStringLike<const view_type>);
            CATCH_CHECK(fly::detail::StandardStringLike<view_type const>);
        }

        CATCH_SECTION("C++-string type references")
        {
            CATCH_CHECK(fly::detail::StandardStringLike<string_type &>);
            CATCH_CHECK(fly::detail::StandardStringLike<const string_type &>);
            CATCH_CHECK(fly::detail::StandardStringLike<string_type const &>);

            CATCH_CHECK(fly::SameAs<fly::detail::StandardStringType<string_type &>, string_type>);
            CATCH_CHECK(
                fly::SameAs<fly::detail::StandardStringType<const string_type &>, string_type>);
            CATCH_CHECK(
                fly::SameAs<fly::detail::StandardStringType<string_type const &>, string_type>);

            CATCH_CHECK(fly::detail::StandardStringLike<view_type &>);
            CATCH_CHECK(fly::detail::StandardStringLike<const view_type &>);
            CATCH_CHECK(fly::detail::StandardStringLike<view_type const &>);
        }

        CATCH_SECTION("C++-string type pointers")
        {
            CATCH_CHECK_FALSE(fly::detail::StandardStringLike<string_type *>);
            CATCH_CHECK_FALSE(fly::detail::StandardStringLike<const string_type *>);
            CATCH_CHECK_FALSE(fly::detail::StandardStringLike<string_type const *>);

            CATCH_CHECK_FALSE(fly::detail::StandardStringLike<view_type *>);
            CATCH_CHECK_FALSE(fly::detail::StandardStringLike<const view_type *>);
            CATCH_CHECK_FALSE(fly::detail::StandardStringLike<view_type const *>);
        }
    }

    CATCH_SECTION("Check whether types are supported strings via constrained overloads")
    {
        CATCH_CHECK(is_supported_string(string_type()));

        CATCH_CHECK_FALSE(is_supported_string(int()));
        CATCH_CHECK_FALSE(is_supported_string(char_type()));
        CATCH_CHECK_FALSE(is_supported_string(char_pointer_type()));
    }

    CATCH_SECTION("Check whether types are supported characters via constrained overloads")
    {
        CATCH_CHECK(is_supported_character(char_type()));

        CATCH_CHECK_FALSE(is_supported_character(string_type()));
        CATCH_CHECK_FALSE(is_supported_character(int()));
        CATCH_CHECK_FALSE(is_supported_character(char_pointer_type()));
    }

    CATCH_SECTION("Check whether types are like supported strings via constrained overloads")
    {
        CATCH_CHECK(is_like_supported_string(string_type()));
        CATCH_CHECK(is_like_supported_string(char_pointer_type()));

        CATCH_CHECK_FALSE(is_like_supported_string(int()));
        CATCH_CHECK_FALSE(is_like_supported_string(char_type()));
    }
}

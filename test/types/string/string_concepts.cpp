#include "fly/types/string/string_concepts.hpp"

#include "fly/concepts/concepts.hpp"
#include "fly/types/string/detail/string_traits.hpp"

#include "catch2/catch_template_test_macros.hpp"
#include "catch2/catch_test_macros.hpp"

namespace {

template <fly::StandardString T>
constexpr bool is_supported_string(const T &)
{
    return true;
}

template <typename T>
constexpr bool is_supported_string(const T &) requires(!fly::StandardString<T>)
{
    return false;
}

template <fly::StandardCharacter T>
constexpr bool is_supported_character(const T &)
{
    return true;
}

template <typename T>
constexpr bool is_supported_character(const T &) requires(!fly::StandardCharacter<T>)
{
    return false;
}

template <fly::StandardStringLike T>
constexpr bool is_like_supported_string(const T &)
{
    return true;
}

template <typename T>
constexpr bool is_like_supported_string(const T &) requires(!fly::StandardStringLike<T>)
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
            CATCH_CHECK_FALSE(fly::StandardString<int>);
            CATCH_CHECK_FALSE(fly::StandardString<const int>);
            CATCH_CHECK_FALSE(fly::StandardString<int const>);

            CATCH_CHECK_FALSE(fly::StandardString<char_type>);
            CATCH_CHECK_FALSE(fly::StandardString<const char_type>);
            CATCH_CHECK_FALSE(fly::StandardString<char_type const>);

            CATCH_CHECK_FALSE(fly::StandardString<char_type &>);
            CATCH_CHECK_FALSE(fly::StandardString<const char_type &>);
            CATCH_CHECK_FALSE(fly::StandardString<char_type const &>);
        }

        CATCH_SECTION("C-string types")
        {
            CATCH_CHECK_FALSE(fly::StandardString<char_type *>);
            CATCH_CHECK_FALSE(fly::StandardString<const char_type *>);
            CATCH_CHECK_FALSE(fly::StandardString<char_type const *>);
        }

        CATCH_SECTION("C++-string types")
        {
            CATCH_CHECK(fly::StandardString<string_type>);
            CATCH_CHECK(fly::StandardString<const string_type>);
            CATCH_CHECK(fly::StandardString<string_type const>);

            CATCH_CHECK_FALSE(fly::StandardString<view_type>);
            CATCH_CHECK_FALSE(fly::StandardString<const view_type>);
            CATCH_CHECK_FALSE(fly::StandardString<view_type const>);
        }

        CATCH_SECTION("C++-string type references")
        {
            CATCH_CHECK(fly::StandardString<string_type &>);
            CATCH_CHECK(fly::StandardString<const string_type &>);
            CATCH_CHECK(fly::StandardString<string_type const &>);

            CATCH_CHECK_FALSE(fly::StandardString<view_type &>);
            CATCH_CHECK_FALSE(fly::StandardString<const view_type &>);
            CATCH_CHECK_FALSE(fly::StandardString<view_type const &>);
        }

        CATCH_SECTION("C++-string type pointers")
        {
            CATCH_CHECK_FALSE(fly::StandardString<string_type *>);
            CATCH_CHECK_FALSE(fly::StandardString<const string_type *>);
            CATCH_CHECK_FALSE(fly::StandardString<string_type const *>);

            CATCH_CHECK_FALSE(fly::StandardString<view_type *>);
            CATCH_CHECK_FALSE(fly::StandardString<const view_type *>);
            CATCH_CHECK_FALSE(fly::StandardString<view_type const *>);
        }
    }

    CATCH_SECTION("Check whether types are supported characters via traits")
    {
        CATCH_SECTION("Plain data types")
        {
            CATCH_CHECK_FALSE(fly::StandardCharacter<int>);
            CATCH_CHECK_FALSE(fly::StandardCharacter<const int>);
            CATCH_CHECK_FALSE(fly::StandardCharacter<int const>);

            CATCH_CHECK(fly::StandardCharacter<char_type>);
            CATCH_CHECK(fly::StandardCharacter<const char_type>);
            CATCH_CHECK(fly::StandardCharacter<char_type const>);

            CATCH_CHECK(fly::StandardCharacter<char_type &>);
            CATCH_CHECK(fly::StandardCharacter<const char_type &>);
            CATCH_CHECK(fly::StandardCharacter<char_type const &>);
        }

        CATCH_SECTION("C-string types")
        {
            CATCH_CHECK_FALSE(fly::StandardCharacter<char_type *>);
            CATCH_CHECK_FALSE(fly::StandardCharacter<const char_type *>);
            CATCH_CHECK_FALSE(fly::StandardCharacter<char_type const *>);
        }

        CATCH_SECTION("C++-string types")
        {
            CATCH_CHECK_FALSE(fly::StandardCharacter<string_type>);
            CATCH_CHECK_FALSE(fly::StandardCharacter<const string_type>);
            CATCH_CHECK_FALSE(fly::StandardCharacter<string_type const>);

            CATCH_CHECK_FALSE(fly::StandardString<view_type>);
            CATCH_CHECK_FALSE(fly::StandardString<const view_type>);
            CATCH_CHECK_FALSE(fly::StandardString<view_type const>);
        }

        CATCH_SECTION("C++-string type references")
        {
            CATCH_CHECK_FALSE(fly::StandardCharacter<string_type &>);
            CATCH_CHECK_FALSE(fly::StandardCharacter<const string_type &>);
            CATCH_CHECK_FALSE(fly::StandardCharacter<string_type const &>);

            CATCH_CHECK_FALSE(fly::StandardString<view_type &>);
            CATCH_CHECK_FALSE(fly::StandardString<const view_type &>);
            CATCH_CHECK_FALSE(fly::StandardString<view_type const &>);
        }

        CATCH_SECTION("C++-string type pointers")
        {
            CATCH_CHECK_FALSE(fly::StandardCharacter<string_type *>);
            CATCH_CHECK_FALSE(fly::StandardCharacter<const string_type *>);
            CATCH_CHECK_FALSE(fly::StandardCharacter<string_type const *>);

            CATCH_CHECK_FALSE(fly::StandardString<view_type *>);
            CATCH_CHECK_FALSE(fly::StandardString<const view_type *>);
            CATCH_CHECK_FALSE(fly::StandardString<view_type const *>);
        }
    }

    CATCH_SECTION("Check whether types are like supported strings via traits")
    {
        CATCH_SECTION("Plain data types")
        {
            CATCH_CHECK_FALSE(fly::StandardStringLike<int>);
            CATCH_CHECK_FALSE(fly::StandardStringLike<const int>);
            CATCH_CHECK_FALSE(fly::StandardStringLike<int const>);

            CATCH_CHECK_FALSE(fly::StandardStringLike<char_type>);
            CATCH_CHECK_FALSE(fly::StandardStringLike<const char_type>);
            CATCH_CHECK_FALSE(fly::StandardStringLike<char_type const>);

            CATCH_CHECK_FALSE(fly::StandardStringLike<char_type &>);
            CATCH_CHECK_FALSE(fly::StandardStringLike<const char_type &>);
            CATCH_CHECK_FALSE(fly::StandardStringLike<char_type const &>);
        }

        CATCH_SECTION("C-string types")
        {
            CATCH_CHECK(fly::StandardStringLike<char_type *>);
            CATCH_CHECK(fly::StandardStringLike<const char_type *>);
            CATCH_CHECK(fly::StandardStringLike<char_type const *>);

            CATCH_CHECK(fly::SameAs<fly::StandardStringType<char_type *>, string_type>);
            CATCH_CHECK(fly::SameAs<fly::StandardStringType<const char_type *>, string_type>);
            CATCH_CHECK(fly::SameAs<fly::StandardStringType<char_type const *>, string_type>);
        }

        CATCH_SECTION("C++-string types")
        {
            CATCH_CHECK(fly::StandardStringLike<string_type>);
            CATCH_CHECK(fly::StandardStringLike<const string_type>);
            CATCH_CHECK(fly::StandardStringLike<string_type const>);

            CATCH_CHECK(fly::SameAs<fly::StandardStringType<string_type>, string_type>);
            CATCH_CHECK(fly::SameAs<fly::StandardStringType<const string_type>, string_type>);
            CATCH_CHECK(fly::SameAs<fly::StandardStringType<string_type const>, string_type>);

            CATCH_CHECK(fly::StandardStringLike<view_type>);
            CATCH_CHECK(fly::StandardStringLike<const view_type>);
            CATCH_CHECK(fly::StandardStringLike<view_type const>);
        }

        CATCH_SECTION("C++-string type references")
        {
            CATCH_CHECK(fly::StandardStringLike<string_type &>);
            CATCH_CHECK(fly::StandardStringLike<const string_type &>);
            CATCH_CHECK(fly::StandardStringLike<string_type const &>);

            CATCH_CHECK(fly::SameAs<fly::StandardStringType<string_type &>, string_type>);
            CATCH_CHECK(fly::SameAs<fly::StandardStringType<const string_type &>, string_type>);
            CATCH_CHECK(fly::SameAs<fly::StandardStringType<string_type const &>, string_type>);

            CATCH_CHECK(fly::StandardStringLike<view_type &>);
            CATCH_CHECK(fly::StandardStringLike<const view_type &>);
            CATCH_CHECK(fly::StandardStringLike<view_type const &>);
        }

        CATCH_SECTION("C++-string type pointers")
        {
            CATCH_CHECK_FALSE(fly::StandardStringLike<string_type *>);
            CATCH_CHECK_FALSE(fly::StandardStringLike<const string_type *>);
            CATCH_CHECK_FALSE(fly::StandardStringLike<string_type const *>);

            CATCH_CHECK_FALSE(fly::StandardStringLike<view_type *>);
            CATCH_CHECK_FALSE(fly::StandardStringLike<const view_type *>);
            CATCH_CHECK_FALSE(fly::StandardStringLike<view_type const *>);
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

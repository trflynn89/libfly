#pragma once

#include "fly/types/string/string.hpp"

#include <gtest/gtest.h>

#include <string>

/**
 * Test class to be used for testing all string types supported by fly::BasicString<>.
 */
template <typename StringType>
struct BasicStringTest : public ::testing::Test
{
    using string_type = StringType;
};

using StringTypes = ::testing::Types<std::string, std::wstring, std::u16string, std::u32string>;
TYPED_TEST_SUITE(BasicStringTest, StringTypes, );

// Helper macro to forward declare aliases used in tests.
#define DECLARE_ALIASES                                                                            \
    using string_type [[maybe_unused]] = typename TestFixture::string_type;                        \
    using StringClass [[maybe_unused]] = fly::BasicString<string_type>;                            \
    using traits [[maybe_unused]] = typename fly::detail::BasicStringTraits<string_type>;          \
    using char_type [[maybe_unused]] = typename StringClass::char_type;                            \
    using char_pointer_type [[maybe_unused]] = typename std::add_pointer<char_type>::type;         \
    using size_type [[maybe_unused]] = typename StringClass::size_type;                            \
    using streamed_type [[maybe_unused]] = typename StringClass::streamed_type;                    \
    using streamed_char [[maybe_unused]] = typename streamed_type::value_type;                     \
    using ustreamed_char [[maybe_unused]] = std::make_unsigned_t<streamed_char>;

// Helper classes for testing objects that do and do not define operator<<.
template <typename StringType>
class Streamable
{
public:
    using ostream_type = typename fly::BasicString<StringType>::ostream_type;

    Streamable(const StringType &str, int num) noexcept : m_str(str), m_num(num)
    {
    }

    StringType str() const noexcept
    {
        return m_str;
    };

    int num() const noexcept
    {
        return m_num;
    };

    friend ostream_type &operator<<(ostream_type &stream, const Streamable &obj)
    {
        stream << '[';
        stream << obj.str() << ' ' << std::hex << obj.num() << std::dec;
        stream << ']';

        return stream;
    }

private:
    StringType m_str;
    int m_num;
};

class NotStreamable
{
};

#include "fly/types/string/string.hpp"

#include "fly/types/numeric/literals.hpp"

#include "catch2/catch_template_test_macros.hpp"
#include "catch2/catch_test_macros.hpp"

#include <string>
#include <vector>

namespace {

template <typename StringType>
class Streamable
{
public:
    Streamable(const StringType &str, int num) noexcept : m_str(str), m_num(num)
    {
    }

    StringType str() const
    {
        return m_str;
    };

    int num() const
    {
        return m_num;
    };

    friend std::ostream &operator<<(std::ostream &stream, const Streamable &obj)
    {
        stream << fly::String::format("[{} {:x}]", obj.str(), obj.num());
        return stream;
    }

private:
    StringType m_str;
    int m_num;
};

} // namespace

CATCH_TEMPLATE_TEST_CASE("BasicString", "[string]", char, wchar_t, char8_t, char16_t, char32_t)
{
    using BasicString = fly::BasicString<TestType>;

    using string_type = typename BasicString::string_type;
    using char_type = typename BasicString::char_type;
    using size_type = typename BasicString::size_type;

    CATCH_SECTION("Split a string by a character delimeter")
    {
        static constexpr size_type s_size = 10;
        std::vector<string_type> input_split(s_size);

        constexpr char_type delim = ' ';
        string_type input;

        for (size_type i = 0; i < s_size; ++i)
        {
            const string_type curr = BasicString::generate_random_string(10);

            input += curr + delim;
            input_split[i] = curr;
        }

        const auto output_split = BasicString::split(input, delim);
        CATCH_CHECK(input_split.size() == output_split.size());

        for (size_type i = 0; i < s_size; ++i)
        {
            CATCH_CHECK(input_split[i] == output_split[i]);
        }
    }

    CATCH_SECTION("Split a string by a character delimeter, with a maximum number of results")
    {
        static constexpr size_type s_size = 10;
        static constexpr size_type s_count = 6;
        std::vector<string_type> input_split(s_count);

        constexpr char_type delim = ';';
        string_type input;

        for (size_type i = 0; i < s_size; ++i)
        {
            const string_type curr = BasicString::generate_random_string(10);
            input += curr + delim;

            if (i < s_count)
            {
                input_split[i] = curr;
            }
            else
            {
                input_split.back() += delim;
                input_split.back() += curr;
            }
        }

        const auto output_split = BasicString::split(input, delim, s_count);
        CATCH_CHECK(input_split.size() == output_split.size());

        for (size_type i = 0; i < s_count; ++i)
        {
            CATCH_CHECK(input_split[i] == output_split[i]);
        }
    }

    CATCH_SECTION("Trim whitespace from a string")
    {
        string_type test1;
        string_type test2 = FLY_STR(char_type, "   abc");
        string_type test3 = FLY_STR(char_type, "abc   ");
        string_type test4 = FLY_STR(char_type, "   abc   ");
        string_type test5 = FLY_STR(char_type, " \n\t\r  abc  \n\t\r ");
        string_type test6 = FLY_STR(char_type, " \n\t\r  a   c  \n\t\r ");
        string_type test7 = FLY_STR(char_type, " \n\t\r  a\n \tc  \n\t\r ");

        const string_type expected1;
        const string_type expected2 = FLY_STR(char_type, "abc");
        const string_type expected3 = FLY_STR(char_type, "a   c");
        const string_type expected4 = FLY_STR(char_type, "a\n \tc");

        BasicString::trim(test1);
        BasicString::trim(test2);
        BasicString::trim(test3);
        BasicString::trim(test4);
        BasicString::trim(test5);
        BasicString::trim(test6);
        BasicString::trim(test7);

        CATCH_CHECK(test1 == expected1);
        CATCH_CHECK(test2 == expected2);
        CATCH_CHECK(test3 == expected2);
        CATCH_CHECK(test4 == expected2);
        CATCH_CHECK(test5 == expected2);
        CATCH_CHECK(test6 == expected3);
        CATCH_CHECK(test7 == expected4);
    }

    CATCH_SECTION("Replace all instances of a substring in a string with a character")
    {
        string_type source = FLY_STR(char_type, "To Be Replaced! To Be Replaced!");
        const string_type search = FLY_STR(char_type, "Be Replaced");
        const char_type replace('x');

        BasicString::replace_all(source, search, replace);
        CATCH_CHECK(source == FLY_STR(char_type, "To x! To x!"));
    }

    CATCH_SECTION("Replace all instances of a substring in a string with another string")
    {
        string_type source = FLY_STR(char_type, "To Be Replaced! To Be Replaced!");
        const string_type search = FLY_STR(char_type, "Be Replaced");
        const string_type replace = FLY_STR(char_type, "new value");

        BasicString::replace_all(source, search, replace);
        CATCH_CHECK(source == FLY_STR(char_type, "To new value! To new value!"));
    }

    CATCH_SECTION("Replace all instances of a substring in a string with an empty string")
    {
        string_type source = FLY_STR(char_type, "To Be Replaced! To Be Replaced!");
        const string_type replace = FLY_STR(char_type, "new value");

        BasicString::replace_all(source, string_type(), replace);
        CATCH_CHECK(source == FLY_STR(char_type, "To Be Replaced! To Be Replaced!"));
    }

    CATCH_SECTION("Remove all instances of a substring in a string")
    {
        string_type source = FLY_STR(char_type, "To Be Replaced! To Be Replaced!");
        const string_type search = FLY_STR(char_type, "Be Rep");

        BasicString::remove_all(source, search);
        CATCH_CHECK(source == FLY_STR(char_type, "To laced! To laced!"));
    }

    CATCH_SECTION("Remove all instances of an empty string in a string")
    {
        string_type source = FLY_STR(char_type, "To Be Replaced! To Be Replaced!");

        BasicString::remove_all(source, string_type());
        CATCH_CHECK(source == FLY_STR(char_type, "To Be Replaced! To Be Replaced!"));
    }

    CATCH_SECTION("Check if a wildcarded string matches another string")
    {
        string_type test1, test2;

        test1 = FLY_STR(char_type, "");
        test2 = FLY_STR(char_type, "*");
        CATCH_CHECK(BasicString::wildcard_match(test1, test2));

        test1 = FLY_STR(char_type, "");
        test2 = FLY_STR(char_type, "**");
        CATCH_CHECK(BasicString::wildcard_match(test1, test2));

        test1 = FLY_STR(char_type, "a");
        test2 = FLY_STR(char_type, "a");
        CATCH_CHECK(BasicString::wildcard_match(test1, test2));

        test1 = FLY_STR(char_type, "b");
        test2 = FLY_STR(char_type, "*");
        CATCH_CHECK(BasicString::wildcard_match(test1, test2));

        test1 = FLY_STR(char_type, "c");
        test2 = FLY_STR(char_type, "**");
        CATCH_CHECK(BasicString::wildcard_match(test1, test2));

        test1 = FLY_STR(char_type, "abc");
        test2 = FLY_STR(char_type, "a*");
        CATCH_CHECK(BasicString::wildcard_match(test1, test2));

        test1 = FLY_STR(char_type, "abc");
        test2 = FLY_STR(char_type, "a*c");
        CATCH_CHECK(BasicString::wildcard_match(test1, test2));

        test1 = FLY_STR(char_type, "abc");
        test2 = FLY_STR(char_type, "a*");
        CATCH_CHECK(BasicString::wildcard_match(test1, test2));

        test1 = FLY_STR(char_type, "abc");
        test2 = FLY_STR(char_type, "*b*");
        CATCH_CHECK(BasicString::wildcard_match(test1, test2));

        test1 = FLY_STR(char_type, "abc");
        test2 = FLY_STR(char_type, "*bc");
        CATCH_CHECK(BasicString::wildcard_match(test1, test2));

        test1 = FLY_STR(char_type, "abc");
        test2 = FLY_STR(char_type, "*c");
        CATCH_CHECK(BasicString::wildcard_match(test1, test2));

        test1 = FLY_STR(char_type, "");
        test2 = FLY_STR(char_type, "");
        CATCH_CHECK_FALSE(BasicString::wildcard_match(test1, test2));

        test1 = FLY_STR(char_type, "a");
        test2 = FLY_STR(char_type, "");
        CATCH_CHECK_FALSE(BasicString::wildcard_match(test1, test2));

        test1 = FLY_STR(char_type, "a");
        test2 = FLY_STR(char_type, "b");
        CATCH_CHECK_FALSE(BasicString::wildcard_match(test1, test2));

        test1 = FLY_STR(char_type, "a");
        test2 = FLY_STR(char_type, "b*");
        CATCH_CHECK_FALSE(BasicString::wildcard_match(test1, test2));

        test1 = FLY_STR(char_type, "a");
        test2 = FLY_STR(char_type, "*b");
        CATCH_CHECK_FALSE(BasicString::wildcard_match(test1, test2));

        test1 = FLY_STR(char_type, "abc");
        test2 = FLY_STR(char_type, "a");
        CATCH_CHECK_FALSE(BasicString::wildcard_match(test1, test2));

        test1 = FLY_STR(char_type, "abc");
        test2 = FLY_STR(char_type, "b*");
        CATCH_CHECK_FALSE(BasicString::wildcard_match(test1, test2));

        test1 = FLY_STR(char_type, "abc");
        test2 = FLY_STR(char_type, "*b");
        CATCH_CHECK_FALSE(BasicString::wildcard_match(test1, test2));

        test1 = FLY_STR(char_type, "abc");
        test2 = FLY_STR(char_type, "*d*");
        CATCH_CHECK_FALSE(BasicString::wildcard_match(test1, test2));
    }

    CATCH_SECTION("Generate a random string with a specific size")
    {
        static constexpr size_type s_size = (1 << 10);

        const auto random = BasicString::generate_random_string(s_size);
        CATCH_CHECK(s_size == random.size());
    }

    CATCH_SECTION("Join generic types by a delimeter")
    {
        string_type str = FLY_STR(char_type, "a");
        const char_type *ctr = FLY_STR(char_type, "b");
        const char_type arr[] = {'c', '\0'};
        const char_type chr = 'd';

        const Streamable<string_type> obj(FLY_STR(char_type, "hi"), 0xbeef);

        CATCH_CHECK(FLY_STR(char_type, "a") == BasicString::join('.', str));
        CATCH_CHECK(FLY_STR(char_type, "b") == BasicString::join('.', ctr));
        CATCH_CHECK(FLY_STR(char_type, "c") == BasicString::join('.', arr));
        CATCH_CHECK(FLY_STR(char_type, "d") == BasicString::join('.', chr));
        CATCH_CHECK(FLY_STR(char_type, "a,a") == BasicString::join(',', str, str));
        CATCH_CHECK(FLY_STR(char_type, "a,b") == BasicString::join(',', str, ctr));
        CATCH_CHECK(FLY_STR(char_type, "a,c") == BasicString::join(',', str, arr));
        CATCH_CHECK(FLY_STR(char_type, "a,d") == BasicString::join(',', str, chr));
        CATCH_CHECK(FLY_STR(char_type, "b,a") == BasicString::join(',', ctr, str));
        CATCH_CHECK(FLY_STR(char_type, "b,b") == BasicString::join(',', ctr, ctr));
        CATCH_CHECK(FLY_STR(char_type, "b,c") == BasicString::join(',', ctr, arr));
        CATCH_CHECK(FLY_STR(char_type, "b,d") == BasicString::join(',', ctr, chr));
        CATCH_CHECK(FLY_STR(char_type, "c,a") == BasicString::join(',', arr, str));
        CATCH_CHECK(FLY_STR(char_type, "c,b") == BasicString::join(',', arr, ctr));
        CATCH_CHECK(FLY_STR(char_type, "c,c") == BasicString::join(',', arr, arr));
        CATCH_CHECK(FLY_STR(char_type, "c,d") == BasicString::join(',', arr, chr));
        CATCH_CHECK(FLY_STR(char_type, "d,a") == BasicString::join(',', chr, str));
        CATCH_CHECK(FLY_STR(char_type, "d,b") == BasicString::join(',', chr, ctr));
        CATCH_CHECK(FLY_STR(char_type, "d,c") == BasicString::join(',', chr, arr));
        CATCH_CHECK(FLY_STR(char_type, "d,d") == BasicString::join(',', chr, chr));
        CATCH_CHECK(FLY_STR(char_type, "[hi beef]") == BasicString::join('.', obj));
        CATCH_CHECK(
            FLY_STR(char_type, "a:[hi beef]:c:d") == BasicString::join(':', str, obj, arr, chr));
        CATCH_CHECK(FLY_STR(char_type, "a:c:d") == BasicString::join(':', str, arr, chr));
    }
}

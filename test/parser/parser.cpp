#include "fly/parser/parser.hpp"

#include "fly/types/json/json.hpp"
#include "fly/types/string/string.hpp"

#include <catch2/catch.hpp>

#include <optional>
#include <sstream>
#include <vector>

namespace {

class EofParser : public fly::Parser
{
public:
    void compare(std::vector<int> &&chars) const
    {
        CATCH_CHECK(m_chars == chars);
    }

protected:
    /**
     * Dummy parser to fail if the given stream contains any characters.
     */
    std::optional<fly::Json> parse_internal(std::istream &stream) override
    {
        m_chars.clear();
        int c = 0;

        while ((c = stream.get()) != EOF)
        {
            m_chars.push_back(c);
        }

        if (!m_chars.empty())
        {
            return std::nullopt;
        }

        return fly::Json();
    }

private:
    std::vector<int> m_chars;
};

} // namespace

CATCH_TEST_CASE("Parser", "[parser]")
{
    EofParser parser;

    CATCH_SECTION("Leading byte that is not a byte order mark is not consumed as such")
    {
        CATCH_CHECK_FALSE(parser.parse_string(std::string("\xee")).has_value());
        parser.compare({0xee});
    }

    CATCH_SECTION("UTF-8 byte order mark")
    {
        CATCH_CHECK_FALSE(parser.parse_string(std::string("\xef")).has_value());
        parser.compare({0xef});

        CATCH_CHECK_FALSE(parser.parse_string(std::string("\xef\xee")).has_value());
        parser.compare({0xef, 0xee});

        CATCH_CHECK_FALSE(parser.parse_string(std::string("\xef\xbb")).has_value());
        parser.compare({0xef, 0xbb});

        CATCH_CHECK_FALSE(parser.parse_string(std::string("\xef\xbb\xee")).has_value());
        parser.compare({0xef, 0xbb, 0xee});

        CATCH_CHECK(parser.parse_string(std::string("\xef\xbb\xbf")).has_value());
        parser.compare({});
    }

    CATCH_SECTION("UTF-16 big endian byte order mark")
    {
        std::optional<fly::Json> parsed;

        CATCH_CHECK_FALSE(parser.parse_string(std::string("\xfe")).has_value());
        parser.compare({0xfe});

        CATCH_CHECK_FALSE(parser.parse_string(std::string("\xfe\xee")).has_value());
        parser.compare({0xfe, 0xee});

        CATCH_CHECK(parser.parse_string(std::string("\xfe\xff")).has_value());
        parser.compare({});
    }

    CATCH_SECTION("UTF-16 little endian byte order mark")
    {
        std::optional<fly::Json> parsed;

        CATCH_CHECK_FALSE(parser.parse_string(std::string("\xff")).has_value());
        parser.compare({0xff});

        CATCH_CHECK_FALSE(parser.parse_string(std::string("\xff\xee")).has_value());
        parser.compare({0xff, 0xee});

        CATCH_CHECK(parser.parse_string(std::string("\xff\xfe")).has_value());
        parser.compare({});
    }

    CATCH_SECTION("UTF-32 big endian byte order mark")
    {
        std::optional<fly::Json> parsed;

        CATCH_CHECK_FALSE(parser.parse_string(std::string("\x00", 1)).has_value());
        parser.compare({0x00});

        CATCH_CHECK_FALSE(parser.parse_string(std::string("\x00\xee", 2)).has_value());
        parser.compare({0x00, 0xee});

        CATCH_CHECK_FALSE(parser.parse_string(std::string("\x00\x00", 2)).has_value());
        parser.compare({0x00, 0x00});

        CATCH_CHECK_FALSE(parser.parse_string(std::string("\x00\x00\xee", 3)).has_value());
        parser.compare({0x00, 0x00, 0xee});

        CATCH_CHECK_FALSE(parser.parse_string(std::string("\x00\x00\xfe", 3)).has_value());
        parser.compare({0x00, 0x00, 0xfe});

        CATCH_CHECK_FALSE(parser.parse_string(std::string("\x00\x00\xfe\xee", 4)).has_value());
        parser.compare({0x00, 0x00, 0xfe, 0xee});

        CATCH_CHECK(parser.parse_string(std::string("\x00\x00\xfe\xff", 4)).has_value());
        parser.compare({});
    }

    CATCH_SECTION("UTF-32 little endian byte order mark")
    {
        std::optional<fly::Json> parsed;

        CATCH_CHECK_FALSE(parser.parse_string(std::string("\xff\xfe\x61\x00", 4)).has_value());
        parser.compare({0x61}); // 0xff 0xfe is interpreted as UTF-16 little endian

        CATCH_CHECK_FALSE(parser.parse_string(std::string("\xff\xfe\x00\x61", 4)).has_value());
        parser.compare({0xe6, 0x84, 0x80}); // 0xff 0xfe is interpreted as UTF-16 little endian

        CATCH_CHECK(parser.parse_string(std::string("\xff\xfe\x00\x00", 4)).has_value());
        parser.compare({});
    }
}

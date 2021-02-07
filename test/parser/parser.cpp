#include "fly/parser/parser.hpp"

#include "test/util/path_util.hpp"

#include "fly/types/json/json.hpp"
#include "fly/types/string/string.hpp"

#include "catch2/catch_test_macros.hpp"

#include <filesystem>
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
    std::optional<fly::Json> parse_internal() override
    {
        m_chars.clear();
        int c = 0;

        while ((c = get()) != EOF)
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

    fly::test::PathUtil::ScopedTempDirectory path;
    std::filesystem::path file = path.file();

    auto parse_bytes = [&parser, &file](std::vector<int> &&bytes, std::vector<int> &&compare)
    {
        CATCH_CAPTURE(bytes);

        std::string contents;
        contents.reserve(bytes.size());

        for (int byte : bytes)
        {
            contents.push_back(static_cast<char>(byte));
        }

        CATCH_REQUIRE(fly::test::PathUtil::write_file(file, contents));

        auto result = parser.parse_file(file);

        CATCH_CHECK(result.has_value() == compare.empty());
        parser.compare(std::move(compare));
    };

    CATCH_SECTION("Leading byte that is not a byte order mark is not consumed as such")
    {
        parse_bytes({0xee}, {0xee});
    }

    CATCH_SECTION("UTF-8 byte order mark")
    {
        parse_bytes({0xef}, {0xef});
        parse_bytes({0xef, 0xee}, {0xef, 0xee});
        parse_bytes({0xef, 0xbb}, {0xef, 0xbb});
        parse_bytes({0xef, 0xbb, 0xee}, {0xef, 0xbb, 0xee});

        parse_bytes({0xef, 0xbb, 0xbf}, {});
    }

    CATCH_SECTION("UTF-16 big endian byte order mark")
    {
        parse_bytes({0xfe}, {0xfe});
        parse_bytes({0xfe, 0xee}, {0xfe, 0xee});

        parse_bytes({0xfe, 0xff}, {});
    }

    CATCH_SECTION("UTF-16 little endian byte order mark")
    {
        parse_bytes({0xff}, {0xff});
        parse_bytes({0xff, 0xee}, {0xff, 0xee});

        parse_bytes({0xff, 0xfe}, {});
    }

    CATCH_SECTION("UTF-32 big endian byte order mark")
    {
        parse_bytes({0x00}, {0x00});
        parse_bytes({0x00, 0xee}, {0x00, 0xee});
        parse_bytes({0x00, 0x00}, {0x00, 0x00});
        parse_bytes({0x00, 0x00, 0xee}, {0x00, 0x00, 0xee});
        parse_bytes({0x00, 0x00, 0xfe}, {0x00, 0x00, 0xfe});
        parse_bytes({0x00, 0x00, 0xfe, 0xee}, {0x00, 0x00, 0xfe, 0xee});

        parse_bytes({0x00, 0x00, 0xfe, 0xff}, {});
    }

    CATCH_SECTION("UTF-32 little endian byte order mark")
    {
        // 0xff 0xfe is interpreted as UTF-16 little endian in both of these cases.
        parse_bytes({0xff, 0xfe, 0x61, 0x00}, {0x61});
        parse_bytes({0xff, 0xfe, 0x00, 0x61}, {0xe6, 0x84, 0x80});

        parse_bytes({0xff, 0xfe, 0x00, 0x00}, {});
    }
}

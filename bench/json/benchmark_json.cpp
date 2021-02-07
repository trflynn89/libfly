#include "bench/util/table.hpp"
#include "test/util/path_util.hpp"

#include "fly/fly.hpp"
#include "fly/parser/json_parser.hpp"
#include "fly/types/json/json.hpp"

#include "boost/json/src.hpp"
#include "catch2/catch_test_macros.hpp"
#include "nlohmann/json.hpp"

#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

namespace {

using JsonTable = fly::benchmark::Table<std::string, double, double>;

class JsonParserBase
{
public:
    virtual ~JsonParserBase() = default;
    virtual void parse(const std::filesystem::path &path) = 0;
};

// libfly - https://github.com/trflynn89/libfly
class LibflyJsonParser : public JsonParserBase
{
public:
    void parse(const std::filesystem::path &path) override
    {
        FLY_UNUSED(m_parser.parse_file(path));
    }

private:
    fly::JsonParser m_parser;
};

// Boost.JSON - https://github.com/boostorg/json
class BoostJsonParser : public JsonParserBase
{
public:
    void parse(const std::filesystem::path &path) override
    {
        boost::json::error_code error_code;
        m_parser.reset();

        const std::string contents = fly::test::PathUtil::read_file(path);
        m_parser.write(contents.data(), contents.size(), error_code);

        m_parser.finish(error_code);
        FLY_UNUSED(m_parser.release());
    }

private:
    boost::json::stream_parser m_parser;
};

// JSON for Modern C++ - https://github.com/nlohmann/json
class NLohmannJsonParser : public JsonParserBase
{
public:
    void parse(const std::filesystem::path &path) override
    {
        std::ifstream stream(path);
        FLY_UNUSED(nlohmann::json::parse(stream));
    }
};

} // namespace

CATCH_TEST_CASE("JSON", "[bench]")
{
    static constexpr std::size_t s_iterations = 11;

    const auto here = std::filesystem::path(__FILE__).parent_path();
    const auto root = here.parent_path().parent_path();

    static std::vector<std::filesystem::path> s_test_files {
        root / "test" / "parser" / "json" / "unicode" / "all_unicode.json",
        here / "data" / "canada.json",
        here / "data" / "gsoc-2018.json",
    };

    std::map<std::string, std::unique_ptr<JsonParserBase>> parsers;
#if !defined(FLY_PROFILE)
    parsers.emplace("boost", std::make_unique<BoostJsonParser>());
    parsers.emplace("nlohmann", std::make_unique<NLohmannJsonParser>());
#endif
    parsers.emplace("libfly", std::make_unique<LibflyJsonParser>());

    for (const auto &file : s_test_files)
    {
        JsonTable table(
            "JSON: " + file.filename().string(),
            {"Parser", "Duration (ms)", "Speed (MB/s)"});

        for (auto &parser : parsers)
        {
            std::vector<double> results;

            for (std::size_t i = 0; i < s_iterations; ++i)
            {
                const auto start = std::chrono::system_clock::now();
                parser.second->parse(file);
                const auto end = std::chrono::system_clock::now();

                const auto duration = std::chrono::duration<double>(end - start);
                results.push_back(duration.count());
            }

            std::sort(results.rbegin(), results.rend());

            const auto size = std::filesystem::file_size(file);
            const auto duration = results[s_iterations / 2];
            const auto speed = size / duration / 1024.0 / 1024.0;

            table.append_row(parser.first, duration * 1000, speed);
        }

        std::cout << table << '\n';
    }
}

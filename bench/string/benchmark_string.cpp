#include "bench/util/table.hpp"

#include "fly/types/string/format.hpp"

#include "catch2/catch_test_macros.hpp"
#include "fmt/format.h"

#include <iostream>
#include <map>
#include <sstream>
#include <string>

#if __has_include(<format>)
#    include <format>
#endif

namespace {

using StringTable = fly::benchmark::Table<std::string, double>;
static constexpr std::size_t s_iterations = 1000001;

class StringBase
{
public:
    virtual ~StringBase() = default;
    virtual void format_with_floats() = 0;
    virtual void format_without_floats() = 0;
};

// libfly - https://github.com/trflynn89/libfly
class LibflyFormat : public StringBase
{
public:
    void format_with_floats() override
    {
        FLY_UNUSED(fly::string::format(
            "{:.10f}:{:04}:{:+}:{}:{}:{}:%\n",
            1.234,
            42,
            3.13,
            "str",
            nullptr,
            'X'));
    }

    void format_without_floats() override
    {
        FLY_UNUSED(fly::string::format(
            "{:10}:{:04}:{:+}:{}:{}:{}:%\n",
            1234,
            42,
            313,
            "str",
            nullptr,
            'X'));
    }
};

// {fmt} - https://github.com/fmtlib/fmt
class FmtFormat : public StringBase
{
public:
    void format_with_floats() override
    {
        FLY_UNUSED(
            fmt::format("{:.10f}:{:04}:{:+}:{}:{}:{}:%\n", 1.234, 42, 3.13, "str", nullptr, 'X'));
    }

    void format_without_floats() override
    {
        FLY_UNUSED(
            fmt::format("{:10}:{:04}:{:+}:{}:{}:{}:%\n", 1234, 42, 313, "str", nullptr, 'X'));
    }
};

#if __has_include(<format>)

// std::format
class StdFormat : public StringBase
{
public:
    void format_with_floats() override
    {
        FLY_UNUSED(
            std::format("{:.10f}:{:04}:{:+}:{}:{}:{}:%\n", 1.234, 42, 3.13, "str", nullptr, 'X'));
    }

    void format_without_floats() override
    {
        FLY_UNUSED(
            std::format("{:10}:{:04}:{:+}:{}:{}:{}:%\n", 1234, 42, 313, "str", nullptr, 'X'));
    }
};

#endif

// STL IO Streams
class StreamFormat : public StringBase
{
public:
    void format_with_floats() override
    {
        std::stringstream stream;
        stream << std::setprecision(10) << std::fixed << 1.234 << ':'
               << std::resetiosflags(std::ios::floatfield) << std::setw(4) << std::setfill('0')
               << 42 << std::setfill(' ') << ':' << std::setiosflags(std::ios::showpos) << 3.13
               << std::resetiosflags(std::ios::showpos) << ':' << "str" << ':' << nullptr << ':'
               << 'X' << ":%\n";
    }

    void format_without_floats() override
    {
        std::stringstream stream;
        stream << std::setw(10) << 1234 << ':' << std::setw(4) << std::setfill('0') << 42
               << std::setfill(' ') << ':' << std::setiosflags(std::ios::showpos) << 313
               << std::resetiosflags(std::ios::showpos) << ':' << "str" << ':' << nullptr << ':'
               << 'X' << ":%\n";
    }
};

template <typename WithFloats>
void run_format_test(std::string &&name)
{
    std::map<std::string, std::unique_ptr<StringBase>> formatters;
#if !defined(FLY_PROFILE)
    formatters.emplace("{fmt}", std::make_unique<FmtFormat>());
    formatters.emplace("std::stringstream", std::make_unique<StreamFormat>());
#    if __has_include(<format>)
    formatters.emplace("std::format", std::make_unique<StdFormat>());
#    endif
#endif
    formatters.emplace("libfly", std::make_unique<LibflyFormat>());

    StringTable table(std::move(name), {"Formatter", "Duration (ns)"});

    for (auto &formatter : formatters)
    {
        std::vector<double> results;

        for (std::size_t i = 0; i < s_iterations; ++i)
        {
            auto const start = std::chrono::steady_clock::now();

            if constexpr (std::is_same_v<WithFloats, std::true_type>)
            {
                formatter.second->format_with_floats();
            }
            else
            {
                formatter.second->format_without_floats();
            }

            auto const end = std::chrono::steady_clock::now();

            auto const duration = std::chrono::duration<double>(end - start);
            results.push_back(duration.count());
        }

        std::sort(results.rbegin(), results.rend());

        auto const duration = results[s_iterations / 2];
        table.append_row(formatter.first, duration * 1000 * 1000);
    }

    std::cout << table << '\n';
}

} // namespace

CATCH_TEST_CASE("String", "[bench]")
{
    run_format_test<std::true_type>("Formatting (with floats)");
    run_format_test<std::false_type>("Formatting (without floats)");
}

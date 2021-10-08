#include "fly/logger/styler.hpp"
#include "fly/types/string/string.hpp"

// clang-format off
// Due to a missing #include in catch_reporter_registrars.hpp, this must be included first.
#include "catch2/interfaces/catch_interfaces_reporter.hpp"
// clang-format on

#include "catch2/catch_reporter_registrars.hpp"
#include "catch2/catch_session.hpp"
#include "catch2/catch_test_case_info.hpp"
#include "catch2/reporters/catch_reporter_console.hpp"

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

/**
 * A Catch2 test reporter for reporting colorful test and section names to console.
 */
class FlyReporter : public Catch::ConsoleReporter
{
public:
    explicit FlyReporter(const Catch::ReporterConfig &config);
    ~FlyReporter() override = default;

    static std::string getDescription();

    void testRunStarting(const Catch::TestRunInfo &info) override;
    void testCaseStarting(const Catch::TestCaseInfo &info) override;
    void sectionStarting(const Catch::SectionInfo &info) override;
    void sectionEnded(const Catch::SectionStats &stats) override;
    void testCaseEnded(const Catch::TestCaseStats &stats) override;
    void testRunEnded(const Catch::TestRunStats &stats) override;

private:
    void stream_header(fly::logger::Color::StandardColor color, std::string message);

    std::chrono::steady_clock::time_point m_test_start;
    std::chrono::steady_clock::time_point m_current_test_case_start;

    std::vector<std::string> m_sections;
    std::size_t m_section_level {0};
};

//==================================================================================================
FlyReporter::FlyReporter(const Catch::ReporterConfig &config) : Catch::ConsoleReporter(config)
{
}

//==================================================================================================
std::string FlyReporter::getDescription()
{
    return "Catch2 test reporter for libfly";
}

//==================================================================================================
void FlyReporter::testRunStarting(const Catch::TestRunInfo &info)
{
    Catch::ConsoleReporter::testRunStarting(info);
    m_test_start = std::chrono::steady_clock::now();
}

//==================================================================================================
void FlyReporter::testCaseStarting(const Catch::TestCaseInfo &info)
{
    Catch::ConsoleReporter::testCaseStarting(info);

    stream_header(fly::logger::Color::Green, fly::String::format("{} Test", info.name));
    m_current_test_case_start = std::chrono::steady_clock::now();
}

//==================================================================================================
void FlyReporter::sectionStarting(const Catch::SectionInfo &info)
{
    Catch::ConsoleReporter::sectionStarting(info);
    std::size_t level = m_section_level++;

    if (level == 0)
    {
        m_sections.push_back(info.name);
        return;
    }

    auto section = fly::String::join('/', m_sections.back(), info.name);

    if (auto it = std::find(m_sections.begin(), m_sections.end(), section); it != m_sections.end())
    {
        std::swap(*it, *(m_sections.end() - 1));
        return;
    }

    const fly::logger::Styler style(fly::logger::Color::Cyan, fly::logger::Style::Italic);
    stream << style << "[ ";

    if (level != 1)
    {
        stream << fly::String::format("{: >{}}└─➤ ", "", (level - 2) * 4);
    }

    stream << fly::String::format("{} ]\n", info.name);

    m_sections.push_back(std::move(section));
}

//==================================================================================================
void FlyReporter::sectionEnded(const Catch::SectionStats &stats)
{
    Catch::ConsoleReporter::sectionEnded(stats);
    --m_section_level;
}

//==================================================================================================
void FlyReporter::testCaseEnded(const Catch::TestCaseStats &stats)
{
    Catch::ConsoleReporter::testCaseEnded(stats);

    const auto end = std::chrono::steady_clock::now();
    const auto duration = std::chrono::duration<double>(end - m_current_test_case_start);

    const std::string &name = stats.testInfo->name;

    if (stats.totals.assertions.allOk())
    {
        stream_header(
            fly::logger::Color::Green,
            fly::String::format("PASSED {} ({:.3f} seconds)", name, duration.count()));
    }
    else
    {
        stream_header(
            fly::logger::Color::Red,
            fly::String::format("FAILED {} ({:.3f} seconds)", name, duration.count()));
    }

    stream << '\n';
    m_sections.clear();
}

//==================================================================================================
void FlyReporter::testRunEnded(const Catch::TestRunStats &stats)
{
    Catch::ConsoleReporter::testRunEnded(stats);

    const auto end = std::chrono::steady_clock::now();
    const auto duration = std::chrono::duration<double>(end - m_test_start);

    // ConsoleReporter prints a second newline above, so go up one line before logging the time.
    stream << fly::logger::Styler(fly::logger::Cursor::Up);

    stream << fly::logger::Styler(fly::logger::Style::Bold, fly::logger::Color::Cyan)
           << "Total time ";
    stream << fly::String::format("{:.3f} seconds\n\n", duration.count());
}

//==================================================================================================
void FlyReporter::stream_header(fly::logger::Color::StandardColor color, std::string message)
{
    stream << fly::logger::Styler(fly::logger::Style::Bold, color)
           << fly::String::format("[==== {} ====]\n", message);
}

//==================================================================================================
CATCH_REGISTER_REPORTER("libfly", FlyReporter)

int main(int argc, char **argv)
{
    return Catch::Session().run(argc, argv);
}

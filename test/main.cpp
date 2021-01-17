#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_DEFAULT_REPORTER "libfly"

#include "fly/logger/styler.hpp"
#include "fly/types/string/string.hpp"

#include "catch2/catch.hpp"

/**
 * A Catch2 test reporter for reporting colorful test and section names to console.
 */
class FlyReporter : public Catch::ConsoleReporter
{
public:
    FlyReporter(const Catch::ReporterConfig &config) : Catch::ConsoleReporter(config)
    {
    }

    ~FlyReporter() override = default;

    static std::string getDescription()
    {
        return "Catch2 test reporter for libfly";
    }

    void testRunStarting(const Catch::TestRunInfo &info) override
    {
        Catch::ConsoleReporter::testRunStarting(info);
        m_test_start = std::chrono::system_clock::now();
    }

    void testRunEnded(const Catch::TestRunStats &stats) override
    {
        Catch::ConsoleReporter::testRunEnded(stats);

        const auto end = std::chrono::system_clock::now();
        const auto duration = std::chrono::duration<double>(end - m_test_start);

        // ConsoleReporter prints a second newline above, so go up one line before logging the time.
        stream << fly::Styler(fly::Cursor::Up, fly::Style::Bold, fly::Color::Cyan) << "Total time ";
        fly::String::format(stream, "{:.3f} seconds\n\n", duration.count());
    }

    void testCaseStarting(const Catch::TestCaseInfo &info) override
    {
        const auto style = fly::Styler(fly::Style::Bold, fly::Color::Green);
        fly::String::format(stream, "[{}{:=>4}{} Test{:=<4}]\n", style, ' ', info.name, ' ');

        Catch::ConsoleReporter::testCaseStarting(info);

        m_current_test_case_start = std::chrono::system_clock::now();
        m_current_test_case = info.name;
    }

    void sectionStarting(const Catch::SectionInfo &info) override
    {
        if (info.name != m_current_test_case)
        {
            const auto style = fly::Styler(fly::Style::Italic, fly::Color::Cyan);
            fly::String::format(stream, "{}[ {} ]\n", style, info.name);
        }

        Catch::ConsoleReporter::sectionStarting(info);
    }

    void testCaseEnded(const Catch::TestCaseStats &stats) override
    {
        const auto end = std::chrono::system_clock::now();
        const auto duration = std::chrono::duration<double>(end - m_current_test_case_start);

        const std::string &name = stats.testInfo.name;

        if (stats.totals.assertions.allOk())
        {
            fly::String::format(
                stream,
                "{}[==== PASSED {} ({:.3f} seconds) ====]\n\n",
                fly::Styler(fly::Style::Bold, fly::Color::Green),
                name,
                duration.count());
        }
        else
        {
            fly::String::format(
                stream,
                "{}[==== FAILED {} ({:.3f} seconds) ====]\n\n",
                fly::Styler(fly::Style::Bold, fly::Color::Red),
                name,
                duration.count());
        }

        Catch::ConsoleReporter::testCaseEnded(stats);
        m_current_test_case.clear();
    }

private:
    std::chrono::system_clock::time_point m_test_start;

    std::chrono::system_clock::time_point m_current_test_case_start;
    std::string m_current_test_case;
};

CATCH_REGISTER_REPORTER("libfly", FlyReporter)

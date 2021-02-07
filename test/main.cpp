#include "fly/logger/styler.hpp"
#include "fly/types/string/string.hpp"

//clang-format off
// Due to a missing #include in catch_reporter_registrars.hpp, this must be included first.
#include "catch2/interfaces/catch_interfaces_reporter.hpp"
//clang-format on

#include "catch2/catch_reporter_registrars.hpp"
#include "catch2/catch_session.hpp"
#include "catch2/catch_test_case_info.hpp"
#include "catch2/reporters/catch_reporter_console.hpp"

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
        stream << fly::String::format("{:.3f} seconds\n\n", duration.count());
    }

    void testCaseStarting(const Catch::TestCaseInfo &info) override
    {
        const auto style = fly::Styler(fly::Style::Bold, fly::Color::Green);
        stream << style << fly::String::format("[{:=>4}{} Test{:=<4}]\n", ' ', info.name, ' ');

        Catch::ConsoleReporter::testCaseStarting(info);

        m_current_test_case_start = std::chrono::system_clock::now();
        m_current_test_case = info.name;
    }

    void sectionStarting(const Catch::SectionInfo &info) override
    {
        if (info.name != m_current_test_case)
        {
            const auto style = fly::Styler(fly::Style::Italic, fly::Color::Cyan);
            stream << style << fly::String::format("[ {} ]\n", info.name);
        }

        Catch::ConsoleReporter::sectionStarting(info);
    }

    void testCaseEnded(const Catch::TestCaseStats &stats) override
    {
        const auto end = std::chrono::system_clock::now();
        const auto duration = std::chrono::duration<double>(end - m_current_test_case_start);

        const std::string &name = stats.testInfo->name;

        if (stats.totals.assertions.allOk())
        {
            const auto style = fly::Styler(fly::Style::Bold, fly::Color::Green);
            stream << style;

            stream << fly::String::format(
                "[==== PASSED {} ({:.3f} seconds) ====]\n\n",
                name,
                duration.count());
        }
        else
        {
            const auto style = fly::Styler(fly::Style::Bold, fly::Color::Red);
            stream << style;

            stream << fly::String::format(
                "[==== FAILED {} ({:.3f} seconds) ====]\n\n",
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

int main(int argc, char **argv)
{
    return Catch::Session().run(argc, argv);
}

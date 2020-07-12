#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_DEFAULT_REPORTER "libfly"

#include "fly/logger/styler.hpp"
#include "fly/types/string/string.hpp"

#include <catch2/catch.hpp>

namespace fly::test {

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

    void testCaseStarting(const Catch::TestCaseInfo &info) override
    {
        stream << fly::Styler(fly::Style::Bold, fly::Color::Green)
               << fly::String::format("[==== Test Case: %s ====]", info.name) << '\n';

        Catch::ConsoleReporter::testCaseStarting(info);
        m_current_test_case = info.name;
    }

    void sectionStarting(const Catch::SectionInfo &info) override
    {
        if (info.name != m_current_test_case)
        {
            stream << fly::Styler(fly::Style::Italic, fly::Color::Cyan)
                   << fly::String::format("[ %s ]", info.name) << '\n';
        }

        Catch::ConsoleReporter::sectionStarting(info);
    }

    void testCaseEnded(const Catch::TestCaseStats &stats) override
    {
        const std::string &name = stats.testInfo.name;

        if (stats.totals.assertions.allOk())
        {
            stream << fly::Styler(fly::Style::Bold, fly::Color::Green)
                   << fly::String::format("[==== PASSED %s ====]", name) << "\n\n";
        }
        else
        {
            stream << fly::Styler(fly::Style::Bold, fly::Color::Red)
                   << fly::String::format("[==== FAILED %s ====]", name) << "\n\n";
        }

        Catch::ConsoleReporter::testCaseEnded(stats);
        m_current_test_case.clear();
    }

private:
    std::string m_current_test_case;
};

CATCH_REGISTER_REPORTER("libfly", FlyReporter)

} // namespace fly::test

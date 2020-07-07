#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_DEFAULT_REPORTER "libfly"

#include <catch2/catch.hpp>

namespace Catch {

/**
 * A Catch2 test reporter for reporting colorful test and section names to console.
 */
class FlyReporter : public ConsoleReporter
{
public:
    FlyReporter(const ReporterConfig &config) : ConsoleReporter(config)
    {
    }

    ~FlyReporter() override = default;

    static std::string getDescription()
    {
        return "Catch2 test reporter for libfly";
    }

    void testCaseStarting(const TestCaseInfo &info) override
    {
        stream << Colour(Colour::BrightGreen) << "[==== Test Case: " << info.name << " ====]\n";

        ConsoleReporter::testCaseStarting(info);
        m_current_test_case = info.name;
    }

    void sectionStarting(const SectionInfo &info) override
    {
        if (info.name != m_current_test_case)
        {
            // Explicitly flush the stream so this output is not included in tests capturing stdout.
            stream << Colour(Colour::Blue) << "[ " << info.name << " ]" << std::endl;
        }

        ConsoleReporter::sectionStarting(info);
    }

    void testCaseEnded(const TestCaseStats &stats) override
    {
        const std::string &name = stats.testInfo.name;

        if (stats.totals.assertions.allOk())
        {
            stream << Colour(Colour::ResultSuccess) << "[==== PASSED " << name << " ====]\n\n";
        }
        else
        {
            stream << Colour(Colour::ResultError) << "[==== FAILED " << name << " ====]\n\n";
        }

        ConsoleReporter::testCaseEnded(stats);
        m_current_test_case.clear();
    }

private:
    std::string m_current_test_case;
};

CATCH_REGISTER_REPORTER("libfly", FlyReporter)

} // namespace Catch

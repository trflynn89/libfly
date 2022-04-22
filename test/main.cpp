#include "fly/logger/styler.hpp"
#include "fly/types/numeric/literals.hpp"
#include "fly/types/string/string.hpp"

#include "catch2/catch_session.hpp"
#include "catch2/catch_test_case_info.hpp"
#include "catch2/reporters/catch_reporter_registrars.hpp"
#include "catch2/reporters/catch_reporter_streaming_base.hpp"

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

using namespace fly::literals::numeric_literals;

/**
 * A Catch2 test reporter for reporting colorful test and section names to console.
 */
class FlyReporter final : public Catch::StreamingReporterBase
{
public:
    explicit FlyReporter(Catch::ReporterConfig const &config);
    ~FlyReporter() override = default;

    static std::string getDescription();

    void testRunStarting(Catch::TestRunInfo const &info) override;
    void testCaseStarting(Catch::TestCaseInfo const &info) override;
    void sectionStarting(Catch::SectionInfo const &info) override;
    void assertionEnded(Catch::AssertionStats const &) override;
    void sectionEnded(Catch::SectionStats const &stats) override;
    void testCaseEnded(Catch::TestCaseStats const &stats) override;
    void testRunEnded(Catch::TestRunStats const &stats) override;

private:
    void stream_header(fly::logger::Color::StandardColor color, std::string message);
    void stream_summary(Catch::Totals const &totals);

    std::chrono::steady_clock::time_point m_test_start;
    std::chrono::steady_clock::time_point m_current_test_case_start;

    std::vector<std::string> m_sections;
    std::size_t m_section_level {0};
};

/**
 * A helper to log information about a single failed assertion during text execution.
 */
class FailedAssertionLogger
{
public:
    explicit FailedAssertionLogger(Catch::AssertionStats const &stats);
    friend std::ostream &operator<<(std::ostream &stream, FailedAssertionLogger const &logger);

private:
    void stream_source_info(std::ostream &stream) const;
    void stream_expression(std::ostream &stream) const;
    void stream_message(std::ostream &stream) const;

    Catch::AssertionResult const &m_result;
    std::vector<Catch::MessageInfo> const &m_messages;
    std::string_view m_label;
};

//==================================================================================================
FlyReporter::FlyReporter(Catch::ReporterConfig const &config) :
    Catch::StreamingReporterBase(config)
{
}

//==================================================================================================
std::string FlyReporter::getDescription()
{
    return "Catch2 test reporter for libfly";
}

//==================================================================================================
void FlyReporter::testRunStarting(Catch::TestRunInfo const &info)
{
    Catch::StreamingReporterBase::testRunStarting(info);
    m_test_start = std::chrono::steady_clock::now();
}

//==================================================================================================
void FlyReporter::testCaseStarting(Catch::TestCaseInfo const &info)
{
    Catch::StreamingReporterBase::testCaseStarting(info);

    stream_header(fly::logger::Color::Green, fly::String::format("{} Test", info.name));
    m_current_test_case_start = std::chrono::steady_clock::now();
}

//==================================================================================================
void FlyReporter::sectionStarting(Catch::SectionInfo const &info)
{
    Catch::StreamingReporterBase::sectionStarting(info);
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

    fly::logger::Styler const style(fly::logger::Color::Cyan, fly::logger::Style::Italic);
    m_stream << style << "[ ";

    if (level != 1)
    {
        m_stream << fly::String::format("{: >{}}└─➤ ", "", (level - 2) * 4);
    }

    m_stream << fly::String::format("{} ]\n", info.name);

    m_sections.push_back(std::move(section));
}

//==================================================================================================
void FlyReporter::assertionEnded(Catch::AssertionStats const &stats)
{
    if (!stats.assertionResult.isOk())
    {
        FailedAssertionLogger logger(stats);
        m_stream << logger << '\n';
    }
}

//==================================================================================================
void FlyReporter::sectionEnded(Catch::SectionStats const &stats)
{
    Catch::StreamingReporterBase::sectionEnded(stats);
    --m_section_level;
}

//==================================================================================================
void FlyReporter::testCaseEnded(Catch::TestCaseStats const &stats)
{
    Catch::StreamingReporterBase::testCaseEnded(stats);

    auto const end = std::chrono::steady_clock::now();
    auto const duration = std::chrono::duration<double>(end - m_current_test_case_start);

    std::string const &name = stats.testInfo->name;

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

    m_stream << '\n';
    m_sections.clear();
}

//==================================================================================================
void FlyReporter::testRunEnded(Catch::TestRunStats const &stats)
{
    Catch::StreamingReporterBase::testRunEnded(stats);

    auto const end = std::chrono::steady_clock::now();
    auto const duration = std::chrono::duration<double>(end - m_test_start);

    stream_summary(stats.totals);

    m_stream << fly::logger::Styler(fly::logger::Style::Bold, fly::logger::Color::Cyan)
             << "Total time ";
    m_stream << fly::String::format("{:.3f} seconds\n", duration.count());
}

//==================================================================================================
void FlyReporter::stream_header(fly::logger::Color::StandardColor color, std::string message)
{
    m_stream << fly::logger::Styler(fly::logger::Style::Bold, color)
             << fly::String::format("[==== {} ====]\n", message);
}

//==================================================================================================
void FlyReporter::stream_summary(Catch::Totals const &totals)
{
    static constexpr std::size_t s_divider_width = 80;

    auto stream_divider = [this](auto color, auto width) {
        m_stream << fly::logger::Styler(fly::logger::Style::Bold, color)
                 << fly::String::format("{:=>{}}", "", width);
    };

    if (totals.testCases.total() == 0)
    {
        stream_divider(fly::logger::Color::Yellow, s_divider_width);
    }
    else
    {
        auto compute_ratio = [](auto numerator, auto denominator) -> std::size_t {
            auto ratio = s_divider_width * static_cast<std::size_t>(numerator);
            ratio /= static_cast<std::size_t>(denominator);

            return ((ratio == 0) && (numerator > 0)) ? 1_zu : ratio;
        };

        std::size_t fail_ratio = compute_ratio(totals.testCases.failed, totals.testCases.total());
        stream_divider(fly::logger::Color::Red, fail_ratio);

        std::size_t pass_ratio = compute_ratio(totals.testCases.passed, totals.testCases.total());
        stream_divider(fly::logger::Color::Green, pass_ratio);
    }

    m_stream << '\n';

    auto pluralise = [](auto number, auto label) {
        if (number == 1)
            return fly::String::format("{} {}", number, label);
        return fly::String::format("{} {}s", number, label);
    };

    if (totals.testCases.total() == 0)
    {
        m_stream << fly::logger::Styler(fly::logger::Color::Yellow) << "No tests ran\n";
    }
    else if (totals.assertions.total() > 0 && totals.testCases.allPassed())
    {
        m_stream << fly::logger::Styler(fly::logger::Style::Bold, fly::logger::Color::Green)
                 << "All tests passed";
        m_stream << " (" << pluralise(totals.assertions.passed, "assertion") << " in "
                 << pluralise(totals.testCases.passed, "test case") << ')' << '\n';
    }
    else
    {
        m_stream << fly::logger::Styler(fly::logger::Color::Red) << "Failed";
        m_stream << fly::String::format(
            " {} of {}\n",
            totals.testCases.failed,
            pluralise(totals.testCases.total(), "test case"));

        m_stream << fly::logger::Styler(fly::logger::Color::Red) << "Failed";
        m_stream << fly::String::format(
            " {} of {}\n",
            totals.assertions.failed,
            pluralise(totals.assertions.total(), "assertion"));
    }
}

//==================================================================================================
FailedAssertionLogger::FailedAssertionLogger(Catch::AssertionStats const &stats) :
    m_result(stats.assertionResult),
    m_messages(stats.infoMessages)
{
    switch (m_result.getResultType())
    {
        case Catch::ResultWas::ExpressionFailed:
            if (!m_messages.empty())
            {
                m_label = "Failed with message";
            }
            break;
        case Catch::ResultWas::ExplicitFailure:
            if (!m_messages.empty())
            {
                m_label = "Failed explicitly with message";
            }
            break;
        case Catch::ResultWas::FatalErrorCondition:
            m_label = "Failed due to a fatal error condition";
            break;
        case Catch::ResultWas::ThrewException:
            m_label = "Failed due to unexpected exception with message";
            break;
        case Catch::ResultWas::DidntThrowException:
            m_label = "Failed because no exception was thrown where one was expected";
            break;
        default:
            break;
    }
}

//==================================================================================================
std::ostream &operator<<(std::ostream &stream, FailedAssertionLogger const &logger)
{
    logger.stream_source_info(stream);
    logger.stream_expression(stream);
    logger.stream_message(stream);
    return stream;
}

//==================================================================================================
void FailedAssertionLogger::stream_source_info(std::ostream &stream) const
{
    stream << fly::logger::Styler(fly::logger::Style::Bold, fly::logger::Color::Red)
           << m_result.getSourceInfo() << ":\n";
}

//==================================================================================================
void FailedAssertionLogger::stream_expression(std::ostream &stream) const
{
    if (m_result.hasExpression())
    {
        stream << fly::logger::Styler(fly::logger::Color::Cyan) << "    "
               << m_result.getExpressionInMacro() << '\n';
    }
}

//==================================================================================================
void FailedAssertionLogger::stream_message(std::ostream &stream) const
{
    if (!m_label.empty())
    {
        stream << m_label << ':' << '\n';
    }

    for (auto const &message : m_messages)
    {
        stream << "    " << message.message << '\n';
    }
}

//==================================================================================================
CATCH_REGISTER_REPORTER("libfly", FlyReporter)

int main(int argc, char **argv)
{
    return Catch::Session().run(argc, argv);
}

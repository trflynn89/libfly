#include "fly/logger/logger.hpp"
#include "fly/logger/logger_config.hpp"
#include "fly/logger/sink.hpp"
#include "fly/logger/styler.hpp"
#include "fly/types/string/string.hpp"

#include "catch2/catch_session.hpp"
#include "catch2/catch_test_case_info.hpp"
#include "catch2/reporters/catch_reporter_registrars.hpp"
#include "catch2/reporters/catch_reporter_streaming_base.hpp"

#include <string>

/**
 * Log sink to drop all received logs.
 */
class DropSink : public fly::logger::Sink
{
public:
    bool initialize() override
    {
        return true;
    }

    bool stream(fly::logger::Log &&) override
    {
        return true;
    }
};

/**
 * A Catch2 test reporter for printing nothing except a header before test cases.
 */
class SilentReporter final : public Catch::StreamingReporterBase
{
public:
    explicit SilentReporter(Catch::ReporterConfig const &config) :
        StreamingReporterBase(config)
    {
    }

    ~SilentReporter() final = default;

    static std::string getDescription()
    {
        return "Catch2 silent reporter for libfly benchmarking";
    }

    void assertionStarting(Catch::AssertionInfo const &) override
    {
    }

    void assertionEnded(Catch::AssertionStats const &) override
    {
    }

    void testCaseStarting(Catch::TestCaseInfo const &info) override
    {
        auto const style = fly::logger::Styler(fly::logger::Style::Bold, fly::logger::Color::Cyan);
        m_stream << style << fly::String::format("[{:=>13}{}{:=<13}]\n\n", ' ', info.name, ' ');
    }
};

CATCH_REGISTER_REPORTER("libfly", SilentReporter)

int main(int argc, char **argv)
{
    fly::logger::Logger::set_default_logger(fly::logger::Logger::create(
        "silent",
        std::make_shared<fly::logger::LoggerConfig>(),
        std::make_unique<DropSink>()));

    return Catch::Session().run(argc, argv);
}

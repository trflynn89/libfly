#define CATCH_CONFIG_RUNNER
#define CATCH_CONFIG_DEFAULT_REPORTER "silent"

#include "fly/logger/log_sink.hpp"
#include "fly/logger/logger.hpp"
#include "fly/logger/logger_config.hpp"
#include "fly/logger/styler.hpp"
#include "fly/types/string/string.hpp"

#include <catch2/catch.hpp>

#include <string>

/**
 * Log sink to drop all received logs.
 */
class DropSink : public fly::LogSink
{
public:
    bool initialize() override
    {
        return true;
    }

    bool stream(fly::Log &&) override
    {
        return true;
    }
};

/**
 * A Catch2 test reporter for printing nothing except a header before test cases.
 */
class SilentReporter final : public Catch::StreamingReporterBase<SilentReporter>
{
public:
    explicit SilentReporter(const Catch::ReporterConfig &config) : StreamingReporterBase(config)
    {
    }

    ~SilentReporter() final = default;

    static std::string getDescription()
    {
        return "Catch2 silent reporter for libfly benchmarking";
    }

    void assertionStarting(const Catch::AssertionInfo &) final
    {
    }

    bool assertionEnded(const Catch::AssertionStats &) final
    {
        return true;
    }

    void testCaseStarting(const Catch::TestCaseInfo &info) override
    {
        stream << fly::Styler(fly::Style::Bold, fly::Color::Cyan)
               << fly::String::format("[============ %s ============]", info.name) << "\n\n";
    }
};

CATCH_REGISTER_REPORTER("silent", SilentReporter)

int main(int argc, char **argv)
{
    fly::Logger::set_default_logger(fly::Logger::create_logger(
        "silent",
        std::make_shared<fly::LoggerConfig>(),
        std::make_unique<DropSink>()));

    return Catch::Session().run(argc, argv);
}

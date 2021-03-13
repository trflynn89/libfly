#include "test/util/path_util.hpp"

#include "fly/coders/coder_config.hpp"
#include "fly/coders/huffman/huffman_decoder.hpp"
#include "fly/fly.hpp"
#include "fly/logger/logger.hpp"
#include "fly/logger/logger_config.hpp"
#include "fly/types/numeric/literals.hpp"
#include "fly/types/string/string.hpp"

#if defined(FLY_LINUX)
#    include "test/mock/mock_system.hpp"
#endif

#include "catch2/catch_test_macros.hpp"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>

using namespace fly::literals::numeric_literals;

namespace {

/**
 * Subclass of the logger config to decrease the default log file size for faster testing.
 */
class MutableLoggerConfig : public fly::logger::LoggerConfig
{
public:
    MutableLoggerConfig() noexcept : fly::logger::LoggerConfig()
    {
        m_default_max_log_file_size = 1 << 10;
    }

    void disable_compression()
    {
        m_default_compress_log_files = false;
    }
};

/**
 * Subclass of the coder config to contain invalid values.
 */
class MutableCoderConfig : public fly::coders::CoderConfig
{
public:
    void invalidate_max_code_length()
    {
        m_default_huffman_encoder_max_code_length =
            std::numeric_limits<fly::coders::code_type>::digits;
    }
};

/**
 * Find the current log file used by the file sink.
 *
 * @param path Directory containing the log file(s).
 *
 * @return The current log file.
 */
std::filesystem::path find_log_file(const fly::test::PathUtil::ScopedTempDirectory &path)
{
    std::uint32_t most_recent_log_index = 0_u32;
    std::filesystem::path most_recent_log_file;

    for (auto &it : std::filesystem::directory_iterator(path()))
    {
        std::vector<std::string> segments = fly::String::split(it.path().filename().string(), '_');
        auto log_index = fly::String::convert<std::uint32_t>(segments[1]);

        if (log_index && (*log_index > most_recent_log_index))
        {
            most_recent_log_index = *log_index;
            most_recent_log_file = it.path();
        }
    }

    return most_recent_log_file;
}

/**
 * Measure the size, in bytes, of a log point.
 *
 * @param string Message to store in the log.
 *
 * @return uintmax_t Size of the log point.
 */
std::uintmax_t log_size(const std::string &message)
{
    fly::logger::Log log;

    log.m_message = message;
    log.m_level = fly::logger::Level::Debug;
    log.m_trace = {__FILE__, __FUNCTION__, static_cast<std::uint32_t>(__LINE__)};

    return fly::String::format("{}\t{}", 1, log).length();
}

} // namespace

CATCH_TEST_CASE("FileLogger", "[logger]")
{
    auto logger_config = std::make_shared<MutableLoggerConfig>();
    auto coder_config = std::make_shared<MutableCoderConfig>();
    fly::test::PathUtil::ScopedTempDirectory path;

    auto logger =
        fly::logger::Logger::create_file_logger("test", logger_config, coder_config, path());

    CATCH_SECTION("Valid logger file paths should be created after creating logger")
    {
        std::filesystem::path log_file = find_log_file(path);
        CATCH_CHECK(log_file.string().starts_with(path().string()));

        CATCH_REQUIRE(std::filesystem::exists(log_file));
    }

    CATCH_SECTION("Cannot start logger with a bad file path")
    {
        logger =
            fly::logger::Logger::create_file_logger("test", logger_config, coder_config, __FILE__);
        CATCH_CHECK(logger == nullptr);
    }

#if defined(FLY_LINUX)

    CATCH_SECTION("Writing to log file fails due to ::write() system call")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Write);

        logger->debug("This log will be received");
        logger->debug("This log will be rejected");

        std::filesystem::path log_file = find_log_file(path);
        const std::string contents = fly::test::PathUtil::read_file(log_file);
        CATCH_REQUIRE(contents.empty());
    }

#endif

    CATCH_SECTION("Debug log points")
    {
        logger->debug("Debug Log");

        std::filesystem::path log_file = find_log_file(path);
        const std::string contents = fly::test::PathUtil::read_file(log_file);
        CATCH_REQUIRE_FALSE(contents.empty());

        CATCH_CHECK(contents.find("Debug Log") != std::string::npos);
    }

    CATCH_SECTION("Informational log points")
    {
        logger->info("Info Log");

        std::filesystem::path log_file = find_log_file(path);
        const std::string contents = fly::test::PathUtil::read_file(log_file);
        CATCH_REQUIRE_FALSE(contents.empty());

        CATCH_CHECK(contents.find("Info Log") != std::string::npos);
    }

    CATCH_SECTION("Warning log points")
    {
        logger->warn("Warning Log");

        std::filesystem::path log_file = find_log_file(path);
        const std::string contents = fly::test::PathUtil::read_file(log_file);
        CATCH_REQUIRE_FALSE(contents.empty());

        CATCH_CHECK(contents.find("Warning Log") != std::string::npos);
    }

    CATCH_SECTION("Error log points")
    {
        logger->error("Error Log");

        std::filesystem::path log_file = find_log_file(path);
        const std::string contents = fly::test::PathUtil::read_file(log_file);
        CATCH_REQUIRE_FALSE(contents.empty());

        CATCH_CHECK(contents.find("Error Log") != std::string::npos);
    }

    CATCH_SECTION("Logger should compress log files by default")
    {
        std::filesystem::path log_file = find_log_file(path);

        std::uintmax_t max_log_file_size = logger_config->max_log_file_size();
        std::uint32_t max_message_size = logger_config->max_message_size();

        std::string random = fly::String::generate_random_string(max_message_size);

        std::uintmax_t expected_size = log_size(random);
        std::uintmax_t count = 0;

        // Create enough log points to fill the log file, plus some extra to start a second log.
        while (++count < ((max_log_file_size / expected_size) + 10))
        {
            logger->debug("{}", random);
        }

        CATCH_CHECK(log_file != find_log_file(path));

        std::filesystem::path compressed_path = log_file;
        compressed_path.replace_extension(".log.enc");

        CATCH_REQUIRE_FALSE(std::filesystem::exists(log_file));
        CATCH_REQUIRE(std::filesystem::exists(compressed_path));

        fly::coders::HuffmanDecoder decoder;
        CATCH_REQUIRE(decoder.decode_file(compressed_path, log_file));

        std::uintmax_t actual_size = std::filesystem::file_size(log_file);
        CATCH_CHECK(actual_size >= max_message_size);
    }

    CATCH_SECTION("When compression is disabled, logger should produce uncompressed logs")
    {
        logger_config->disable_compression();

        std::filesystem::path log_file = find_log_file(path);

        std::uintmax_t max_log_file_size = logger_config->max_log_file_size();
        std::uint32_t max_message_size = logger_config->max_message_size();

        std::string random = fly::String::generate_random_string(max_message_size);

        std::uintmax_t expected_size = log_size(random);
        std::uintmax_t count = 0;

        // Create enough log points to fill the log file, plus some extra to start a second log.
        while (++count < ((max_log_file_size / expected_size) + 10))
        {
            logger->debug("{}", random);
        }

        CATCH_CHECK(log_file != find_log_file(path));
        CATCH_REQUIRE(std::filesystem::exists(log_file));

        fly::coders::HuffmanDecoder decoder;
        CATCH_CHECK_FALSE(decoder.decode_file(log_file, path.file()));

        std::uintmax_t actual_size = std::filesystem::file_size(log_file);
        CATCH_CHECK(actual_size >= max_message_size);
    }

    CATCH_SECTION("When compression fails, logger should produce uncompressed logs")
    {
        coder_config->invalidate_max_code_length();

        std::filesystem::path log_file = find_log_file(path);

        std::uintmax_t max_log_file_size = logger_config->max_log_file_size();
        std::uint32_t max_message_size = logger_config->max_message_size();

        std::string random = fly::String::generate_random_string(max_message_size);

        std::uintmax_t expected_size = log_size(random);
        std::uintmax_t count = 0;

        // Create enough log points to fill the log file, plus some extra to start a second log.
        while (++count < ((max_log_file_size / expected_size) + 10))
        {
            logger->debug("{}", random);
        }

        CATCH_CHECK(log_file != find_log_file(path));
        CATCH_REQUIRE(std::filesystem::exists(log_file));

        fly::coders::HuffmanDecoder decoder;
        CATCH_CHECK_FALSE(decoder.decode_file(log_file, path.file()));

        std::uintmax_t actual_size = std::filesystem::file_size(log_file);
        CATCH_CHECK(actual_size >= max_message_size);
    }
}

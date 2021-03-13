#pragma once

#include "fly/logger/log_sink.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>

namespace fly::coders {
class CoderConfig;
} // namespace fly::coders

namespace fly::logger {
class LoggerConfig;
struct Log;
} // namespace fly::logger

namespace fly::logger::detail {

/**
 * A log sink for streaming log points to a file. Log files are size-limted, rotated, and optionally
 * compressed.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version October 11, 2020
 */
class FileSink : public fly::logger::LogSink
{
public:
    /**
     * Constructor.
     *
     * @param logger_config Reference to the logger configuration.
     * @param coder_config Reference to the coder configuration.
     * @param logger_directory Path to store the log files.
     */
    FileSink(
        std::shared_ptr<fly::logger::LoggerConfig> logger_config,
        std::shared_ptr<fly::coders::CoderConfig> coder_config,
        std::filesystem::path logger_directory);

    /**
     * Create the initial log file.
     *
     * @return True if the log file could be created.
     */
    bool initialize() override;

    /**
     * Stream the given log point to the currently opened file. If the log file has exceeded its
     * maximum size after streaming, rotate the log file.
     *
     * @param log The log point to stream.
     *
     * @return True if the log file is healthy, and if needed, a new log file could be created.
     */
    bool stream(fly::logger::Log &&log) override;

private:
    /**
     * Create a log file. If a log file is already open, close it.
     *
     * @return True if the log file could be created.
     */
    bool create_log_file();

    std::shared_ptr<fly::logger::LoggerConfig> m_logger_config;
    std::shared_ptr<fly::coders::CoderConfig> m_coder_config;

    const std::filesystem::path m_log_directory;
    std::filesystem::path m_log_file;
    std::ofstream m_log_stream;
    std::uint32_t m_log_index {0};
};

} // namespace fly::logger::detail

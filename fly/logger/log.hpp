#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>

namespace fly {

class LoggerConfig;

/**
 * Struct to store data about single log. A log contains:
 *
 * 1. The monotonically increasing index of the log.
 * 2. The log level.
 * 3. The time the log was made.
 * 4. The file name the log is in.
 * 5. The function name the log is in.
 * 6. The line number the log is on.
 * 7. The message being logged.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 18, 2016
 */
struct Log
{
    /**
     * Enumeration to define the level of a log.
     */
    enum class Level : std::uint8_t
    {
        Debug,
        Info,
        Warn,
        Error,

        NumLevels
    };

    /**
     * Default constructor.
     */
    Log() = default;

    /**
     * Move constructor.
     */
    Log(Log &&log) noexcept;

    /**
     * Constructor. Initialize with a message.
     *
     * @param config Reference to the logger config.
     * @param message Message to store.
     */
    Log(const std::shared_ptr<LoggerConfig> &config, std::string &&message) noexcept;

    /**
     * Move assignment operator.
     */
    Log &operator=(Log &&log) noexcept;

    std::uintmax_t m_index {0};
    Level m_level {Level::NumLevels};
    double m_time {-1.0};
    const char *m_file;
    const char *m_function;
    std::uint32_t m_line {0};
    std::string m_message;

    friend std::ostream &operator<<(std::ostream &stream, const Log &log);

    friend std::ostream &operator<<(std::ostream &stream, const Level &level);
};

} // namespace fly

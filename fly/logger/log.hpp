#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>

namespace fly::logger {

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
 * Structure to store trace information about a log point.
 */
struct Trace
{
    std::string_view m_file;
    std::string_view m_function;
    std::uint32_t m_line {0};
};

/**
 * Struct to store data about single log. A log contains:
 *
 * 1. The monotonically increasing index of the log.
 * 2. The log level.
 * 3. The time the log was made.
 * 4. Trace information about the log point (file name, function name, line number).
 * 5. The message being logged.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 18, 2016
 */
struct Log
{
    /**
     * Default constructor.
     */
    Log() = default;

    /**
     * Constructor. Initialize with a message.
     *
     * @param trace The trace information for the log point.
     * @param config Reference to the logger config.
     * @param message Message to store.
     */
    Log(Trace &&trace, std::string &&message, std::uint32_t max_message_size) noexcept;

    /**
     * Move constructor.
     */
    Log(Log &&log) noexcept;

    /**
     * Move assignment operator.
     */
    Log &operator=(Log &&log) noexcept;

    std::uintmax_t m_index {0};
    Level m_level {Level::NumLevels};
    Trace m_trace {};
    double m_time {-1.0};
    std::string m_message;
};

std::ostream &operator<<(std::ostream &stream, const Log &log);
std::ostream &operator<<(std::ostream &stream, const Level &level);
std::ostream &operator<<(std::ostream &stream, const Trace &trace);

} // namespace fly::logger

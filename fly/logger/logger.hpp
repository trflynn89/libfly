#pragma once

#include "fly/logger/detail/logger_macros.hpp"
#include "fly/logger/log.hpp"
#include "fly/system/system.hpp"
#include "fly/types/string/string.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>

/**
 * Add a debug log point to the default logger with trace information.
 *
 * At minimum, a format string is required as the first argument. Subsequent arguments are used to
 * format that string. For example:
 *
 *   LOGD("This is a message");
 *   LOGD("This is message number %d", 10);
 */
#define LOGD(...)                                                                                  \
    do                                                                                             \
    {                                                                                              \
        fly::Logger::get_default_logger()->debug(                                                  \
            {__FILE__, __FUNCTION__, static_cast<std::uint32_t>(__LINE__)},                        \
            FLY_FORMAT_STRING(__VA_ARGS__) FLY_FORMAT_ARGS(__VA_ARGS__));                          \
    } while (0)

/**
 * Add an informational log point to the default logger with trace information.
 *
 * At minimum, a format string is required as the first argument. Subsequent arguments are used to
 * format that string. For example:
 *
 *   LOGI("This is a message");
 *   LOGI("This is message number %d", 10);
 */
#define LOGI(...)                                                                                  \
    do                                                                                             \
    {                                                                                              \
        fly::Logger::get_default_logger()->info(                                                   \
            {__FILE__, __FUNCTION__, static_cast<std::uint32_t>(__LINE__)},                        \
            FLY_FORMAT_STRING(__VA_ARGS__) FLY_FORMAT_ARGS(__VA_ARGS__));                          \
    } while (0)

/**
 * Add a warning log point to the default logger with trace information.
 *
 * At minimum, a format string is required as the first argument. Subsequent arguments are used to
 * format that string. For example:
 *
 *   LOGW("This is a message");
 *   LOGW("This is message number %d", 10);
 */
#define LOGW(...)                                                                                  \
    do                                                                                             \
    {                                                                                              \
        fly::Logger::get_default_logger()->warn(                                                   \
            {__FILE__, __FUNCTION__, static_cast<std::uint32_t>(__LINE__)},                        \
            FLY_FORMAT_STRING(__VA_ARGS__) FLY_FORMAT_ARGS(__VA_ARGS__));                          \
    } while (0)

/**
 * Add a system warning log point to the default logger with trace information. The log point will
 * include the system's last error code and message.
 *
 * At minimum, a format string is required as the first argument. Subsequent arguments are used to
 * format that string. For example:
 *
 *   LOGS("This is a message");
 *   LOGS("This is message number %d", 10);
 */
#define LOGS(...)                                                                                  \
    do                                                                                             \
    {                                                                                              \
        fly::Logger::get_default_logger()->warn(                                                   \
            {__FILE__, __FUNCTION__, static_cast<std::uint32_t>(__LINE__)},                        \
            FLY_FORMAT_STRING(__VA_ARGS__) ": %s" FLY_FORMAT_ARGS(__VA_ARGS__),                    \
            fly::System::get_error_string());                                                      \
    } while (0)

/**
 * Add an error log point to the default logger with trace information.
 *
 * At minimum, a format string is required as the first argument. Subsequent arguments are used to
 * format that string. For example:
 *
 *   LOGE("This is a message");
 *   LOGE("This is message number %d", 10);
 */
#define LOGE(...)                                                                                  \
    do                                                                                             \
    {                                                                                              \
        fly::Logger::get_default_logger()->error(                                                  \
            {__FILE__, __FUNCTION__, static_cast<std::uint32_t>(__LINE__)},                        \
            FLY_FORMAT_STRING(__VA_ARGS__) FLY_FORMAT_ARGS(__VA_ARGS__));                          \
    } while (0)

namespace fly::detail {
class Registry;
} // namespace fly::detail

namespace fly {

class CoderConfig;
class LoggerConfig;
class SequencedTaskRunner;
class LogSink;

/**
 * Logging class to provide configurable instrumentation. There are 4 levels of instrumentation:
 *
 * 1. Debug = Common points.
 * 2. Informational = Less common, event based points.
 * 3. Warning = Something went wrong, but the system is OK.
 * 4. Error = Something went wrong, and the sytem is not OK.
 *
 * This class manages creating log points, but delegates the streaming of those log points to a log
 * sink. Sinks may stream log points however they wish, for example to the console or to a file.
 *
 * Loggers may be created as synchronous or asynchronous loggers. With synchronous loggers, the
 * log sink receives the log points immediately on the same thread they are created. Asynchronous
 * loggers defer handing the log point to the sink to a dedicated thread sequence.
 *
 * Any number of loggers may be created. By default, a synchronous console logger will be used, but
 * callers may override the default logger.
 *
 * The logging macros above may be used to add log points to the default logger. They are useful for
 * providing trace information about the log point (e.g. file name, line number). The logging macros
 * support up to and including 50 format arguments. If more are needed, invoke the logger's public
 * logging methods directly.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 21, 2016
 */
class Logger : public std::enable_shared_from_this<Logger>
{
public:
    /**
     * Destructor.
     */
    ~Logger();

    /**
     * Create a synchronous logger with the provided log sink.
     *
     * @param name Name of the logger to create.
     * @param logger_config Reference to the logger configuration.
     * @param sink The log sink to receive log points for streaming.
     *
     * @return The created logger, or null if the logger could not be initialized.
     */
    static std::shared_ptr<Logger> create_logger(
        const std::string &name,
        const std::shared_ptr<LoggerConfig> &logger_config,
        std::unique_ptr<LogSink> &&sink);

    /**
     * Create an asynchronous logger with the provided log sink.
     *
     * @param name Name of the logger to create.
     * @param task_runner The sequence on which logs are streamed.
     * @param logger_config Reference to the logger configuration.
     * @param sink The log sink to receive log points for streaming.
     *
     * @return The created logger, or null if the logger could not be initialized.
     */
    static std::shared_ptr<Logger> create_logger(
        const std::string &name,
        const std::shared_ptr<SequencedTaskRunner> &task_runner,
        const std::shared_ptr<LoggerConfig> &logger_config,
        std::unique_ptr<LogSink> &&sink);

    /**
     * Create a synchronous file logger.
     *
     * @param name Name of the logger to create.
     * @param logger_config Reference to the logger configuration.
     * @param coder_config Reference to the coder configuration.
     * @param logger_directory Path to store log files.
     *
     * @return The created logger, or null if the logger could not be initialized.
     */
    static std::shared_ptr<Logger> create_file_logger(
        const std::string &name,
        const std::shared_ptr<LoggerConfig> &logger_config,
        const std::shared_ptr<CoderConfig> &coder_config,
        const std::filesystem::path &logger_directory);

    /**
     * Create an asynchronous file logger.
     *
     * @param name Name of the logger to create.
     * @param task_runner The sequence on which logs are streamed.
     * @param logger_config Reference to the logger configuration.
     * @param coder_config Reference to the coder configuration.
     * @param logger_directory Path to store log files.
     *
     * @return The created logger, or null if the logger could not be initialized.
     */
    static std::shared_ptr<Logger> create_file_logger(
        const std::string &name,
        const std::shared_ptr<SequencedTaskRunner> &task_runner,
        const std::shared_ptr<LoggerConfig> &logger_config,
        const std::shared_ptr<CoderConfig> &coder_config,
        const std::filesystem::path &logger_directory);

    /**
     * Create a synchronous console logger.
     *
     * @param name Name of the logger to create.
     * @param logger_config Reference to the logger configuration.
     *
     * @return The created logger, or null if the logger could not be initialized.
     */
    static std::shared_ptr<Logger> create_console_logger(
        const std::string &name,
        const std::shared_ptr<LoggerConfig> &logger_config);

    /**
     * Create an asynchronous console logger.
     *
     * @param name Name of the logger to create.
     * @param task_runner The sequence on which logs are streamed.
     * @param logger_config Reference to the logger configuration.
     *
     * @return The created logger, or null if the logger could not be initialized.
     */
    static std::shared_ptr<Logger> create_console_logger(
        const std::string &name,
        const std::shared_ptr<SequencedTaskRunner> &task_runner,
        const std::shared_ptr<LoggerConfig> &logger_config);

    /**
     * Set the default logger instance for the LOG* macro functions. If the provided logger is null,
     * the default logger is reset to the initial synchronous console logger.
     *
     * The default logger is retained until it is replaced or reset.
     *
     * Warning: Setting the default logger is not thread-safe. Do not set the default logger on one
     * thread while invoking a LOG* macro on another thread. The default logger should be set once
     * during initialization.
     *
     * @param logger The logger instance.
     */
    static void set_default_logger(const std::shared_ptr<Logger> &default_logger);

    /**
     * @return The default logger instance for the LOG* macro functions.
     */
    static Logger *get_default_logger();

    /**
     * Retrieve a logger by name. If the logger is not found, or if the logger instance has been
     * deleted, returns null.
     *
     * @param name Name of the logger to retrieve.
     *
     * @return The logger instance, or null.
     */
    static std::shared_ptr<Logger> get(const std::string &name);

    /**
     * @return This logger's name.
     */
    const std::string &name() const;

    /**
     * Add a debug log point to the logger.
     *
     * @tparam Args Variadic template arguments.
     *
     * @param format The format string for the log point.
     * @param args The variadic list of arguments to augment the format string with.
     */
    template <typename... Args>
    void debug(const char *format, const Args &... args)
    {
        log(Log::Level::Debug, {}, String::format(format, args...));
    }

    /**
     * Add a debug log point to the logger with trace information.
     *
     * @tparam Args Variadic template arguments.
     *
     * @param trace The trace information for the log point.
     * @param format The format string for the log point.
     * @param args The variadic list of arguments to augment the format string with.
     */
    template <typename... Args>
    void debug(Log::Trace &&trace, const char *format, const Args &... args)
    {
        log(Log::Level::Debug, std::move(trace), String::format(format, args...));
    }

    /**
     * Add an informational log point to the logger.
     *
     * @tparam Args Variadic template arguments.
     *
     * @param format The format string for the log point.
     * @param args The variadic list of arguments to augment the format string with.
     */
    template <typename... Args>
    void info(const char *format, const Args &... args)
    {
        log(Log::Level::Info, {}, String::format(format, args...));
    }

    /**
     * Add an informational log point to the logger with trace information.
     *
     * @tparam Args Variadic template arguments.
     *
     * @param trace The trace information for the log point.
     * @param format The format string for the log point.
     * @param args The variadic list of arguments to augment the format string with.
     */
    template <typename... Args>
    void info(Log::Trace &&trace, const char *format, const Args &... args)
    {
        log(Log::Level::Info, std::move(trace), String::format(format, args...));
    }

    /**
     * Add a warning log point to the logger.
     *
     * @tparam Args Variadic template arguments.
     *
     * @param format The format string for the log point.
     * @param args The variadic list of arguments to augment the format string with.
     */
    template <typename... Args>
    void warn(const char *format, const Args &... args)
    {
        log(Log::Level::Warn, {}, String::format(format, args...));
    }

    /**
     * Add a warning log point to the logger with trace information.
     *
     * @tparam Args Variadic template arguments.
     *
     * @param trace The trace information for the log point.
     * @param format The format string for the log point.
     * @param args The variadic list of arguments to augment the format string with.
     */
    template <typename... Args>
    void warn(Log::Trace &&trace, const char *format, const Args &... args)
    {
        log(Log::Level::Warn, std::move(trace), String::format(format, args...));
    }

    /**
     * Add an error log point to the logger.
     *
     * @tparam Args Variadic template arguments.
     *
     * @param format The format string for the log point.
     * @param args The variadic list of arguments to augment the format string with.
     */
    template <typename... Args>
    void error(const char *format, const Args &... args)
    {
        log(Log::Level::Error, {}, String::format(format, args...));
    }

    /**
     * Add an error log point to the logger with trace information.
     *
     * @tparam Args Variadic template arguments.
     *
     * @param trace The trace information for the log point.
     * @param format The format string for the log point.
     * @param args The variadic list of arguments to augment the format string with.
     */
    template <typename... Args>
    void error(Log::Trace &&trace, const char *format, const Args &... args)
    {
        log(Log::Level::Error, std::move(trace), String::format(format, args...));
    }

private:
    friend class detail::Registry;

    /**
     * Constructor. Creates a synchronous or an asynchronous logger, depending on whether the given
     * task runner is null.
     *
     * @param name Name of the logger.
     * @param task_runner If not null, the sequence on which logs are streamed.
     * @param config Reference to the logger configuration.
     * @param sink The log sink to receive log points for streaming.
     */
    Logger(
        const std::string &name,
        const std::shared_ptr<SequencedTaskRunner> &task_runner,
        const std::shared_ptr<LoggerConfig> &config,
        std::unique_ptr<LogSink> &&sink) noexcept;

    /**
     * Initialize the log sink.
     *
     * @return True if the logger could be initialized.
     */
    bool initialize();

    /**
     * Add a log point to the logger, optionally with trace information.
     *
     * Synchronous loggers will forward the log to the log sink immediately. Asynchronous loggers
     * will post a task to forward the log later.
     *
     * @param level The level of the log point.
     * @param trace The trace information for the log point.
     * @param message The message to log.
     */
    void log(Log::Level level, Log::Trace &&trace, std::string &&message);

    /**
     * Forward a log point to the log sink.
     *
     * @param level The level of the log point.
     * @param trace The trace information for the log point.
     * @param message The message to log.
     * @param time The time the log point was made.
     */
    void log_to_sink(
        Log::Level level,
        Log::Trace &&trace,
        std::string &&message,
        std::chrono::high_resolution_clock::time_point time);

    const std::string m_name;

    std::shared_ptr<LoggerConfig> m_config;
    std::unique_ptr<LogSink> m_sink;

    std::shared_ptr<SequencedTaskRunner> m_task_runner;
    std::atomic_bool m_last_task_failed {true};

    const std::chrono::high_resolution_clock::time_point m_start_time;
    std::uintmax_t m_index {0};
};

} // namespace fly

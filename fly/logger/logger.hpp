#pragma once

#include "fly/fly.hpp"
#include "fly/logger/detail/logger_macros.hpp"
#include "fly/logger/log.hpp"
#include "fly/system/system.hpp"
#include "fly/task/task.hpp"
#include "fly/types/concurrency/concurrent_queue.hpp"
#include "fly/types/string/string.hpp"

#include <chrono>
#include <cstdarg>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>

//==================================================================================================
#define LOGD(...)                                                                                  \
    _FLY_LOG(                                                                                      \
        fly::Log::Level::Debug,                                                                    \
        fly::String::format(_FLY_FORMAT_STRING(__VA_ARGS__) _FLY_FORMAT_ARGS(__VA_ARGS__)))

//==================================================================================================
#define LOGI(...)                                                                                  \
    _FLY_LOG(                                                                                      \
        fly::Log::Level::Info,                                                                     \
        fly::String::format(_FLY_FORMAT_STRING(__VA_ARGS__) _FLY_FORMAT_ARGS(__VA_ARGS__)))

//==================================================================================================
#define LOGW(...)                                                                                  \
    _FLY_LOG(                                                                                      \
        fly::Log::Level::Warn,                                                                     \
        fly::String::format(_FLY_FORMAT_STRING(__VA_ARGS__) _FLY_FORMAT_ARGS(__VA_ARGS__)))

//==================================================================================================
#define LOGS(...)                                                                                  \
    _FLY_LOG(                                                                                      \
        fly::Log::Level::Warn,                                                                     \
        fly::String::format(                                                                       \
            _FLY_FORMAT_STRING(__VA_ARGS__) ": %s" _FLY_FORMAT_ARGS(__VA_ARGS__),                  \
            fly::System::get_error_string()))

//==================================================================================================
#define LOGE(...)                                                                                  \
    _FLY_LOG(                                                                                      \
        fly::Log::Level::Error,                                                                    \
        fly::String::format(_FLY_FORMAT_STRING(__VA_ARGS__) _FLY_FORMAT_ARGS(__VA_ARGS__)))

//==================================================================================================
#define LOGC(...)                                                                                  \
    fly::Logger::console_log(                                                                      \
        true,                                                                                      \
        fly::String::format(_FLY_FORMAT_STRING(__VA_ARGS__) _FLY_FORMAT_ARGS(__VA_ARGS__)))

//==================================================================================================
#define LOGC_NO_LOCK(...)                                                                          \
    fly::Logger::console_log(                                                                      \
        false,                                                                                     \
        fly::String::format(_FLY_FORMAT_STRING(__VA_ARGS__) _FLY_FORMAT_ARGS(__VA_ARGS__)))

namespace fly {

class LoggerConfig;
class LoggerTask;
class SequencedTaskRunner;

/**
 * Provides thread safe instrumentation. There are 4 levels of instrumentation:
 *
 * 1. Debug = Really common points.
 * 2. Info = Less common, event based points.
 * 3. Warning = Something went wrong, but the system is OK.
 * 4. Error = Something went wrong, and the sytem is not OK.
 *
 * The following macros should be used to add points to the log: LOGD, LOGI, LOGW, LOGE. Usage is as
 * follows:
 *
 *   LOGD(message, message arguments, ...)
 *   For example, LOGD("This is message number %d", 10)
 *
 * The LOGS macro is provided for system error logging. It produces a warning-level log point with
 * the last system error appended to the given message.
 *
 * The LOGC macro is provided for thread-safe console logging. LOGC_NO_LOCK is also provided for
 * console logging without acquiring the console lock while inside, e.g., a signal handler.
 *
 * The logging macros support up to and including 50 format arguments. If more are needed, invoke
 * Logger::AddLog directly.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 21, 2016
 */
class Logger : public std::enable_shared_from_this<Logger>
{
    friend class LoggerTask;

public:
    /**
     * Constructor.
     *
     * @param task_runner Task runner for posting logger-related tasks onto.
     * @param config Reference to logger configuration.
     * @param logger_directory Path to store the log file.
     */
    Logger(
        const std::shared_ptr<SequencedTaskRunner> &task_runner,
        const std::shared_ptr<LoggerConfig> &config,
        const std::filesystem::path &logger_directory) noexcept;

    /**
     * Set the logger instance so that the LOG* macros function.
     *
     * @param logger The logger instance.
     */
    static void set_instance(const std::shared_ptr<Logger> &logger);

    /**
     * Log to the console in a thread-safe manner.
     *
     * @param acquire_lock Whether to acquire lock before logging.
     * @param message The message to log.
     */
    static void console_log(bool acquire_lock, const std::string &message);

    /**
     * Add a log to the static logger instance.
     *
     * @param level The level (debug, info, etc.) of the log.
     * @param file Name of the file storing the log.
     * @param function Name of the function storing the log.
     * @param line The line number the log point occurs.
     * @param message The message to log.
     */
    static void add_log(
        Log::Level level,
        const char *file,
        const char *function,
        std::uint32_t line,
        const std::string &message);

    /**
     * Create the logger's log file on disk and initialize the logger task.
     *
     * @return True if the logger is in a valid state.
     */
    bool start();

    /**
     * @return Path to the current log file.
     */
    std::filesystem::path get_log_file_path() const;

private:
    /**
     * Perform any IO operations. Wait for a log item to be available and write it to disk.
     *
     * @return True if the current log file is still open and healthy.
     */
    bool poll();

    /**
     * Add a log to this logger instance.
     *
     * @param level The level (debug, info, etc.) of the log.
     * @param file Name of the file storing the log.
     * @param function Name of the function storing the log.
     * @param line The line number the log point occurs.
     * @param message The message to log.
     */
    void add_log_internal(
        Log::Level level,
        const char *file,
        const char *function,
        std::uint32_t line,
        const std::string &message);

    /**
     * Create the log file. If a log file is already open, close it.
     *
     * @return True if the log file could be opened.
     */
    bool create_log_file();

    static std::weak_ptr<Logger> s_weak_instance;
    static std::mutex s_console_mutex;

    fly::ConcurrentQueue<Log> m_log_queue;

    std::shared_ptr<SequencedTaskRunner> m_task_runner;
    std::shared_ptr<Task> m_task;

    std::shared_ptr<LoggerConfig> m_config;

    const std::filesystem::path m_log_directory;
    std::filesystem::path m_log_file;
    std::ofstream m_log_stream;

    std::uintmax_t m_index;

    const std::chrono::high_resolution_clock::time_point m_start_time;
};

/**
 * Task to be executed to check for new log entries.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version August 12, 2018
 */
class LoggerTask : public Task
{
public:
    explicit LoggerTask(std::weak_ptr<Logger> weak_logger) noexcept;

protected:
    /**
     * Call back into the logger to check for new log entries. The task re-arms itself.
     */
    void run() override;

private:
    std::weak_ptr<Logger> m_weak_logger;
};

} // namespace fly

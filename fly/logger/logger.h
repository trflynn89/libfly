#pragma once

#include "fly/fly.h"
#include "fly/logger/detail/logger_macros.h"
#include "fly/logger/log.h"
#include "fly/system/system.h"
#include "fly/task/task.h"
#include "fly/types/concurrency/concurrent_queue.h"
#include "fly/types/string/string.h"

#include <chrono>
#include <cstdarg>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>

// The private macros used internally insert commas only if one is needed, which
// the formatter doesn't handle.
// clang-format off

//==============================================================================
#define LOGD(...)                                                              \
    _FLY_LOG(                                                                  \
        fly::Log::Level::Debug,                                                \
        fly::String::Format(                                                   \
            _FLY_FORMAT_STRING(__VA_ARGS__)                                    \
            _FLY_FORMAT_ARGS(__VA_ARGS__)))

//==============================================================================
#define LOGI(...)                                                              \
    _FLY_LOG(                                                                  \
        fly::Log::Level::Info,                                                 \
        fly::String::Format(                                                   \
            _FLY_FORMAT_STRING(__VA_ARGS__)                                    \
            _FLY_FORMAT_ARGS(__VA_ARGS__)))

//==============================================================================
#define LOGW(...)                                                              \
    _FLY_LOG(                                                                  \
        fly::Log::Level::Warn,                                                 \
        fly::String::Format(                                                   \
            _FLY_FORMAT_STRING(__VA_ARGS__)                                    \
            _FLY_FORMAT_ARGS(__VA_ARGS__)))

//==============================================================================
#define LOGS(...)                                                              \
    _FLY_LOG(                                                                  \
        fly::Log::Level::Warn,                                                 \
        fly::String::Format(                                                   \
            _FLY_FORMAT_STRING(__VA_ARGS__) ": %s"                             \
            _FLY_FORMAT_ARGS(__VA_ARGS__),                                     \
            fly::System::GetErrorString()))

//==============================================================================
#define LOGE(...)                                                              \
    _FLY_LOG(                                                                  \
        fly::Log::Level::Error,                                                \
        fly::String::Format(                                                   \
            _FLY_FORMAT_STRING(__VA_ARGS__)                                    \
            _FLY_FORMAT_ARGS(__VA_ARGS__)))

//==============================================================================
#define LOGC(...)                                                              \
    fly::Logger::ConsoleLog(                                                   \
        true,                                                                  \
        fly::String::Format(                                                   \
            _FLY_FORMAT_STRING(__VA_ARGS__)                                    \
            _FLY_FORMAT_ARGS(__VA_ARGS__)))

//==============================================================================
#define LOGC_NO_LOCK(...)                                                      \
    fly::Logger::ConsoleLog(                                                   \
        false,                                                                 \
        fly::String::Format(                                                   \
            _FLY_FORMAT_STRING(__VA_ARGS__)                                    \
            _FLY_FORMAT_ARGS(__VA_ARGS__)))

// clang-format on

namespace fly {

class LoggerConfig;
class LoggerTask;
class SequencedTaskRunner;

/**
 * Provides thread safe instrumentation. There are 4 levels of instrumentation:
 * 1. Debug = Really common points.
 * 2. Info = Less common, event based points.
 * 3. Warning = Something went wrong, but the system is OK.
 * 4. Error = Something went wrong, and the sytem is not OK.
 *
 * The following macros should be used to add points to the log: LOGD, LOGI,
 * LOGW, LOGE. Usage is as follows:
 *
 *   LOGD(message, message arguments)
 *   For example, LOGD(1, "This is message number %d", 10)
 *
 * THE LOGS macro is provided for system error logging. It produces a warning-
 * level log point with the last system error appended to the given message.
 *
 * The LOGC macro is also provided for thread-safe console logging. LOGC_NO_LOCK
 * is also provided for console logging without acquiring the console lock while
 * inside, e.g., a signal handler.
 *
 * The logging macros support up to and including 50 format arguments. If more
 * are needed, invoke Logger::AddLog directly.
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
     * @param TaskRunner Task runner for posting logger-related tasks onto.
     * @param LoggerConfig Reference to logger configuration.
     * @param path Path to store the log file.
     */
    Logger(
        const std::shared_ptr<SequencedTaskRunner> &,
        const std::shared_ptr<LoggerConfig> &,
        const std::filesystem::path &) noexcept;

    /**
     * Set the logger instance so that the LOG* macros function.
     *
     * @param Logger The logger instance.
     */
    static void SetInstance(const std::shared_ptr<Logger> &) noexcept;

    /**
     * @return The logger instance.
     */
    static std::shared_ptr<Logger> GetInstance() noexcept;

    /**
     * Log to the console in a thread-safe manner.
     *
     * @param bool Whether to acquire lock before logging.
     * @param string The message to log.
     */
    static void ConsoleLog(bool, const std::string &) noexcept;

    /**
     * Add a log to the static logger instance.
     *
     * @param Level The level (debug, info, etc.) of the log.
     * @param const char * Name of the file storing the log.
     * @param const char * Name of the function storing the log.
     * @param unsigned int The line number the log point occurs.
     * @param string The message to log.
     */
    static void AddLog(
        Log::Level,
        const char *,
        const char *,
        unsigned int,
        const std::string &) noexcept;

    /**
     * Create the logger's log file on disk and initialize the logger task.
     *
     * @return bool True if the logger is in a valid state.
     */
    bool Start() noexcept;

    /**
     * @return path Path to the current log file.
     */
    std::filesystem::path GetLogFilePath() const noexcept;

private:
    /**
     * Perform any IO operations. Wait for a log item to be available and write
     * it to disk.
     *
     * @return bool True if the current log file is still open and healthy.
     */
    bool poll() noexcept;

    /**
     * Add a log to this logger instance.
     *
     * @param Level The level (debug, info, etc.) of the log.
     * @param const char * Name of the file storing the log.
     * @param const char * Name of the function storing the log.
     * @param unsigned int The line number the log point occurs.
     * @param string The message to log.
     */
    void addLog(
        Log::Level,
        const char *,
        const char *,
        unsigned int,
        const std::string &) noexcept;

    /**
     * Create the log file. If a log file is already open, close it.
     *
     * @return True if the log file could be opened.
     */
    bool createLogFile() noexcept;

    static std::weak_ptr<Logger> s_wpInstance;
    static std::mutex s_consoleMutex;

    fly::ConcurrentQueue<Log> m_logQueue;

    std::shared_ptr<SequencedTaskRunner> m_spTaskRunner;
    std::shared_ptr<Task> m_spTask;

    std::shared_ptr<LoggerConfig> m_spConfig;

    const std::filesystem::path m_logDirectory;
    std::filesystem::path m_logFile;
    std::ofstream m_logStream;

    std::uintmax_t m_index;

    const std::chrono::high_resolution_clock::time_point m_startTime;
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
    LoggerTask(std::weak_ptr<Logger>) noexcept;

protected:
    /**
     * Call back into the logger to check for new log entries. The task re-arms
     * itself.
     */
    void Run() noexcept override;

private:
    std::weak_ptr<Logger> m_wpLogger;
};

} // namespace fly

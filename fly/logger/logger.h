#pragma once

#include <chrono>
#include <cstdarg>
#include <fstream>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "fly/fly.h"
#include "fly/logger/log.h"
#include "fly/system/system.h"
#include "fly/task/task.h"
#include "fly/types/concurrent_queue.h"
#include "fly/types/string.h"

//==============================================================================
#define LOG(lvl, gameId, fmt)                                                  \
(                                                                              \
    fly::Logger::AddLog(lvl, gameId, __FILE__, __FUNCTION__, __LINE__, fmt)    \
)

//==============================================================================
#define LOGD(gameId, fmt, ...)                                                 \
(                                                                              \
    LOG(fly::Log::Level::Debug, gameId,                                        \
        fly::String::Format(fmt, ##__VA_ARGS__))                               \
)

//==============================================================================
#define LOGI(gameId, fmt, ...)                                                 \
(                                                                              \
    LOG(fly::Log::Level::Info, gameId,                                         \
        fly::String::Format(fmt, ##__VA_ARGS__))                               \
)

//==============================================================================
#define LOGW(gameId, fmt, ...)                                                 \
(                                                                              \
    LOG(fly::Log::Level::Warn, gameId,                                         \
        fly::String::Format(fmt, ##__VA_ARGS__))                               \
)

//==============================================================================
#define LOGS(gameId, fmt, ...)                                                 \
(                                                                              \
    LOG(fly::Log::Level::Warn, gameId,                                         \
        fly::String::Format(fmt ": ", ##__VA_ARGS__) +                         \
            fly::System::GetErrorString())                                     \
)

//==============================================================================
#define LOGE(gameId, fmt, ...)                                                 \
(                                                                              \
    LOG(fly::Log::Level::Error, gameId,                                        \
        fly::String::Format(fmt, ##__VA_ARGS__))                               \
)

//==============================================================================
#define LOGC(fmt, ...)                                                         \
(                                                                              \
    fly::Logger::ConsoleLog(true, fly::String::Format(fmt, ##__VA_ARGS__))     \
)

//==============================================================================
#define LOGC_NO_LOCK(fmt, ...)                                                 \
(                                                                              \
    fly::Logger::ConsoleLog(false, fly::String::Format(fmt, ##__VA_ARGS__))    \
)

namespace fly {

FLY_CLASS_PTRS(Logger);
FLY_CLASS_PTRS(LoggerTask);

FLY_CLASS_PTRS(SequencedTaskRunner);
FLY_CLASS_PTRS(LoggerConfig);

/**
 * Provides thread safe instrumentation. There are 4 levels of instrumentation:
 * 1. Debug = Really common points.
 * 2. Info = Less common, event based point (e.g. client connection, game over)
 * 3. Warning = Something went wrong, but the system is OK.
 * 4. Error = Something went wrong, and the sytem is not OK.
 *
 * The following macros should be used to add points to the log: LOGD, LOGI,
 * LOGW, LOGE. Usage is as follows:
 *
 *   LOGD(game ID, message, message arguments)
 *   For example, LOGD(1, "This is message number %d", 10)
 *
 * The LOGC macro is also provided for thread-safe console logging. LOGC_NO_LOCK
 * is also provided for console logging without acquiring the console lock while
 * inside, e.g., a signal handler.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 21, 2016
 */
class Logger : public std::enable_shared_from_this<Logger>
{
    friend class LoggerTask;

public:
    /**
     * Constructor.
     *
     * @param TaskRunnerPtr Task runner for posting logger-related tasks onto.
     * @param LoggerConfigPtr Reference to logger configuration.
     * @param string Path to store the log file.
     */
    Logger(const SequencedTaskRunnerPtr &, const LoggerConfigPtr &, const std::string &);

    /**
     * Set the logger instance so that the LOG* macros function.
     *
     * @param LoggerPtr The logger instance.
     */
    static void SetInstance(const LoggerPtr &);

    /**
     * @return The logger instance.
     */
    static LoggerPtr GetInstance();

    /**
     * Log to the console in a thread-safe manner.
     *
     * @param bool Whether to acquire lock before logging.
     * @param string The message to log.
     */
    static void ConsoleLog(bool, const std::string &);

    /**
     * Add a log to the static logger instance.
     *
     * @param Level The level (debug, info, etc.) of this log.
     * @param ssize_t The ID of the game storing this entry.
     * @param const char * Name of the file storing this log.
     * @param const char * Name of the function storing this log.
     * @param unsigned int The line number this log point occurs.
     * @param string The message to log.
     */
    static void AddLog(Log::Level, ssize_t, const char *, const char *, unsigned int, const std::string &);

    /**
     * Create the logger's log file on disk and initialize the logger task.
     *
     * @return bool True if the logger is in a valid state.
     */
    bool Start();

    /**
     * @return string Path to the current log file.
     */
    std::string GetLogFilePath() const;

private:
    /**
     * Perform any IO operations. Wait for a log item to be available and write
     * it to disk.
     *
     * @return bool True if the current log file is still open and healthy.
     */
    bool poll();

    /**
     * Add a log to this logger instance.
     *
     * @param Level The level (debug, info, etc.) of this log.
     * @param ssize_t The ID of the game storing this entry.
     * @param const char * Name of the file storing this log.
     * @param const char * Name of the function storing this log.
     * @param unsigned int The line number this log point occurs.
     * @param string The message to log.
     */
    void addLog(Log::Level, ssize_t, const char *, const char *, unsigned int, const std::string &);

    /**
     * Create the log file. If a log file is already open, close it.
     *
     * @return True if the log file could be opened.
     */
    bool createLogFile();

    static LoggerWPtr s_wpInstance;
    static std::mutex s_consoleMutex;

    std::ofstream m_logFile;
    fly::ConcurrentQueue<Log> m_logQueue;
    std::future<void> m_future;

    SequencedTaskRunnerPtr m_spTaskRunner;
    TaskPtr m_spTask;

    LoggerConfigPtr m_spConfig;

    const std::string m_filePath;
    std::string m_fileName;
    size_t m_fileSize;

    size_t m_index;

    const std::chrono::high_resolution_clock::time_point m_startTime;
};

/**
 * Task to be executed to check for new log entries.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version August 12, 2018
 */
class LoggerTask : public Task
{
public:
    LoggerTask(const LoggerWPtr &);

protected:
    /**
     * Call back into the logger to check for new log entries. The task re-arms
     * itself.
     */
    void Run() override;

private:
    LoggerWPtr m_wpLogger;
};

}

#pragma once

#include <chrono>
#include <cstdarg>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "fly/fly.h"
#include "fly/concurrency/concurrent_queue.h"
#include "fly/logger/log.h"
#include "fly/string/string.h"
#include "fly/task/runner.h"

//==============================================================================
#define LOG(lvl, gameId, fmt)                                                 \
(                                                                             \
    fly::Logger::AddLog(lvl, gameId, __FILE__, __FUNCTION__, __LINE__, fmt)  \
)

//==============================================================================
#define LOGD(gameId, fmt, ...)                                                \
(                                                                             \
    LOG(LOG_DEBUG, gameId, fly::String::Format(fmt, ##__VA_ARGS__))          \
)

//==============================================================================
#define LOGI(gameId, fmt, ...)                                                \
(                                                                             \
    LOG(LOG_INFO, gameId, fly::String::Format(fmt, ##__VA_ARGS__))           \
)

//==============================================================================
#define LOGW(gameId, fmt, ...)                                                \
(                                                                             \
    LOG(LOG_WARN, gameId, fly::String::Format(fmt, ##__VA_ARGS__))           \
)

//==============================================================================
#define LOGE(gameId, fmt, ...)                                                \
(                                                                             \
    LOG(LOG_ERROR, gameId, fly::String::Format(fmt, ##__VA_ARGS__))          \
)

//==============================================================================
#define LOGC(fmt, ...)                                                        \
(                                                                             \
    fly::Logger::ConsoleLog(true, fly::String::Format(fmt, ##__VA_ARGS__))  \
)

//==============================================================================
#define LOGC_NO_LOCK(fmt, ...)                                                \
(                                                                             \
    fly::Logger::ConsoleLog(false, fly::String::Format(fmt, ##__VA_ARGS__)) \
)

namespace fly {

DEFINE_CLASS_PTRS(ConfigManager);
DEFINE_CLASS_PTRS(Logger);
DEFINE_CLASS_PTRS(LoggerConfig);

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
class Logger : public Runner
{
public:
    /**
     * Default constructor. Constructs default loger configuration, meant for
     * unit tests.
     *
     * @param string Path to store the log file.
     */
    Logger(const std::string &);

    /**
     * Constructor.
     *
     * @param ConfigManagerPtr Reference to the configuration manager.
     * @param string Path to store the log file.
     */
    Logger(ConfigManagerPtr &, const std::string &);

    /**
     * Destructor.
     */
    virtual ~Logger();

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
     * @param LogLevel The level (debug, info, etc.) of this log.
     * @param ssize_t The ID of the game storing this entry.
     * @param const char * Name of the file storing this log.
     * @param const char * Name of the function storing this log.
     * @param unsigned int The line number this log point occurs.
     * @param string The message to log.
     */
    static void AddLog(LogLevel, ssize_t, const char *, const char *, unsigned int, const std::string &);

    /**
     * @return string Path to the current log file.
     */
    std::string GetLogFilePath() const;

    /**
     * @return LoggerConfigPtr Shared pointer to the log config.
     */
    LoggerConfigPtr GetLogConfig() const;

protected:
    /**
     * Start the logger. Create the logger's log file on disk.
     *
     * @return True if the log file could be created.
     */
    virtual bool StartRunner();

    /**
     * Stop the logger.
     */
    virtual void StopRunner();

    /**
     * Perform any IO operations. Wait for a log item to be available and write
     * it to disk.
     */
    virtual bool DoWork();

private:
    /**
     * Add a log to this logger instance.
     *
     * @param LogLevel The level (debug, info, etc.) of this log.
     * @param ssize_t The ID of the game storing this entry.
     * @param const char * Name of the file storing this log.
     * @param const char * Name of the function storing this log.
     * @param unsigned int The line number this log point occurs.
     * @param string The message to log.
     */
    void addLog(LogLevel, ssize_t, const char *, const char *, unsigned int, const std::string &);

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

    LoggerConfigPtr m_spConfig;

    const std::string m_filePath;
    std::string m_fileName;
    size_t m_fileSize;

    size_t m_index;

    const std::chrono::high_resolution_clock::time_point m_startTime;
};

}

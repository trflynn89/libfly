#include "fly/logger/logger.h"

#include "fly/logger/logger_config.h"
#include "fly/task/task_runner.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <system_error>

namespace fly {

//==============================================================================
std::weak_ptr<Logger> Logger::s_wpInstance;
std::mutex Logger::s_consoleMutex;

//==============================================================================
Logger::Logger(
    const std::shared_ptr<SequencedTaskRunner> &spTaskRunner,
    const std::shared_ptr<LoggerConfig> &spConfig,
    const std::filesystem::path &loggerDirectory) noexcept :
    m_spTaskRunner(spTaskRunner),
    m_spConfig(spConfig),
    m_logDirectory(loggerDirectory),
    m_index(0),
    m_startTime(std::chrono::high_resolution_clock::now())
{
}

//==============================================================================
void Logger::SetInstance(const std::shared_ptr<Logger> &spLogger) noexcept
{
    s_wpInstance = spLogger;
}

//==============================================================================
std::shared_ptr<Logger> Logger::GetInstance() noexcept
{
    return s_wpInstance.lock();
}

//==============================================================================
void Logger::ConsoleLog(bool acquireLock, const std::string &message) noexcept
{
    std::unique_lock<std::mutex> lock(s_consoleMutex, std::defer_lock);
    std::string timeStr = System::LocalTime();

    if (acquireLock)
    {
        lock.lock();
    }

    std::cout << timeStr << ": " << message << std::endl;
}

//==============================================================================
void Logger::AddLog(
    Log::Level level,
    const char *file,
    const char *func,
    unsigned int line,
    const std::string &message) noexcept
{
    std::shared_ptr<Logger> spLogger = GetInstance();

    if (spLogger)
    {
        spLogger->addLog(level, file, func, line, message);
    }
    else
    {
        std::string console =
            String::Format("%d %s:%s:%d %s", level, file, func, line, message);

        ConsoleLog(true, console);
    }
}

//==============================================================================
bool Logger::Start() noexcept
{
    if (createLogFile())
    {
        std::shared_ptr<Logger> spLogger = shared_from_this();

        m_spTask = std::make_shared<LoggerTask>(spLogger);
        m_spTaskRunner->PostTask(m_spTask);

        return true;
    }

    return false;
}

//==============================================================================
std::filesystem::path Logger::GetLogFilePath() const noexcept
{
    return m_logFile;
}

//==============================================================================
bool Logger::poll() noexcept
{
    Log log;

    if (m_logQueue.Pop(log, m_spConfig->QueueWaitTime()) && m_logStream.good())
    {
        const std::string logStr = String::Format("%u\t%s", m_index++, log);
        m_logStream << logStr << std::flush;

        std::error_code error;

        if (std::filesystem::file_size(m_logFile, error) >
            m_spConfig->MaxLogFileSize())
        {
            createLogFile();
        }
    }

    return m_logStream.good();
}

//==============================================================================
void Logger::addLog(
    Log::Level level,
    const char *file,
    const char *func,
    unsigned int line,
    const std::string &message) noexcept
{
    auto now = std::chrono::high_resolution_clock::now();

    auto logTime = std::chrono::duration_cast<std::chrono::duration<double>>(
        now - m_startTime);

    if ((level >= Log::Level::Debug) && (level < Log::Level::NumLevels))
    {
        Log log(m_spConfig, message);

        log.m_level = level;
        log.m_time = logTime.count();
        log.m_line = line;

        snprintf(log.m_file, sizeof(log.m_file), "%s", file);
        snprintf(log.m_function, sizeof(log.m_function), "%s", func);

        m_logQueue.Push(std::move(log));
    }
}

//==============================================================================
bool Logger::createLogFile() noexcept
{
    std::string randStr = String::GenerateRandomString(10);
    std::string timeStr = System::LocalTime();

    String::ReplaceAll(timeStr, ":", "-");
    String::ReplaceAll(timeStr, " ", "_");

    const std::string fileName =
        String::Format("Log_%s_%s.log", timeStr, randStr);
    m_logFile = m_logDirectory / fileName;

    if (m_logStream.is_open())
    {
        m_logStream.close();
    }

    LOGC("Creating logger file: %s", m_logFile);
    m_logStream.open(m_logFile, std::ios::out);

    return m_logStream.good();
}

//==============================================================================
LoggerTask::LoggerTask(std::weak_ptr<Logger> wpLogger) noexcept :
    Task(),
    m_wpLogger(wpLogger)
{
}

//==============================================================================
void LoggerTask::Run() noexcept
{
    std::shared_ptr<Logger> spLogger = m_wpLogger.lock();

    if (spLogger && spLogger->poll())
    {
        spLogger->m_spTaskRunner->PostTask(spLogger->m_spTask);
    }
}

} // namespace fly

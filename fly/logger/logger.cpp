#include "fly/logger/logger.h"

#include <algorithm>
#include <climits>
#include <cstdio>
#include <cstring>
#include <cwchar>

#include "fly/logger/logger_config.h"
#include "fly/path/path.h"
#include "fly/task/task_runner.h"

namespace fly {

//==============================================================================
LoggerWPtr Logger::s_wpInstance;
std::mutex Logger::s_consoleMutex;

//==============================================================================
Logger::Logger(
    const SequencedTaskRunnerPtr &spTaskRunner,
    const LoggerConfigPtr &spConfig,
    const std::string &filePath
):
    m_spTaskRunner(spTaskRunner),
    m_spConfig(spConfig),
    m_filePath(filePath),
    m_fileSize(0),
    m_index(0),
    m_startTime(std::chrono::high_resolution_clock::now())
{
}

//==============================================================================
void Logger::SetInstance(const LoggerPtr &spLogger)
{
    s_wpInstance = spLogger;
}

//==============================================================================
LoggerPtr Logger::GetInstance()
{
    return s_wpInstance.lock();
}

//==============================================================================
void Logger::ConsoleLog(bool acquireLock, const std::string &message)
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
void Logger::AddLog(Log::Level level, ssize_t gameId, const char *file,
    const char *func, unsigned int line, const std::string &message)
{
    LoggerPtr spLogger = GetInstance();

    if (spLogger)
    {
        spLogger->addLog(level, gameId, file, func, line, message);
    }
    else
    {
        std::string console = String::Format("%d %d %s:%s:%d %s",
            level, gameId, file, func, line, message);

        ConsoleLog(true, console);
    }
}

//==============================================================================
bool Logger::Start()
{
    if (createLogFile())
    {
        LoggerPtr spLogger = shared_from_this();

        m_spTask = std::make_shared<LoggerTask>(spLogger);
        m_spTaskRunner->PostTask(m_spTask);

        return true;
    }

    return false;
}

//==============================================================================
std::string Logger::GetLogFilePath() const
{
    return m_fileName;
}

//==============================================================================
bool Logger::poll()
{
    Log log;

    if (m_logQueue.Pop(log, m_spConfig->QueueWaitTime()) && m_logFile.good())
    {
        const std::string logStr = String::Format("%u\t%s", m_index++, log);

        m_logFile << logStr << std::flush;
        m_fileSize += logStr.size();

        if (m_fileSize > m_spConfig->MaxLogFileSize())
        {
            createLogFile();
        }
    }

    return m_logFile.good();
}

//==============================================================================
void Logger::addLog(Log::Level level, ssize_t gameId, const char *file,
    const char *func, unsigned int line, const std::string &message)
{
    auto now = std::chrono::high_resolution_clock::now();

    auto logTime = std::chrono::duration_cast<std::chrono::duration<double>>(
        now - m_startTime
    );

    if ((level >= Log::Level::Debug) && (level < Log::Level::NumLevels))
    {
        Log log(m_spConfig, message);

        log.m_level = level;
        log.m_time = logTime.count();
        log.m_gameId = gameId;
        log.m_line = line;

        snprintf(log.m_file, sizeof(log.m_file), "%s", file);
        snprintf(log.m_function, sizeof(log.m_function), "%s", func);

        m_logQueue.Push(log);
    }
}

//==============================================================================
bool Logger::createLogFile()
{
    std::string randStr = String::GenerateRandomString(10);
    std::string timeStr = System::LocalTime();

    String::ReplaceAll(timeStr, ":", "-");
    String::ReplaceAll(timeStr, " ", "_");

    std::string fileName = String::Format("Log_%s_%s.log", timeStr, randStr);
    m_fileName = Path::Join(m_filePath, fileName);

    if (m_logFile.is_open())
    {
        m_logFile.close();
    }

    LOGC("Creating logger file: %s", m_fileName);
    m_logFile.open(m_fileName, std::ios::out);
    m_fileSize = 0;

    return m_logFile.good();
}

//==============================================================================
LoggerTask::LoggerTask(const LoggerWPtr &wpLogger) :
    Task(),
    m_wpLogger(wpLogger)
{
}

//==============================================================================
void LoggerTask::Run()
{
    LoggerPtr spLogger = m_wpLogger.lock();

    if (spLogger && spLogger->poll())
    {
        spLogger->m_spTaskRunner->PostTask(spLogger->m_spTask);
    }
}

}

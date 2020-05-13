#include "fly/logger/logger.hpp"

#include "fly/logger/logger_config.hpp"
#include "fly/task/task_runner.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <system_error>

namespace fly {

//==============================================================================
std::weak_ptr<Logger> Logger::s_weak_instance;
std::mutex Logger::s_console_mutex;

//==============================================================================
Logger::Logger(
    const std::shared_ptr<SequencedTaskRunner> &task_runner,
    const std::shared_ptr<LoggerConfig> &config,
    const std::filesystem::path &logger_directory) noexcept :
    m_task_runner(task_runner),
    m_config(config),
    m_log_directory(logger_directory),
    m_index(0),
    m_start_time(std::chrono::high_resolution_clock::now())
{
}

//==============================================================================
void Logger::set_instance(const std::shared_ptr<Logger> &logger) noexcept
{
    s_weak_instance = logger;
}

//==============================================================================
void Logger::console_log(bool acquire_lock, const std::string &message) noexcept
{
    std::unique_lock<std::mutex> lock(s_console_mutex, std::defer_lock);
    const std::string time_str = System::local_time();

    if (acquire_lock)
    {
        lock.lock();
    }

    std::cout << time_str << ": " << message << std::endl;
}

//==============================================================================
void Logger::add_log(
    Log::Level level,
    const char *file,
    const char *function,
    std::uint32_t line,
    const std::string &message) noexcept
{
    std::shared_ptr<Logger> logger = s_weak_instance.lock();

    if (logger)
    {
        logger->add_log_internal(level, file, function, line, message);
    }
    else
    {
        const std::string console = String::format(
            "%d %s:%s:%d %s",
            level,
            file,
            function,
            line,
            message);

        console_log(true, console);
    }
}

//==============================================================================
bool Logger::start() noexcept
{
    if (create_log_file())
    {
        std::shared_ptr<Logger> logger = shared_from_this();

        m_task = std::make_shared<LoggerTask>(logger);
        m_task_runner->post_task(m_task);

        return true;
    }

    return false;
}

//==============================================================================
std::filesystem::path Logger::get_log_file_path() const noexcept
{
    return m_log_file;
}

//==============================================================================
bool Logger::poll() noexcept
{
    Log log;

    if (m_log_queue.pop(log, m_config->queue_wait_time()) &&
        m_log_stream.good())
    {
        String::format(m_log_stream, "%u\t%s", m_index++, log) << std::flush;
        std::error_code error;

        if (std::filesystem::file_size(m_log_file, error) >
            m_config->max_log_file_size())
        {
            create_log_file();
        }
    }

    return m_log_stream.good();
}

//==============================================================================
void Logger::add_log_internal(
    Log::Level level,
    const char *file,
    const char *function,
    std::uint32_t line,
    const std::string &message) noexcept
{
    auto now = std::chrono::high_resolution_clock::now();

    auto log_time = std::chrono::duration_cast<std::chrono::duration<double>>(
        now - m_start_time);

    if ((level >= Log::Level::Debug) && (level < Log::Level::NumLevels))
    {
        Log log(m_config, message);

        log.m_level = level;
        log.m_time = log_time.count();
        log.m_line = line;

        snprintf(log.m_file, sizeof(log.m_file), "%s", file);
        snprintf(log.m_function, sizeof(log.m_function), "%s", function);

        m_log_queue.push(std::move(log));
    }
}

//==============================================================================
bool Logger::create_log_file() noexcept
{
    const std::string rand_str = String::generate_random_string(10);
    std::string time_str = System::local_time();

    String::replace_all(time_str, ":", "-");
    String::replace_all(time_str, " ", "_");

    const std::string file_name =
        String::format("Log_%s_%s.log", time_str, rand_str);
    m_log_file = m_log_directory / file_name;

    if (m_log_stream.is_open())
    {
        m_log_stream.close();
    }

    LOGC("Creating logger file: %s", m_log_file);
    m_log_stream.open(m_log_file, std::ios::out);

    return m_log_stream.good();
}

//==============================================================================
LoggerTask::LoggerTask(std::weak_ptr<Logger> weak_logger) noexcept :
    Task(),
    m_weak_logger(weak_logger)
{
}

//==============================================================================
void LoggerTask::run() noexcept
{
    std::shared_ptr<Logger> logger = m_weak_logger.lock();

    if (logger && logger->poll())
    {
        logger->m_task_runner->post_task(logger->m_task);
    }
}

} // namespace fly

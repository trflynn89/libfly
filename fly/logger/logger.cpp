#include "fly/logger/logger.hpp"

#include "fly/coders/coder_config.hpp"
#include "fly/coders/huffman/huffman_encoder.hpp"
#include "fly/logger/logger_config.hpp"
#include "fly/task/task_runner.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <system_error>

namespace fly {

//==================================================================================================
std::weak_ptr<Logger> Logger::s_weak_instance;
std::mutex Logger::s_console_mutex;

//==================================================================================================
Logger::Logger(
    const std::shared_ptr<SequencedTaskRunner> &task_runner,
    const std::shared_ptr<LoggerConfig> &logger_config,
    const std::shared_ptr<CoderConfig> &coder_config,
    const std::filesystem::path &logger_directory) noexcept :
    m_task_runner(task_runner),
    m_logger_config(logger_config),
    m_coder_config(coder_config),
    m_log_directory(logger_directory),
    m_index(0),
    m_start_time(std::chrono::high_resolution_clock::now())
{
}

//==================================================================================================
void Logger::set_instance(const std::shared_ptr<Logger> &logger)
{
    s_weak_instance = logger;
}

//==================================================================================================
void Logger::console_log(bool acquire_lock, std::string &&message)
{
    std::unique_lock<std::mutex> lock(s_console_mutex, std::defer_lock);
    const std::string time_str = System::local_time();

    if (acquire_lock)
    {
        lock.lock();
    }

    std::cout << time_str << ": " << message << std::endl;
}

//==================================================================================================
void Logger::add_log(
    Log::Level level,
    const char *file,
    const char *function,
    std::uint32_t line,
    std::string &&message)
{
    std::shared_ptr<Logger> logger = s_weak_instance.lock();

    if (logger)
    {
        logger->add_log_internal(level, file, function, line, std::move(message));
    }
    else
    {
        console_log(
            true,
            String::format("%d %s:%s:%d %s", level, file, function, line, std::move(message)));
    }
}

//==================================================================================================
bool Logger::start()
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

//==================================================================================================
std::filesystem::path Logger::get_log_file_path() const
{
    std::unique_lock<std::mutex> lock(m_log_file_mutex);
    return m_log_file;
}

//==================================================================================================
bool Logger::poll()
{
    Log log;

    if (m_log_queue.pop(log, m_logger_config->queue_wait_time()) && m_log_stream.good())
    {
        m_log_stream << log << std::flush;
        std::error_code error;

        std::unique_lock<std::mutex> lock(m_log_file_mutex);

        if (std::filesystem::file_size(m_log_file, error) > m_logger_config->max_log_file_size())
        {
            create_log_file();
        }
    }

    return m_log_stream.good();
}

//==================================================================================================
void Logger::add_log_internal(
    Log::Level level,
    const char *file,
    const char *function,
    std::uint32_t line,
    std::string &&message)
{
    auto now = std::chrono::high_resolution_clock::now();
    auto log_time = std::chrono::duration_cast<std::chrono::duration<double>>(now - m_start_time);

    if ((level >= Log::Level::Debug) && (level < Log::Level::NumLevels))
    {
        Log log(m_logger_config, std::move(message));

        log.m_index = m_index++;
        log.m_level = level;
        log.m_time = log_time.count();
        log.m_line = line;

        snprintf(log.m_file, sizeof(log.m_file), "%s", file);
        snprintf(log.m_function, sizeof(log.m_function), "%s", function);

        m_log_queue.push(std::move(log));
    }
}

//==================================================================================================
bool Logger::create_log_file()
{
    if (m_log_stream.is_open())
    {
        m_log_stream.close();

        if (m_logger_config->compress_log_files())
        {
            std::filesystem::path compressed_log_file = m_log_file;
            compressed_log_file.replace_extension(".log.enc");

            HuffmanEncoder encoder(m_coder_config);

            if (encoder.encode_file(m_log_file, compressed_log_file))
            {
                LOGC("Log file compressed to: %s", compressed_log_file);
                std::filesystem::remove(m_log_file);
            }
            else
            {
                LOGC("Failed to compress: %s", m_log_file);
            }
        }
    }

    const std::string rand_str = String::generate_random_string(10);
    std::string time_str = System::local_time();

    String::replace_all(time_str, ":", "-");
    String::replace_all(time_str, " ", "_");

    const std::string file_name = String::format("Log_%s_%s.log", time_str, rand_str);
    m_log_file = m_log_directory / file_name;

    LOGC("Creating logger file: %s", m_log_file);
    m_log_stream.open(m_log_file, std::ios::out);

    return m_log_stream.good();
}

//==================================================================================================
LoggerTask::LoggerTask(std::weak_ptr<Logger> weak_logger) noexcept :
    Task(),
    m_weak_logger(weak_logger)
{
}

//==================================================================================================
void LoggerTask::run()
{
    std::shared_ptr<Logger> logger = m_weak_logger.lock();

    if (logger && logger->poll())
    {
        logger->m_task_runner->post_task(logger->m_task);
    }
}

} // namespace fly

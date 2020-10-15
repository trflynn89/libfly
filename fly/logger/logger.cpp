#include "fly/logger/logger.hpp"

#include "fly/coders/coder_config.hpp"
#include "fly/logger/detail/console_sink.hpp"
#include "fly/logger/detail/file_sink.hpp"
#include "fly/logger/detail/registry.hpp"
#include "fly/logger/log.hpp"
#include "fly/logger/log_sink.hpp"
#include "fly/logger/logger_config.hpp"
#include "fly/task/task_runner.hpp"

namespace fly {

//==================================================================================================
Logger::Logger(
    const std::string &name,
    const std::shared_ptr<SequencedTaskRunner> &task_runner,
    const std::shared_ptr<LoggerConfig> &config,
    std::unique_ptr<LogSink> &&sink) noexcept :
    m_name(name),
    m_config(config),
    m_sink(std::move(sink)),
    m_task_runner(task_runner),
    m_start_time(std::chrono::high_resolution_clock::now())
{
}

//==================================================================================================
Logger::~Logger()
{
    detail::Registry::instance().unregister_logger(m_name);
}

//==================================================================================================
std::shared_ptr<Logger> Logger::create_logger(
    const std::string &name,
    const std::shared_ptr<LoggerConfig> &logger_config,
    std::unique_ptr<LogSink> &&sink)
{
    return create_logger(name, nullptr, logger_config, std::move(sink));
}

//==================================================================================================
std::shared_ptr<Logger> Logger::create_logger(
    const std::string &name,
    const std::shared_ptr<SequencedTaskRunner> &task_runner,
    const std::shared_ptr<LoggerConfig> &logger_config,
    std::unique_ptr<LogSink> &&sink)
{
    auto logger =
        std::shared_ptr<Logger>(new Logger(name, task_runner, logger_config, std::move(sink)));

    if (detail::Registry::instance().register_logger(logger) && logger->initialize())
    {
        return logger;
    }

    return nullptr;
}

//==================================================================================================
std::shared_ptr<Logger> Logger::create_file_logger(
    const std::string &name,
    const std::shared_ptr<LoggerConfig> &logger_config,
    const std::shared_ptr<CoderConfig> &coder_config,
    const std::filesystem::path &logger_directory)
{
    return create_file_logger(name, nullptr, logger_config, coder_config, logger_directory);
}

//==================================================================================================
std::shared_ptr<Logger> Logger::create_file_logger(
    const std::string &name,
    const std::shared_ptr<SequencedTaskRunner> &task_runner,
    const std::shared_ptr<LoggerConfig> &logger_config,
    const std::shared_ptr<CoderConfig> &coder_config,
    const std::filesystem::path &logger_directory)
{
    auto sink = std::make_unique<detail::FileSink>(logger_config, coder_config, logger_directory);
    return create_logger(name, task_runner, logger_config, std::move(sink));
}

//==================================================================================================
std::shared_ptr<Logger> Logger::create_console_logger(
    const std::string &name,
    const std::shared_ptr<LoggerConfig> &logger_config)
{
    return create_console_logger(name, nullptr, logger_config);
}

//==================================================================================================
std::shared_ptr<Logger> Logger::create_console_logger(
    const std::string &name,
    const std::shared_ptr<SequencedTaskRunner> &task_runner,
    const std::shared_ptr<LoggerConfig> &logger_config)
{
    auto sink = std::make_unique<detail::ConsoleSink>();
    return create_logger(name, task_runner, logger_config, std::move(sink));
}

//==================================================================================================
void Logger::set_default_logger(const std::shared_ptr<Logger> &default_logger)
{
    detail::Registry::instance().set_default_logger(default_logger);
}

//==================================================================================================
Logger *Logger::get_default_logger()
{
    return detail::Registry::instance().get_default_logger();
}

//==================================================================================================
std::shared_ptr<Logger> Logger::get(const std::string &name)
{
    return detail::Registry::instance().get_logger(name);
}

//==================================================================================================
const std::string &Logger::name() const
{
    return m_name;
}

//==================================================================================================
bool Logger::initialize()
{
    if (m_sink && m_sink->initialize())
    {
        m_last_task_failed.store(false);
        return true;
    }

    return false;
}

//==================================================================================================
void Logger::log(Log::Level level, Log::Trace &&trace, std::string &&message)
{
    if (m_last_task_failed || (level < Log::Level::Debug) || (level >= Log::Level::NumLevels))
    {
        return;
    }

    const auto now = std::chrono::high_resolution_clock::now();
    const auto log_time =
        std::chrono::duration_cast<std::chrono::duration<double>>(now - m_start_time);

    Log log(std::move(trace), std::move(message), m_config->max_message_size());
    log.m_index = m_index++;
    log.m_level = level;
    log.m_time = log_time.count();

    if (m_task_runner)
    {
        std::weak_ptr<Logger> weak_self = shared_from_this();
        m_queue.push(std::move(log));

        auto task = [weak_self]() {
            if (auto self = weak_self.lock(); self && !self->m_last_task_failed)
            {
                Log queued_log;

                if (self->m_queue.pop(queued_log, self->m_config->queue_wait_time()))
                {
                    const bool accepted = self->m_sink->stream(std::move(queued_log));
                    self->m_last_task_failed.store(!accepted);
                }
            }
        };

        m_task_runner->post_task(FROM_HERE, std::move(task));
    }
    else
    {
        const bool accepted = m_sink->stream(std::move(log));
        m_last_task_failed.store(!accepted);
    }
}

} // namespace fly

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
    std::string name,
    std::shared_ptr<fly::task::SequencedTaskRunner> task_runner,
    std::shared_ptr<LoggerConfig> config,
    std::unique_ptr<LogSink> &&sink) noexcept :
    m_name(std::move(name)),
    m_config(std::move(config)),
    m_sink(std::move(sink)),
    m_task_runner(std::move(task_runner)),
    m_start_time(std::chrono::steady_clock::now())
{
}

//==================================================================================================
Logger::~Logger()
{
    detail::Registry::instance().unregister_logger(m_name);
}

//==================================================================================================
std::shared_ptr<Logger> Logger::create_logger(
    std::string name,
    std::shared_ptr<LoggerConfig> logger_config,
    std::unique_ptr<LogSink> &&sink)
{
    return create_logger(std::move(name), nullptr, std::move(logger_config), std::move(sink));
}

//==================================================================================================
std::shared_ptr<Logger> Logger::create_logger(
    std::string name,
    std::shared_ptr<fly::task::SequencedTaskRunner> task_runner,
    std::shared_ptr<LoggerConfig> logger_config,
    std::unique_ptr<LogSink> &&sink)
{
    // Logger has a private constructor, thus cannot be used with std::make_shared. This class is
    // used to expose the private constructor locally.
    struct LoggerImpl final : public Logger
    {
        LoggerImpl(
            std::string &&name,
            std::shared_ptr<fly::task::SequencedTaskRunner> &&task_runner,
            std::shared_ptr<LoggerConfig> &&logger_config,
            std::unique_ptr<LogSink> &&sink) noexcept :
            Logger(
                std::move(name),
                std::move(task_runner),
                std::move(logger_config),
                std::move(sink))
        {
        }
    };

    auto logger = std::make_shared<LoggerImpl>(
        std::move(name),
        std::move(task_runner),
        std::move(logger_config),
        std::move(sink));

    if (detail::Registry::instance().register_logger(logger) && logger->initialize())
    {
        return logger;
    }

    return nullptr;
}

//==================================================================================================
std::shared_ptr<Logger> Logger::create_file_logger(
    std::string name,
    std::shared_ptr<LoggerConfig> logger_config,
    std::shared_ptr<CoderConfig> coder_config,
    std::filesystem::path logger_directory)
{
    return create_file_logger(
        std::move(name),
        nullptr,
        std::move(logger_config),
        std::move(coder_config),
        std::move(logger_directory));
}

//==================================================================================================
std::shared_ptr<Logger> Logger::create_file_logger(
    std::string name,
    std::shared_ptr<fly::task::SequencedTaskRunner> task_runner,
    std::shared_ptr<LoggerConfig> logger_config,
    std::shared_ptr<CoderConfig> coder_config,
    std::filesystem::path logger_directory)
{
    auto sink = std::make_unique<detail::FileSink>(
        logger_config,
        std::move(coder_config),
        std::move(logger_directory));

    return create_logger(
        std::move(name),
        std::move(task_runner),
        std::move(logger_config),
        std::move(sink));
}

//==================================================================================================
std::shared_ptr<Logger>
Logger::create_console_logger(std::string name, std::shared_ptr<LoggerConfig> logger_config)
{
    return create_console_logger(std::move(name), nullptr, logger_config);
}

//==================================================================================================
std::shared_ptr<Logger> Logger::create_console_logger(
    std::string name,
    std::shared_ptr<fly::task::SequencedTaskRunner> task_runner,
    std::shared_ptr<LoggerConfig> logger_config)
{
    auto sink = std::make_unique<detail::ConsoleSink>();

    return create_logger(
        std::move(name),
        std::move(task_runner),
        std::move(logger_config),
        std::move(sink));
}

//==================================================================================================
void Logger::set_default_logger(std::shared_ptr<Logger> default_logger)
{
    detail::Registry::instance().set_default_logger(std::move(default_logger));
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

    const auto now = std::chrono::steady_clock::now();

    if (m_task_runner)
    {
        auto task = [level, trace = std::move(trace), message = std::move(message), now](
                        std::shared_ptr<Logger> self) mutable
        {
            if (!self->m_last_task_failed)
            {
                self->log_to_sink(level, std::move(trace), std::move(message), now);
            }
        };

        std::weak_ptr<Logger> weak_self = shared_from_this();
        m_task_runner->post_task(FROM_HERE, std::move(task), std::move(weak_self));
    }
    else
    {
        log_to_sink(level, std::move(trace), std::move(message), now);
    }
}

//==================================================================================================
void Logger::log_to_sink(
    Log::Level level,
    Log::Trace &&trace,
    std::string &&message,
    std::chrono::steady_clock::time_point time)
{
    const std::chrono::duration<double, std::milli> elapsed = time - m_start_time;

    Log log(std::move(trace), std::move(message), m_config->max_message_size());
    log.m_index = m_index++;
    log.m_level = level;
    log.m_time = elapsed.count();

    const bool accepted = m_sink->stream(std::move(log));
    m_last_task_failed.store(!accepted);
}

} // namespace fly

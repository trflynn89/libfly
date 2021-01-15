#include "fly/logger/detail/registry.hpp"

#include "fly/logger/detail/console_sink.hpp"
#include "fly/logger/logger.hpp"
#include "fly/logger/logger_config.hpp"

namespace fly::detail {

namespace {

    constexpr const char *s_default_logger_name = "_libfly_default_";

} // namespace

//==================================================================================================
Registry::Registry() :
    m_initial_default_logger(std::shared_ptr<Logger>(new Logger(
        s_default_logger_name,
        nullptr,
        std::make_shared<fly::LoggerConfig>(),
        std::make_unique<detail::ConsoleSink>())))
{
    m_initial_default_logger->initialize(); // Synchronous console initialization cannot fail.
    set_default_logger(m_initial_default_logger);
}

//==================================================================================================
Registry::~Registry()
{
    m_initial_default_logger.reset();
    m_default_logger.reset();

    std::lock_guard<std::mutex> lock(m_registry_mutex);
    m_registry.clear();
}

//==================================================================================================
Registry &Registry::instance()
{
    static Registry s_singleton;
    return s_singleton;
}

//==================================================================================================
void Registry::set_default_logger(const std::shared_ptr<fly::Logger> &default_logger)
{
    if (default_logger)
    {
        m_default_logger = default_logger;
    }
    else
    {
        m_default_logger = m_initial_default_logger;
    }
}

//==================================================================================================
fly::Logger *Registry::get_default_logger() const
{
    return m_default_logger.get();
}

//==================================================================================================
bool Registry::register_logger(const std::shared_ptr<fly::Logger> &logger)
{
    std::lock_guard<std::mutex> lock(m_registry_mutex);

    if (m_registry.find(logger->name()) != m_registry.end())
    {
        return false;
    }

    m_registry[logger->name()] = logger;
    return true;
}

//==================================================================================================
void Registry::unregister_logger(const std::string &name)
{
    if (name == s_default_logger_name)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(m_registry_mutex);
    m_registry.erase(name);
}

//==================================================================================================
std::shared_ptr<fly::Logger> Registry::get_logger(const std::string &name)
{
    std::lock_guard<std::mutex> lock(m_registry_mutex);

    if (auto it = m_registry.find(name); it != m_registry.end())
    {
        return it->second.lock();
    }

    return nullptr;
}

} // namespace fly::detail

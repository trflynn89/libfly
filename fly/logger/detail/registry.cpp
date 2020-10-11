#include "fly/logger/detail/registry.hpp"

#include "fly/logger/logger.hpp"
#include "fly/logger/logger_config.hpp"

namespace fly::detail {

//==================================================================================================
Registry::Registry() :
    m_initial_default_logger(
        fly::Logger::create_console_logger(std::make_shared<fly::LoggerConfig>()))
{
    set_default_logger(m_initial_default_logger);
}

//==================================================================================================
Registry &Registry::instance()
{
    static Registry s_singleton;
    return s_singleton;
}

//==================================================================================================
void Registry::set_default_logger(const std::shared_ptr<Logger> &default_logger)
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
Logger *Registry::get_default_logger() const
{
    return m_default_logger.get();
}

} // namespace fly::detail

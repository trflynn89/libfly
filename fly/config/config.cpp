#include "fly/config/config.hpp"

#include <mutex>

namespace fly::config {

//==================================================================================================
void Config::update(const Json &values)
{
    std::unique_lock<std::shared_timed_mutex> lock(m_values_mutex);
    m_values = values;
}

} // namespace fly::config

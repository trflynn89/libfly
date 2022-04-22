#include "fly/config/config.hpp"

#include <mutex>

namespace fly::config {

//==================================================================================================
void Config::update(Json const &values)
{
    std::unique_lock<std::shared_timed_mutex> lock(m_values_mutex);
    m_values = values;
}

} // namespace fly::config

#include "fly/config/config.hpp"

namespace fly {

//==================================================================================================
void Config::update(const Json &values) noexcept
{
    std::unique_lock<std::shared_timed_mutex> lock(m_values_mutex);
    m_values = values;
}

} // namespace fly

#include "fly/config/config.h"

namespace fly {

//==============================================================================
void Config::Update(const Json &values) noexcept
{
    std::unique_lock<std::shared_timed_mutex> lock(m_valuesMutex);
    m_values = values;
}

} // namespace fly

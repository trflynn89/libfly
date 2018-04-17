#include "fly/config/config.h"

namespace fly {

//==============================================================================
Config::Config()
{
}

//==============================================================================
Config::~Config()
{
}

//==============================================================================
std::string Config::GetName()
{
    return "Config";
}

//==============================================================================
void Config::Update(const Json &values)
{
    std::unique_lock<std::shared_timed_mutex> lock(m_valuesMutex);
    m_values = values;
}

}

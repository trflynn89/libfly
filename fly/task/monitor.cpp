#include "fly/task/monitor.h"

#include <memory>

#include "fly/config/config_manager.h"
#include "fly/task/task_config.h"

namespace fly {

//==============================================================================
Monitor::Monitor(
    const std::string &name,
    ConfigManagerPtr &spConfigManager
) :
    Runner(name, 1),
    m_spConfig(spConfigManager->CreateConfig<TaskConfig>())
{
}

//==============================================================================
bool Monitor::StartRunner()
{
    StartMonitor();
    return IsValid();
}

//==============================================================================
void Monitor::StopRunner()
{
    StopMonitor();
}

//==============================================================================
bool Monitor::DoWork()
{
    if (IsValid())
    {
        Poll(m_spConfig->PollInterval());
    }

    return IsValid();
}

}

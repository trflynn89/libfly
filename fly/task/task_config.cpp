#include "fly/task/task_config.h"

namespace fly {

//==============================================================================
TaskConfig::TaskConfig()
{
}

//==============================================================================
TaskConfig::~TaskConfig()
{
}

//==============================================================================
std::string TaskConfig::GetName()
{
    return "task";
}

//==============================================================================
int TaskConfig::DefaultWorkerCount() const
{
    return GetValue<int>("default_worker_count", 1);
}

}

#include "fly/task/task_config.h"

namespace fly {

//==============================================================================
std::string TaskConfig::GetName()
{
    return "task";
}

//==============================================================================
std::chrono::milliseconds TaskConfig::PollInterval() const
{
    return std::chrono::milliseconds(
        GetValue<std::chrono::milliseconds::rep>("poll_interval", I64(1000))
    );
}

}

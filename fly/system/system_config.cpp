#include "fly/system/system_config.h"

namespace fly {

//==============================================================================
std::string SystemConfig::GetName()
{
    return "system";
}

//==============================================================================
std::chrono::milliseconds SystemConfig::PollInterval() const
{
    return std::chrono::milliseconds(
        GetValue<std::chrono::milliseconds::rep>("poll_interval", I64(100))
    );
}

}

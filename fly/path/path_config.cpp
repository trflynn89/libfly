#include "fly/path/path_config.h"

namespace fly {

//==============================================================================
std::string PathConfig::GetName()
{
    return "path";
}

//==============================================================================
std::chrono::milliseconds PathConfig::PollInterval() const
{
    return std::chrono::milliseconds(
        GetValue<std::chrono::milliseconds::rep>("poll_interval", I64(1000))
    );
}

}

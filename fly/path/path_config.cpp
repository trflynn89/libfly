#include "fly/path/path_config.h"

namespace fly {

//==============================================================================
PathConfig::PathConfig()
{
}

//==============================================================================
PathConfig::~PathConfig()
{
}

//==============================================================================
std::string PathConfig::GetName()
{
    return "path";
}

//==============================================================================
std::chrono::milliseconds PathConfig::PollTimeout() const
{
    return std::chrono::milliseconds(
        GetValue<std::chrono::milliseconds::rep>("poll_timeout", I64(1000))
    );
}

}

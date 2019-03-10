#include "fly/path/path.h"

namespace fly {

//==============================================================================
std::vector<std::string> Path::Split(const std::string &path)
{
    std::vector<std::string> segments(2);

    const std::string::size_type index =
        path.find_last_of(std::filesystem::path::preferred_separator);

    if (index == std::string::npos)
    {
        segments[1] = path;
    }
    else
    {
        segments[0] = path.substr(0, index);
        segments[1] = path.substr(index + 1);
    }

    return segments;
}

} // namespace fly

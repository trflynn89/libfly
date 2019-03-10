#include "test/util/path_util.h"

#include "fly/path/path.h"
#include "fly/types/string.h"

#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>

namespace fly {

//==============================================================================
std::string PathUtil::GenerateTempDirectory()
{
    return fly::Path::Join(
        std::filesystem::temp_directory_path().string(),
        fly::String::GenerateRandomString(10));
}

//==============================================================================
bool PathUtil::WriteFile(const std::string &path, const std::string &contents)
{
    std::ofstream stream(path, std::ios::out);

    if (stream.good())
    {
        stream << contents;
    }

    return stream.good();
}

//==============================================================================
std::string PathUtil::ReadFile(const std::string &path)
{
    std::ifstream stream(path, std::ios::in);
    std::stringstream sstream;

    if (stream.good())
    {
        sstream << stream.rdbuf();
    }

    return sstream.str();
}

} // namespace fly

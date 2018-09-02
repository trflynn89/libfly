#include "test/util/path_util.h"

#include <limits>
#include <fstream>
#include <sstream>

#include "fly/path/path.h"
#include "fly/types/string.h"

namespace fly {

//==============================================================================
std::string PathUtil::GenerateTempDirectory()
{
    return fly::Path::Join(
        fly::Path::GetTempDirectory(),
        fly::String::GenerateRandomString(10)
    );
}

//==============================================================================
bool PathUtil::CreateFile(const std::string &path, const std::string &contents)
{
    {
        std::ofstream stream(path, std::ios::out);

        if (stream.good())
        {
            stream << contents;
        }
    }

    return (contents == ReadFile(path));
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

//==============================================================================
size_t PathUtil::ComputeFileSize(const std::string &path)
{
    std::ifstream stream(path, std::ios::in);
    size_t size = 0;

    if (stream.good())
    {
        stream.ignore(std::numeric_limits<std::streamsize>::max());
        size = static_cast<size_t>(stream.gcount());
    }

    return size;
}

}

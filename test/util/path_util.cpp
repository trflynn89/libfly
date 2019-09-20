#include "test/util/path_util.h"

#include "fly/types/string/string.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <sstream>

namespace fly {

//==============================================================================
std::filesystem::path PathUtil::GenerateTempDirectory() noexcept
{
    return std::filesystem::temp_directory_path() /
        fly::String::GenerateRandomString(10);
}

//==============================================================================
bool PathUtil::WriteFile(
    const std::filesystem::path &path,
    const std::string &contents) noexcept
{
    std::ofstream stream(path, std::ios::out);

    if (stream.good())
    {
        stream << contents;
    }

    return stream.good();
}

//==============================================================================
std::string PathUtil::ReadFile(const std::filesystem::path &path) noexcept
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
bool PathUtil::CompareFiles(
    const std::filesystem::path &path1,
    const std::filesystem::path &path2)
{
    if (std::filesystem::file_size(path1) != std::filesystem::file_size(path2))
    {
        return false;
    }

    std::ifstream file1(path1, std::ios::binary);
    std::ifstream file2(path2, std::ios::binary);

    std::istreambuf_iterator<char> begin1(file1);
    std::istreambuf_iterator<char> begin2(file2);

    return std::equal(begin1, std::istreambuf_iterator<char>(), begin2);
}

} // namespace fly

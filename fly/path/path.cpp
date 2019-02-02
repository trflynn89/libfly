#include "fly/path/path.h"

#include "fly/fly.h"

#include FLY_OS_IMPL_PATH(path, path)

namespace fly {

//==============================================================================
bool Path::MakePath(const std::string &path)
{
    return PathImpl::MakePath(path);
}

//==============================================================================
bool Path::RemovePath(const std::string &path)
{
    return PathImpl::RemovePath(path);
}

//==============================================================================
bool Path::ListPath(
    const std::string &path,
    std::vector<std::string> &directories,
    std::vector<std::string> &files)
{
    directories.clear();
    files.clear();

    return PathImpl::ListPath(path, directories, files);
}

//==============================================================================
char Path::GetSeparator()
{
    return PathImpl::GetSeparator();
}

//==============================================================================
std::vector<std::string> Path::Split(const std::string &path)
{
    static const char separator(GetSeparator());
    std::vector<std::string> segments(2);

    const std::string::size_type index = path.find_last_of(separator);

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

//==============================================================================
std::string Path::GetTempDirectory()
{
    std::string ret = PathImpl::GetTempDirectory();

    if (ret.back() == Path::GetSeparator())
    {
        ret = ret.substr(0, ret.size() - 1);
    }

    return ret;
}

} // namespace fly

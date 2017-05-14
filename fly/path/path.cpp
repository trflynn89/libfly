#include "fly/path/path.h"

#if defined(FLY_WINDOWS)
    #include "fly/path/win/path_impl.h"
#elif defined(FLY_LINUX)
    #include "fly/path/nix/path_impl.h"
#endif

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
char Path::GetSeparator()
{
    return PathImpl::GetSeparator();
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

}

#include "fly/file/win/path_impl.h"

#include <shlobj.h>
#include <tchar.h>
#include <Windows.h>

#include "fly/logger/logger.h"

namespace fly {

//==============================================================================
bool PathImpl::MakePath(const std::string &path)
{
    TCHAR buffer[4096];
    int ret = 0;

    if (::GetFullPathName(path.c_str(), sizeof(buffer), buffer, NULL) > 0)
    {
        ret = ::SHCreateDirectoryEx(NULL, buffer, NULL);
    }
    else
    {
        ret = ERROR_BAD_PATHNAME;
    }

    return (
        (ret == ERROR_SUCCESS) ||
        (ret == ERROR_FILE_EXISTS) ||
        (ret == ERROR_ALREADY_EXISTS)
    );
}

//==============================================================================
bool PathImpl::RemovePath(const std::string &path)
{
    TCHAR buffer[4096];
    DWORD len = ::GetFullPathName(path.c_str(), sizeof(buffer), buffer, NULL);

    bool ret = ((len > 0) && (len < (sizeof(buffer) - 2)));

    if (ret)
    {
        // SHFILEOPSTRUCT requires the path to be double-NULL-terminated
        buffer[len + 1] = '\0';

        SHFILEOPSTRUCT operation = {
            NULL,
            FO_DELETE,
            buffer,
            NULL,
            FOF_NO_UI,
            FALSE,
            NULL,
            NULL
        };

        int error = ::SHFileOperation(&operation);

        if (error == 0)
        {
            LOGD(-1, "Removed \"%s\"", path);
        }
        else
        {
            LOGW(-1, "Could not remove \"%s\": %d (%x)", path, error, error);
            ret = false;
        }
    }

    return ret;
}

//==============================================================================
char PathImpl::GetSeparator()
{
    return '\\';
}

//==============================================================================
std::string PathImpl::GetTempDirectory()
{
    TCHAR buff[MAX_PATH];
    std::string ret;

    if (::GetTempPath(MAX_PATH, buff) > 0)
    {
        ret = std::string(buff);
    }

    return ret;
}

}

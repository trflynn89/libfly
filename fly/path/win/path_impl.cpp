#include "fly/path/win/path_impl.h"

#include <shlobj.h>
#include <strsafe.h>
#include <tchar.h>

#include "fly/logger/logger.h"

namespace fly {

//==============================================================================
bool PathImpl::MakePath(const std::string &path)
{
    TCHAR buffer[4096];
    int ret = 0;

    if (::GetFullPathName(path.c_str(), sizeof(buffer), buffer, NULL) > 0)
    {
        if (PathIsFile(buffer))
        {
            ret = ERROR_BAD_FILE_TYPE;
        }
        else
        {
            ret = ::SHCreateDirectoryEx(NULL, buffer, NULL);
        }
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

    bool ret = ((len > 0) && (len < (sizeof(buffer) - 2)) && !PathIsFile(buffer));

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
bool PathImpl::ListPath(
    const std::string &path,
    std::vector<std::string> &directories,
    std::vector<std::string> &files
)
{
    // For FindFile(), need to append "\\*" - make sure there is enough space
    if (path.length() > (MAX_PATH - 3))
    {
        LOGW(-1, "Path \"%s\" is too long", path);
        return false;
    }

    TCHAR searchPath[MAX_PATH];
    WIN32_FIND_DATA ffd;

    ::StringCchCopy(searchPath, MAX_PATH, path.c_str());
    ::StringCchCat(searchPath, MAX_PATH, TEXT("\\*"));

    HANDLE handle = ::FindFirstFile(searchPath, &ffd);

    if (handle == INVALID_HANDLE_VALUE)
    {
        LOGW(-1, "Could not open \"%s\"", path);
        return false;
    }

    do
    {
        const std::string file(ffd.cFileName);

        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if ((file != ".") && (file != ".."))
            {
                directories.push_back(file);
            }
        }
        else
        {
            files.push_back(file);
        }
    } while (::FindNextFile(handle, &ffd));

    if (System::GetErrorCode() != ERROR_NO_MORE_FILES)
    {
        LOGS(-1, "Could not completely list \"%s\"", path);
        return false;
    }

    return true;
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

//==============================================================================
bool PathImpl::PathIsFile(LPCSTR path)
{
    DWORD attributes = ::GetFileAttributes(path);

    return (
        (attributes != INVALID_FILE_ATTRIBUTES) &&
        ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
    );
}

}

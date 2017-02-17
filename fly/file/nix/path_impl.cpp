#include "fly/file/nix/path_impl.h"

#include <cstring>

#include <fts.h>
#include <sys/stat.h>
#include <unistd.h>

#include "fly/logger/logger.h"
#include "fly/system/system.h"

namespace fly {

//==============================================================================
bool PathImpl::MakePath(const std::string &path)
{
    static const mode_t mode = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
    struct stat st;

    if (::stat(path.c_str(), &st) == 0)
    {
        if (!S_ISDIR(st.st_mode))
        {
            errno = ENOTDIR;
            return false;
        }

        return true;
    }

    size_t pos = path.rfind('/');

    if (pos != std::string::npos)
    {
        if (!MakePath(path.substr(0, pos)))
        {
            return false;
        }
    }

    return ((::mkdir(path.c_str(), mode) == 0) || (errno == EEXIST));
}

//==============================================================================
bool PathImpl::RemovePath(const std::string &path)
{
    static const int mode = FTS_NOCHDIR | FTS_PHYSICAL | FTS_XDEV;
    struct stat st;

    bool ret = ((::stat(path.c_str(), &st) == 0) && S_ISDIR(st.st_mode));

    if (ret)
    {
        char *files[] = { (char *)path.c_str(), NULL };

        FTS *pFts = ::fts_open(files, mode, NULL);
        FTSENT *pCurr = NULL;

        while (ret && (pFts != NULL) && ((pCurr = ::fts_read(pFts)) != NULL))
        {
            std::string file(pCurr->fts_path, pCurr->fts_pathlen);

            switch(pCurr->fts_info)
            {
            case FTS_NS:
            case FTS_DNR:
            case FTS_ERR:
                LOGW(-1, "Could not read \"%s\": %s", file, ::strerror(pCurr->fts_errno));
                ret = false;
                break;

            case FTS_DP:
            case FTS_F:
            case FTS_SL:
            case FTS_SLNONE:
            case FTS_DEFAULT:
                if (::remove(pCurr->fts_accpath) == 0)
                {
                    LOGD(-1, "Removed \"%s\"", file);
                }
                else
                {
                    LOGW(-1, "Could not remove \"%s\": %s", file, System::GetLastError());
                    ret = false;
                }

                break;

            default:
                break;
            }
        }

        if (pFts != NULL)
        {
            ::fts_close(pFts);
        }
    }

    return ret;
}

//==============================================================================
char PathImpl::GetSeparator()
{
    return '/';
}

//==============================================================================
std::string PathImpl::GetTempDirectory()
{
    static const std::string envs[] = { "TMPDIR", "TMP", "TEMP", "TEMPDIR", "" };

    for (int i = 0; !envs[i].empty(); ++i)
    {
        char *dir = ::getenv(envs[i].c_str());

        if (dir != NULL)
        {
            return std::string(dir);
        }
    }

    return std::string("/tmp");
}

}

#include "fly/path/nix/path_impl.h"

#include "fly/logger/logger.h"
#include "fly/path/path.h"

#include <dirent.h>
#include <fts.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstring>

namespace fly {

namespace {

    const mode_t s_makePathMode = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;

    const int s_ftsMode = FTS_NOCHDIR | FTS_PHYSICAL | FTS_XDEV;

    const std::string s_tmpEnvs[] = { "TMPDIR", "TMP", "TEMP", "TEMPDIR", "" };

    /**
     * RAII wrapper around ::fts_open().
     */
    class FtsWrapper
    {
    public:
        FtsWrapper(char *const *files) :
            m_pFts(::fts_open(files, s_ftsMode, NULL))
        {
        }

        ~FtsWrapper()
        {
            if (m_pFts != NULL)
            {
                ::fts_close(m_pFts);
            }
        }

        FTS *operator()() const
        {
            return m_pFts;
        }

        operator bool() const
        {
            return (m_pFts != NULL);
        }

    private:
        FTS *m_pFts;
    };

    /**
     * RAII wrapper around ::opendir().
     */
    class DirWrapper
    {
    public:
        DirWrapper(const char *path) : m_pDir(::opendir(path))
        {
            if (m_pDir == NULL)
            {
                LOGS(-1, "Could not open \"%s\"", path);
            }
        }

        ~DirWrapper()
        {
            if (m_pDir != NULL)
            {
                ::closedir(m_pDir);
            }
        }

        DIR *operator()() const
        {
            return m_pDir;
        }

        operator bool() const
        {
            return (m_pDir != NULL);
        }

    private:
        DIR *m_pDir;
    };

} // namespace

//==============================================================================
bool PathImpl::MakePath(const std::string &path)
{
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

    return ((::mkdir(path.c_str(), s_makePathMode) == 0) || (errno == EEXIST));
}

//==============================================================================
bool PathImpl::RemovePath(const std::string &path)
{
    struct stat st;
    bool ret = ((::stat(path.c_str(), &st) == 0) && S_ISDIR(st.st_mode));

    if (ret)
    {
        char *files[] = { (char *)path.c_str(), NULL };

        FtsWrapper fts(files);
        FTSENT *pCurr = NULL;

        while (ret && fts && ((pCurr = ::fts_read(fts())) != NULL))
        {
            std::string file(pCurr->fts_path, pCurr->fts_pathlen);

            switch (pCurr->fts_info)
            {
                case FTS_NS:
                case FTS_DNR:
                case FTS_ERR:
                    errno = pCurr->fts_errno; // errno may not be set
                    LOGS(-1, "Could not read \"%s\"", file);
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
                        LOGS(-1, "Could not remove \"%s\"", file);
                        ret = false;
                    }

                    break;

                default:
                    break;
            }
        }
    }

    return ret;
}

//==============================================================================
bool PathImpl::ListPath(
    const std::string &path,
    std::vector<std::string> &directories,
    std::vector<std::string> &files)
{
    DirWrapper dir(path.c_str());
    struct dirent *ent = NULL;

    while (dir && (ent = ::readdir(dir())) != NULL)
    {
        const std::string file(ent->d_name);

        switch (ent->d_type)
        {
            case DT_DIR:
                if ((file != ".") && (file != ".."))
                {
                    directories.push_back(file);
                }
                break;

            case DT_LNK:
            case DT_REG:
                files.push_back(file);
                break;

            default:
                break;
        }
    }

    return dir;
}

//==============================================================================
char PathImpl::GetSeparator()
{
    return '/';
}

//==============================================================================
std::string PathImpl::GetTempDirectory()
{
    for (int i = 0; !s_tmpEnvs[i].empty(); ++i)
    {
        char *dir = ::getenv(s_tmpEnvs[i].c_str());

        if (dir != NULL)
        {
            return std::string(dir);
        }
    }

    return std::string("/tmp");
}

} // namespace fly

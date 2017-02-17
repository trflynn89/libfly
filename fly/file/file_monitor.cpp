#include "fly/file/file_monitor.h"

#include "fly/logger/logger.h"

namespace fly {

namespace
{
    // TODO make configurable
    static const std::chrono::milliseconds s_pollTimeout(1000);
}

//==============================================================================
FileMonitor::FileMonitor() : Runner("FileMonitor", 1)
{
}

//==============================================================================
FileMonitor::~FileMonitor()
{
    RemoveAllPaths();
    Stop();
}

//==============================================================================
bool FileMonitor::AddFile(
    const std::string &path,
    const std::string &file,
    FileEventCallback callback
)
{
    if (callback == nullptr)
    {
        LOGW(-1, "Ignoring NULL callback for \"%s\"", path);
        return false;
    }

    PathInfoPtr spInfo;

    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_pathInfo.find(path);

    if (it == m_pathInfo.end())
    {
        spInfo = CreatePathInfo(path);

        if (spInfo && spInfo->IsValid())
        {
            m_pathInfo[path] = spInfo;
        }
        else
        {
            return false;
        }
    }
    else
    {
        spInfo = it->second;
    }

    LOGD(-1, "Monitoring \"%s\" in \"%s\"", file, path);
    spInfo->m_handlers[file] = callback;

    return true;
}

//==============================================================================
bool FileMonitor::RemoveFile(const std::string &path, const std::string &file)
{
    bool removePath = false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_pathInfo.find(path);

        if (it == m_pathInfo.end())
        {
            LOGW(-1, "Wasn't monitoring \"%s\"", path);
            return false;
        }

        PathInfoPtr spInfo(it->second);
        auto it2 = spInfo->m_handlers.find(file);

        if (it2 == spInfo->m_handlers.end())
        {
            LOGW(-1, "Wasn't monitoring \"%s\" in \"%s\"", file, path);
            return false;
        }

        LOGD(-1, "Stopped monitoring \"%s\" in \"%s\"", file, path);
        spInfo->m_handlers.erase(it2);

        removePath = spInfo->m_handlers.empty();
    }

    return (removePath ? RemovePath(path) : true);
}

//==============================================================================
bool FileMonitor::RemovePath(const std::string &path)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_pathInfo.find(path);

    if (it == m_pathInfo.end())
    {
        LOGW(-1, "Wasn't monitoring \"%s\"", path);
        return false;
    }

    LOGI(-1, "Removed monitor for \"%s\"", path);
    m_pathInfo.erase(it);

    return true;
}

//==============================================================================
void FileMonitor::RemoveAllPaths()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    LOGI(-1, "Removed all monitors");
    m_pathInfo.clear();
}

//==============================================================================
bool FileMonitor::StartRunner()
{
    return IsValid();
}

//==============================================================================
void FileMonitor::StopRunner()
{
    Close();
}

//==============================================================================
bool FileMonitor::DoWork()
{
    if (IsValid())
    {
        Poll(s_pollTimeout);
    }

    return IsValid();
}

}

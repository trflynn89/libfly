#include "fly/path/path_monitor.h"

#include "fly/config/config_manager.h"
#include "fly/logger/logger.h"
#include "fly/path/path_config.h"

namespace fly {

//==============================================================================
PathMonitor::PathMonitor() :
    Runner("PathMonitor", 1),
    m_spConfig(std::make_shared<PathConfig>())
{
}

//==============================================================================
PathMonitor::PathMonitor(ConfigManagerPtr &spConfigManager) :
    Runner("PathMonitor", 1),
    m_spConfig(spConfigManager->CreateConfig<PathConfig>())
{
}

//==============================================================================
PathMonitor::~PathMonitor()
{
    RemoveAllPaths();
    Stop();
}

//==============================================================================
bool PathMonitor::AddPath(const std::string &path, PathEventCallback callback)
{
    if (callback == nullptr)
    {
        LOGW(-1, "Ignoring NULL callback for \"%s\"", path);
    }
    else
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        PathInfoPtr spInfo = getOrCreatePathInfo(path);

        if (spInfo)
        {
            LOGD(-1, "Monitoring all files in \"%s\"", path);
            spInfo->m_pathHandler = callback;

            return true;
        }
    }

    return false;
}

//==============================================================================
bool PathMonitor::RemovePath(const std::string &path)
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
void PathMonitor::RemoveAllPaths()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    LOGI(-1, "Removed all monitors");
    m_pathInfo.clear();
}

//==============================================================================
bool PathMonitor::AddFile(
    const std::string &path,
    const std::string &file,
    PathEventCallback callback
)
{
    if (callback == nullptr)
    {
        LOGW(-1, "Ignoring NULL callback for \"%s\"", path);
    }
    else
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        PathInfoPtr spInfo = getOrCreatePathInfo(path);

        if (spInfo)
        {
            LOGD(-1, "Monitoring \"%s\" in \"%s\"", file, path);
            spInfo->m_fileHandlers[file] = callback;

            return true;
        }
    }

    return false;
}

//==============================================================================
bool PathMonitor::RemoveFile(const std::string &path, const std::string &file)
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
        auto it2 = spInfo->m_fileHandlers.find(file);

        if (it2 == spInfo->m_fileHandlers.end())
        {
            LOGW(-1, "Wasn't monitoring \"%s\" in \"%s\"", file, path);
            return false;
        }

        LOGD(-1, "Stopped monitoring \"%s\" in \"%s\"", file, path);
        spInfo->m_fileHandlers.erase(it2);

        removePath = spInfo->m_fileHandlers.empty();
    }

    return (removePath ? RemovePath(path) : true);
}

//==============================================================================
bool PathMonitor::StartRunner()
{
    return IsValid();
}

//==============================================================================
void PathMonitor::StopRunner()
{
    Close();
}

//==============================================================================
bool PathMonitor::DoWork()
{
    if (IsValid())
    {
        Poll(m_spConfig->PollTimeout());
    }

    return IsValid();
}

//==============================================================================
PathMonitor::PathInfoPtr PathMonitor::getOrCreatePathInfo(const std::string &path)
{
    PathInfoPtr spInfo;

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
            spInfo.reset();
        }
    }
    else
    {
        spInfo = it->second;
    }

    return spInfo;
}

}

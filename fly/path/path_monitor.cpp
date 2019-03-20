#include "fly/path/path_monitor.h"

#include "fly/logger/logger.h"
#include "fly/path/path_config.h"
#include "fly/task/task_runner.h"

#include <system_error>

namespace fly {

//==============================================================================
PathMonitor::PathMonitor(
    const std::shared_ptr<SequencedTaskRunner> &spTaskRunner,
    const std::shared_ptr<PathConfig> &spConfig) noexcept :
    m_spTaskRunner(spTaskRunner),
    m_spConfig(spConfig)
{
}

//==============================================================================
PathMonitor::~PathMonitor()
{
    RemoveAllPaths();
}

//==============================================================================
bool PathMonitor::Start() noexcept
{
    if (IsValid())
    {
        std::shared_ptr<PathMonitor> spPathMonitor = shared_from_this();

        m_spTask = std::make_shared<PathMonitorTask>(spPathMonitor);
        m_spTaskRunner->PostTask(m_spTask);

        return true;
    }

    return false;
}

//==============================================================================
bool PathMonitor::AddPath(
    const std::filesystem::path &path,
    PathEventCallback callback) noexcept
{
    std::error_code error;

    if (callback == nullptr)
    {
        LOGW("Ignoring NULL callback for %s", path);
    }
    else if (!std::filesystem::is_directory(path, error))
    {
        LOGW("Ignoring non-directory %s: %s", path, error);
    }
    else
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::shared_ptr<PathInfo> spInfo = getOrCreatePathInfo(path);

        if (spInfo)
        {
            LOGD("Monitoring all files in %s", path);
            spInfo->m_pathHandler = callback;

            return true;
        }
    }

    return false;
}

//==============================================================================
bool PathMonitor::RemovePath(const std::filesystem::path &path) noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_pathInfo.find(path);

    if (it == m_pathInfo.end())
    {
        LOGW("Wasn't monitoring %s", path);
        return false;
    }

    LOGI("Removed monitor for %s", path);
    m_pathInfo.erase(it);

    return true;
}

//==============================================================================
void PathMonitor::RemoveAllPaths() noexcept
{
    std::lock_guard<std::mutex> lock(m_mutex);

    LOGI("Removed all monitors");
    m_pathInfo.clear();
}

//==============================================================================
bool PathMonitor::AddFile(
    const std::filesystem::path &file,
    PathEventCallback callback) noexcept
{
    std::error_code error;

    if (callback == nullptr)
    {
        LOGW("Ignoring NULL callback for %s", file);
    }
    else if (std::filesystem::is_directory(file, error))
    {
        LOGW("Ignoring directory %s: %s", file, error);
    }
    else if (!std::filesystem::is_directory(file.parent_path(), error))
    {
        LOGW("Ignoring file under non-directory %s: %s", file, error);
    }
    else
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::shared_ptr<PathInfo> spInfo =
            getOrCreatePathInfo(file.parent_path());

        if (spInfo)
        {
            LOGD("Monitoring file %s", file);
            spInfo->m_fileHandlers[file.filename()] = callback;

            return true;
        }
    }

    return false;
}

//==============================================================================
bool PathMonitor::RemoveFile(const std::filesystem::path &file) noexcept
{
    bool removePath = false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_pathInfo.find(file.parent_path());

        if (it == m_pathInfo.end())
        {
            LOGW("Wasn't monitoring %s", file);
            return false;
        }

        std::shared_ptr<PathInfo> spInfo(it->second);
        auto it2 = spInfo->m_fileHandlers.find(file.filename());

        if (it2 == spInfo->m_fileHandlers.end())
        {
            LOGW("Wasn't monitoring %s", file);
            return false;
        }

        LOGD("Stopped monitoring %s", file);
        spInfo->m_fileHandlers.erase(it2);

        removePath = spInfo->m_fileHandlers.empty() && !(spInfo->m_pathHandler);
    }

    return removePath ? RemovePath(file.parent_path()) : true;
}

//==============================================================================
std::shared_ptr<PathMonitor::PathInfo>
PathMonitor::getOrCreatePathInfo(const std::filesystem::path &path) noexcept
{
    std::shared_ptr<PathInfo> spInfo;

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

//==============================================================================
std::ostream &
operator<<(std::ostream &stream, PathMonitor::PathEvent event) noexcept
{
    switch (event)
    {
        case PathMonitor::PathEvent::None:
            stream << "None";
            break;

        case PathMonitor::PathEvent::Created:
            stream << "Created";
            break;

        case PathMonitor::PathEvent::Deleted:
            stream << "Deleted";
            break;

        case PathMonitor::PathEvent::Changed:
            stream << "Changed";
            break;
    }

    return stream;
}

//==============================================================================
PathMonitorTask::PathMonitorTask(
    std::weak_ptr<PathMonitor> wpPathMonitor) noexcept :
    Task(),
    m_wpPathMonitor(wpPathMonitor)
{
}

//==============================================================================
void PathMonitorTask::Run() noexcept
{
    std::shared_ptr<PathMonitor> spPathMonitor = m_wpPathMonitor.lock();

    if (spPathMonitor && spPathMonitor->IsValid())
    {
        spPathMonitor->Poll(spPathMonitor->m_spConfig->PollInterval());

        if (spPathMonitor->IsValid())
        {
            spPathMonitor->m_spTaskRunner->PostTask(spPathMonitor->m_spTask);
        }
    }
}

} // namespace fly

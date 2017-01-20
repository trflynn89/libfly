#include "file_monitor.h"

#include <fly/file/file_monitor_impl.h>
#include <fly/logging/logger.h>

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
        FileMonitorPtr spMonitor = SharedFromThis<FileMonitor>();
        spInfo = std::make_shared<FileMonitorImpl::PathInfoImpl>(spMonitor, path);

        if (spInfo->IsValid())
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

    LOGD(-1, "Watching for changes to \"%s\" in \"%s\"", file, path);
    spInfo->m_handlers[file] = callback;

    return true;
}

//==============================================================================
bool FileMonitor::RemoveFile(const std::string &path, const std::string &file)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_pathInfo.find(path);

    if (it != m_pathInfo.end())
    {
        PathInfoPtr spInfo(it->second);

        auto it2 = spInfo->m_handlers.find(file);

        if (it2 != spInfo->m_handlers.end())
        {
            LOGD(-1, "Stopped watching for changes to \"%s\" in \"%s\"", file, path);
            spInfo->m_handlers.erase(it2);

            if (spInfo->m_handlers.empty())
            {
                LOGI(-1, "Removed watcher for \"%s\"", path);
                m_pathInfo.erase(it);
            }

            return true;
        }
    }

    LOGW(-1, "Not watching for changes to \"%s\" in \"%s\"", file, path);
    return false;
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

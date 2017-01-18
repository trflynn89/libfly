#include "win_file_monitor.h"

#include <fly/logging/logger.h>
#include <fly/system/system.h>

namespace fly {

namespace
{
    static const DWORD s_accessFlags = (
        FILE_LIST_DIRECTORY
    );

    static const DWORD s_shareFlags = (
        FILE_SHARE_WRITE |
        FILE_SHARE_READ |
        FILE_SHARE_DELETE
    );

    static const DWORD s_dispositionFlags = (
        OPEN_EXISTING
    );

    static const DWORD s_attributeFlags = (
        FILE_FLAG_BACKUP_SEMANTICS |
        FILE_FLAG_OVERLAPPED
    );

    static const DWORD s_changeFlags = (
        FILE_NOTIFY_CHANGE_FILE_NAME |
        FILE_NOTIFY_CHANGE_DIR_NAME |
        FILE_NOTIFY_CHANGE_LAST_WRITE |
        FILE_NOTIFY_CHANGE_CREATION
    );

    static const DWORD s_buffSize = 100;
    static const DWORD s_totalSize = (s_buffSize * sizeof(FILE_NOTIFY_INFORMATION));
}

//==============================================================================
FileMonitorImpl::FileMonitorImpl() :
    FileMonitor(),
    m_iocp(CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0))
{
    if (m_iocp == NULL)
    {
        LOGW(-1, "Could not initialize IOCP: %s", fly::System::GetLastError());
    }
}

//==============================================================================
FileMonitorImpl::~FileMonitorImpl()
{
    Close();
}

//==============================================================================
bool FileMonitorImpl::IsValid() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return (m_iocp != NULL);
}

//==============================================================================
bool FileMonitorImpl::AddFile(
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

    DWORD attributes = GetFileAttributes(path.c_str());

    if ((attributes == INVALID_FILE_ATTRIBUTES) ||
        ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0))
    {
        LOGW(-1, "Could not find directory for \"%s\"", path);
        return false;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    PathMonitor &monitor = m_monitoredPaths[path];

    if (monitor.m_handle == INVALID_HANDLE_VALUE)
    {
        monitor.m_handle = ::CreateFile(
            path.c_str(), s_accessFlags, s_shareFlags, NULL, s_dispositionFlags,
            s_attributeFlags, NULL
        );

        if (monitor.m_handle == INVALID_HANDLE_VALUE)
        {
            LOGW(-1, "Could not create file for \"%s\": %s",
                path, fly::System::GetLastError()
            );

            return false;
        }

        monitor.m_pInfo = new FILE_NOTIFY_INFORMATION[s_buffSize];

        if (monitor.m_pInfo == NULL)
        {
            LOGW(-1, "Could not create notification info for \"%s\": %s",
                path, fly::System::GetLastError()
            );

            return false;
        }

        HANDLE iocp = CreateIoCompletionPort(
            monitor.m_handle, m_iocp, (ULONG_PTR)monitor.m_handle, 0
        );

        if (iocp == NULL)
        {
            LOGW(-1, "Could not create IOCP info for \"%s\": %s",
                path, fly::System::GetLastError()
            );

            return false;
        }

        DWORD bytes = 0;
        BOOL success = ::ReadDirectoryChangesW(
            monitor.m_handle, monitor.m_pInfo, s_totalSize, FALSE, s_changeFlags,
            &bytes, &monitor.m_overlapped, NULL
        );

        if (!success)
        {
            LOGW(-1, "Could not check events for \"%s\": %s",
                path, fly::System::GetLastError()
            );

            return false;
        }
    }

    LOGD(-1, "Watching for changes to \"%s\" in \"%s\"", file, path);
    monitor.m_handlers[file] = callback;

    return true;
}

//==============================================================================
bool FileMonitorImpl::RemoveFile(const std::string &path, const std::string &file)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_monitoredPaths.find(path);

    if (it != m_monitoredPaths.end())
    {
        PathMonitor &monitor = it->second;

        auto it2 = monitor.m_handlers.find(file);

        if (it2 != monitor.m_handlers.end())
        {
            LOGD(-1, "Stopped watching for changes to \"%s\" in \"%s\"", file, path);
            monitor.m_handlers.erase(it2);

            if (monitor.m_handlers.empty())
            {
                LOGI(-1, "Removed watcher for \"%s\"", path);
                m_monitoredPaths.erase(it);
            }

            return true;
        }
    }

    LOGW(-1, "Not watching for changes to \"%s\" in \"%s\"", file, path);
    return false;
}

//==============================================================================
void FileMonitorImpl::Poll(const std::chrono::milliseconds &timeout)
{
    DWORD bytes = 0;
    ULONG_PTR pKey = NULL;
    LPOVERLAPPED pOverlapped = NULL;
    DWORD millis = static_cast<DWORD>(timeout.count());

    if (::GetQueuedCompletionStatus(m_iocp, &bytes, &pKey, &pOverlapped, millis))
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = std::find_if(m_monitoredPaths.begin(), m_monitoredPaths.end(),
            [&pKey](const PathMap::value_type &value) -> bool
            {
                return ((ULONG_PTR)value.second.m_handle == pKey);
            }
        );

        if (it != m_monitoredPaths.end())
        {
            handleEvents(*it);
        }

        BOOL success = ::ReadDirectoryChangesW(
            it->second.m_handle, it->second.m_pInfo, s_totalSize, FALSE,
            s_changeFlags, &bytes, &it->second.m_overlapped, NULL
        );

        if (!success)
        {
            LOGW(-1, "Could not check events for \"%s\": %s",
                it->first, fly::System::GetLastError()
            );
        }
    }
}

//==============================================================================
void FileMonitorImpl::Close()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_iocp != NULL)
    {
        ::CloseHandle(m_iocp);
        m_iocp = NULL;
    }
}

//==============================================================================
void FileMonitorImpl::handleEvents(PathMap::value_type &value)
{
    PathMonitor &monitor = value.second;
    PFILE_NOTIFY_INFORMATION pInfo = monitor.m_pInfo;

    while (pInfo != NULL)
    {
        std::wstring wFile(pInfo->FileName, pInfo->FileNameLength / sizeof(wchar_t));
        std::string file(wFile.begin(), wFile.end());

        FileEventCallback callback = monitor.m_handlers[file];
        FileMonitor::FileEvent event = convertToEvent(pInfo->Action);

        if ((callback != nullptr) && (event != FileMonitor::FILE_NO_CHANGE))
        {
            LOGI(-1, "Handling event %d for \"%s\" in \"%s\"",
                event, file, value.first);

            callback(value.first, file, event);
        }

        if (pInfo->NextEntryOffset == U64(0))
        {
            pInfo = NULL;
        }
        else
        {
            pInfo = reinterpret_cast<PFILE_NOTIFY_INFORMATION>(
                reinterpret_cast<LPBYTE>(pInfo) + pInfo->NextEntryOffset
            );
        }
    }
}

//==============================================================================
FileMonitor::FileEvent FileMonitorImpl::convertToEvent(DWORD action)
{
    FileMonitor::FileEvent event = FileMonitor::FILE_NO_CHANGE;

    switch (action)
    {
    case FILE_ACTION_ADDED:
    case FILE_ACTION_RENAMED_NEW_NAME:
        event = FileMonitor::FILE_CREATED;
        break;

    case FILE_ACTION_REMOVED:
    case FILE_ACTION_RENAMED_OLD_NAME:
        event = FileMonitor::FILE_DELETED;
        break;

    case FILE_ACTION_MODIFIED:
        event = FileMonitor::FILE_CHANGED;
        break;
    }

    return event;
}

}

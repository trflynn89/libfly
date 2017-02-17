#include "fly/file/win/file_monitor_impl.h"

#include "fly/logger/logger.h"
#include "fly/system/system.h"

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
}

//==============================================================================
FileMonitorImpl::FileMonitorImpl() :
    FileMonitor(),
    m_iocp(::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0))
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
    return (m_iocp != NULL);
}

//==============================================================================
FileMonitor::PathInfoPtr FileMonitorImpl::CreatePathInfo(const std::string &path) const
{
    FileMonitor::PathInfoPtr spInfo;

    if (IsValid())
    {
        spInfo = std::make_shared<PathInfoImpl>(m_iocp, path);
    }

    return spInfo;
}

//==============================================================================
void FileMonitorImpl::Poll(const std::chrono::milliseconds &timeout)
{
    DWORD bytes = 0;
    ULONG_PTR pKey = NULL;
    LPOVERLAPPED pOverlapped = NULL;
    DWORD millis = static_cast<DWORD>(timeout.count());

    std::string pathToRemove;

    if (::GetQueuedCompletionStatus(m_iocp, &bytes, &pKey, &pOverlapped, millis))
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = std::find_if(m_pathInfo.begin(), m_pathInfo.end(),
            [&pKey](const PathInfoMap::value_type &value) -> bool
            {
                PathInfoImplPtr spInfo(DownCast<PathInfoImpl>(value.second));
                return ((ULONG_PTR)(spInfo->m_handle) == pKey);
            }
        );

        if (it != m_pathInfo.end())
        {
            PathInfoImplPtr spInfo(DownCast<PathInfoImpl>(it->second));
            handleEvents(spInfo, it->first);

            if (!spInfo->Refresh(it->first))
            {
                pathToRemove = it->first;
            }
        }
    }

    if (!pathToRemove.empty())
    {
        RemovePath(pathToRemove);
    }
}

//==============================================================================
void FileMonitorImpl::Close()
{
    if (m_iocp != NULL)
    {
        ::CloseHandle(m_iocp);
        m_iocp = NULL;
    }
}

//==============================================================================
void FileMonitorImpl::handleEvents(
    const PathInfoImplPtr &spInfo,
    const std::string &path
) const
{
    PFILE_NOTIFY_INFORMATION pInfo = spInfo->m_pInfo;

    while (pInfo != NULL)
    {
        std::wstring wFile(pInfo->FileName, pInfo->FileNameLength / sizeof(wchar_t));
        std::string file(wFile.begin(), wFile.end());

        FileMonitor::FileEventCallback &callback = spInfo->m_handlers[file];
        FileMonitor::FileEvent event = convertToEvent(pInfo->Action);

        if ((callback != nullptr) && (event != FileMonitor::FILE_NO_CHANGE))
        {
            LOGI(-1, "Handling event %d for \"%s\" in \"%s\"",
                event, file, path);

            callback(path, file, event);
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
FileMonitor::FileEvent FileMonitorImpl::convertToEvent(DWORD action) const
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

    default:
        break;
    }

    return event;
}

//==============================================================================
FileMonitorImpl::PathInfoImpl::PathInfoImpl(HANDLE iocp, const std::string &path) :
    FileMonitorImpl::PathInfo(),
    m_valid(false),
    m_handle(INVALID_HANDLE_VALUE),
    m_pInfo(NULL)
{
    ::memset(&m_overlapped, 0, sizeof(m_overlapped));

    DWORD attributes = ::GetFileAttributes(path.c_str());

    if ((attributes == INVALID_FILE_ATTRIBUTES) ||
        ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0))
    {
        LOGW(-1, "Could not find directory for \"%s\"", path);
        return;
    }

    m_handle = ::CreateFile(
        path.c_str(), s_accessFlags, s_shareFlags, NULL, s_dispositionFlags,
        s_attributeFlags, NULL
    );

    if (m_handle == INVALID_HANDLE_VALUE)
    {
        LOGW(-1, "Could not create file for \"%s\": %s",
            path, fly::System::GetLastError()
        );
    }
    else if ((m_pInfo = new FILE_NOTIFY_INFORMATION[s_buffSize]) == NULL)
    {
        LOGW(-1, "Could not create notification info for \"%s\": %s",
            path, fly::System::GetLastError()
        );
    }
    else if (::CreateIoCompletionPort(m_handle, iocp, (ULONG_PTR)m_handle, 0) == NULL)
    {
        LOGW(-1, "Could not create IOCP info for \"%s\": %s",
            path, fly::System::GetLastError()
        );
    }
    else
    {
        m_valid = Refresh(path);
    }
}

//==============================================================================
FileMonitorImpl::PathInfoImpl::~PathInfoImpl()
{
    if (m_pInfo != NULL)
    {
        delete[] m_pInfo;
        m_pInfo = NULL;
    }

    if (m_handle != INVALID_HANDLE_VALUE)
    {
        ::CancelIo(m_handle);
        ::CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }
}

//==============================================================================
bool FileMonitorImpl::PathInfoImpl::IsValid() const
{
    return (m_valid && (m_handle != INVALID_HANDLE_VALUE) && (m_pInfo != NULL));
}

//==============================================================================
bool FileMonitorImpl::PathInfoImpl::Refresh(const std::string &path)
{
    static const DWORD size = (s_buffSize * sizeof(FILE_NOTIFY_INFORMATION));
    DWORD bytes = 0;

    BOOL success = ::ReadDirectoryChangesW(
        m_handle, m_pInfo, size, FALSE, s_changeFlags, &bytes, &m_overlapped, NULL
    );

    if (success == FALSE)
    {
        LOGW(-1, "Could not check events for \"%s\": %s",
            path, fly::System::GetLastError()
        );
    }

    return (success == TRUE);
}

}

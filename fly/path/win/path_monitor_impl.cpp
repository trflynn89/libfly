#include "fly/path/win/path_monitor_impl.h"

#include "fly/fly.h"
#include "fly/logger/logger.h"
#include "fly/task/task_runner.h"

#include <string>

namespace fly {

namespace {

    const DWORD s_accessFlags = FILE_LIST_DIRECTORY;

    const DWORD s_shareFlags =
        FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE;

    const DWORD s_dispositionFlags = OPEN_EXISTING;

    const DWORD s_attributeFlags =
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED;

    const DWORD s_changeFlags = FILE_NOTIFY_CHANGE_FILE_NAME |
        FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE |
        FILE_NOTIFY_CHANGE_CREATION;

    const DWORD s_buffSize = 100;

} // namespace

//==============================================================================
PathMonitorImpl::PathMonitorImpl(
    const std::shared_ptr<SequencedTaskRunner> &spTaskRunner,
    const std::shared_ptr<PathConfig> &spConfig) noexcept :
    PathMonitor(spTaskRunner, spConfig),
    m_iocp(::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0))
{
    if (m_iocp == NULL)
    {
        LOGS("Could not initialize IOCP");
    }
}

//==============================================================================
PathMonitorImpl::~PathMonitorImpl()
{
    if (m_iocp != NULL)
    {
        ::CloseHandle(m_iocp);
        m_iocp = NULL;
    }
}

//==============================================================================
bool PathMonitorImpl::IsValid() const noexcept
{
    return m_iocp != NULL;
}

//==============================================================================
void PathMonitorImpl::Poll(const std::chrono::milliseconds &timeout) noexcept
{
    DWORD bytes = 0;
    ULONG_PTR pKey = NULL;
    LPOVERLAPPED pOverlapped = NULL;
    DWORD delay = static_cast<DWORD>(timeout.count());

    std::filesystem::path pathToRemove;

    if (::GetQueuedCompletionStatus(m_iocp, &bytes, &pKey, &pOverlapped, delay))
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = std::find_if(
            m_pathInfo.begin(),
            m_pathInfo.end(),
            [&pKey](const PathInfoMap::value_type &val) -> bool {
                auto spInfo(std::static_pointer_cast<PathInfoImpl>(val.second));
                return ((ULONG_PTR)(spInfo->m_handle)) == pKey;
            });

        if (it != m_pathInfo.end())
        {
            auto spInfo(std::static_pointer_cast<PathInfoImpl>(it->second));
            handleEvents(spInfo, it->first);

            if (!spInfo->Refresh(it->first))
            {
                // Hold onto the path to be removed until the mutex is released
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
std::shared_ptr<PathMonitor::PathInfo>
PathMonitorImpl::CreatePathInfo(const std::filesystem::path &path) const
    noexcept
{
    std::shared_ptr<PathMonitor::PathInfo> spInfo;

    if (IsValid())
    {
        spInfo = std::make_shared<PathInfoImpl>(m_iocp, path);
    }

    return spInfo;
}

//==============================================================================
void PathMonitorImpl::handleEvents(
    const std::shared_ptr<PathInfoImpl> &spInfo,
    const std::filesystem::path &path) const noexcept
{
    PFILE_NOTIFY_INFORMATION pInfo = spInfo->m_pInfo;

    while (pInfo != NULL)
    {
        PathMonitor::PathEvent event = convertToEvent(pInfo->Action);

        if (event != PathMonitor::PathEvent::None)
        {
            const std::wstring wFile(
                pInfo->FileName, pInfo->FileNameLength / sizeof(wchar_t));

            const std::filesystem::path file(wFile);
            auto callback = spInfo->m_fileHandlers[file];

            if (callback == nullptr)
            {
                callback = spInfo->m_pathHandler;
            }

            if (callback != nullptr)
            {
                auto full = path / file;

                LOGI("Handling event %d for %s", event, full);
                callback(full, event);
            }
        }

        if (pInfo->NextEntryOffset == U64(0))
        {
            pInfo = NULL;
        }
        else
        {
            pInfo = reinterpret_cast<PFILE_NOTIFY_INFORMATION>(
                reinterpret_cast<LPBYTE>(pInfo) + pInfo->NextEntryOffset);
        }
    }
}

//==============================================================================
PathMonitor::PathEvent PathMonitorImpl::convertToEvent(DWORD action) const
    noexcept
{
    PathMonitor::PathEvent event = PathMonitor::PathEvent::None;

    switch (action)
    {
        case FILE_ACTION_ADDED:
        case FILE_ACTION_RENAMED_NEW_NAME:
            event = PathMonitor::PathEvent::Created;
            break;

        case FILE_ACTION_REMOVED:
        case FILE_ACTION_RENAMED_OLD_NAME:
            event = PathMonitor::PathEvent::Deleted;
            break;

        case FILE_ACTION_MODIFIED:
            event = PathMonitor::PathEvent::Changed;
            break;

        default:
            break;
    }

    return event;
}

//==============================================================================
PathMonitorImpl::PathInfoImpl::PathInfoImpl(
    HANDLE iocp,
    const std::filesystem::path &path) noexcept :
    PathMonitorImpl::PathInfo(),
    m_valid(false),
    m_handle(INVALID_HANDLE_VALUE),
    m_pInfo(new FILE_NOTIFY_INFORMATION[s_buffSize])
{
    ::memset(&m_overlapped, 0, sizeof(m_overlapped));

    m_handle = ::CreateFile(
        path.string().c_str(),
        s_accessFlags,
        s_shareFlags,
        NULL,
        s_dispositionFlags,
        s_attributeFlags,
        NULL);

    if (m_handle == INVALID_HANDLE_VALUE)
    {
        LOGS("Could not create file for \"%s\"", path);
        return;
    }

    HANDLE port =
        ::CreateIoCompletionPort(m_handle, iocp, (ULONG_PTR)m_handle, 0);

    if (port == NULL)
    {
        LOGS("Could not create IOCP info for \"%s\"", path);
        return;
    }

    m_valid = Refresh(path);
}

//==============================================================================
PathMonitorImpl::PathInfoImpl::~PathInfoImpl()
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
bool PathMonitorImpl::PathInfoImpl::IsValid() const noexcept
{
    return m_valid && (m_handle != INVALID_HANDLE_VALUE);
}

//==============================================================================
bool PathMonitorImpl::PathInfoImpl::Refresh(
    const std::filesystem::path &path) noexcept
{
    static const DWORD size = (s_buffSize * sizeof(FILE_NOTIFY_INFORMATION));
    DWORD bytes = 0;

    BOOL success = ::ReadDirectoryChangesW(
        m_handle,
        m_pInfo,
        size,
        FALSE,
        s_changeFlags,
        &bytes,
        &m_overlapped,
        NULL);

    if (success == FALSE)
    {
        LOGS("Could not check events for \"%s\"", path);
    }

    return success == TRUE;
}

} // namespace fly

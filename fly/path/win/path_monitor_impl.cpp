#include "fly/path/win/path_monitor_impl.hpp"

#include "fly/logger/logger.hpp"
#include "fly/task/task_runner.hpp"
#include "fly/types/numeric/literals.hpp"

#include <string>

namespace fly {

namespace {

    const DWORD s_access_flags = FILE_LIST_DIRECTORY;

    const DWORD s_share_flags = FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE;

    const DWORD s_disposition_flags = OPEN_EXISTING;

    const DWORD s_attribute_flags = FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED;

    const DWORD s_change_flags = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
        FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;

    const DWORD s_buff_size = 100;

} // namespace

//==================================================================================================
PathMonitorImpl::PathMonitorImpl(
    const std::shared_ptr<SequencedTaskRunner> &task_runner,
    const std::shared_ptr<PathConfig> &config) noexcept :
    PathMonitor(task_runner, config),
    m_iocp(::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0))
{
    if (m_iocp == nullptr)
    {
        LOGS("Could not initialize IOCP");
    }
}

//==================================================================================================
PathMonitorImpl::~PathMonitorImpl()
{
    if (m_iocp != nullptr)
    {
        ::CloseHandle(m_iocp);
        m_iocp = nullptr;
    }
}

//==================================================================================================
bool PathMonitorImpl::is_valid() const noexcept
{
    return m_iocp != nullptr;
}

//==================================================================================================
void PathMonitorImpl::poll(const std::chrono::milliseconds &timeout) noexcept
{
    DWORD bytes = 0;
    ULONG_PTR key = 0;
    LPOVERLAPPED overlapped = nullptr;
    DWORD delay = static_cast<DWORD>(timeout.count());

    // Hold onto the path to be removed until the mutex is released
    std::filesystem::path path_to_remove;

    if (::GetQueuedCompletionStatus(m_iocp, &bytes, &key, &overlapped, delay))
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = std::find_if(
            m_path_info.begin(),
            m_path_info.end(),
            [&key](const PathInfoMap::value_type &value) -> bool {
                const auto *info = static_cast<PathInfoImpl *>(value.second.get());
                return ((ULONG_PTR)(info->m_handle)) == key;
            });

        if (it != m_path_info.end())
        {
            auto *info = static_cast<PathInfoImpl *>(it->second.get());
            handle_events(info, it->first);

            if (!info->refresh(it->first))
            {
                path_to_remove = it->first;
            }
        }
    }

    if (!path_to_remove.empty())
    {
        remove_path(path_to_remove);
    }
}

//==================================================================================================
std::unique_ptr<PathMonitor::PathInfo>
PathMonitorImpl::create_path_info(const std::filesystem::path &path) const noexcept
{
    std::unique_ptr<PathMonitor::PathInfo> info;

    if (is_valid())
    {
        info = std::make_unique<PathInfoImpl>(m_iocp, path);
    }

    return info;
}

//==================================================================================================
void PathMonitorImpl::handle_events(const PathInfoImpl *info, const std::filesystem::path &path)
    const noexcept
{
    PFILE_NOTIFY_INFORMATION file_info = info->m_file_info;

    while (file_info != nullptr)
    {
        PathMonitor::PathEvent path_event = convert_to_event(file_info->Action);

        if (path_event != PathMonitor::PathEvent::None)
        {
            const std::wstring wide_file(
                file_info->FileName,
                file_info->FileNameLength / sizeof(wchar_t));

            const std::filesystem::path file(wide_file);

            auto it = info->m_file_handlers.find(file);
            PathEventCallback callback = nullptr;

            if (it == info->m_file_handlers.end())
            {
                callback = info->m_path_handler;
            }
            else
            {
                callback = it->second;
            }

            if (callback != nullptr)
            {
                auto full = path / file;

                LOGI("Handling event %d for %s", path_event, full);
                callback(full, path_event);
            }
        }

        if (file_info->NextEntryOffset == 0_u64)
        {
            file_info = nullptr;
        }
        else
        {
            file_info = reinterpret_cast<PFILE_NOTIFY_INFORMATION>(
                reinterpret_cast<LPBYTE>(file_info) + file_info->NextEntryOffset);
        }
    }
}

//==================================================================================================
PathMonitor::PathEvent PathMonitorImpl::convert_to_event(DWORD action) const noexcept
{
    PathMonitor::PathEvent path_event = PathMonitor::PathEvent::None;

    switch (action)
    {
        case FILE_ACTION_ADDED:
        case FILE_ACTION_RENAMED_NEW_NAME:
            path_event = PathMonitor::PathEvent::Created;
            break;

        case FILE_ACTION_REMOVED:
        case FILE_ACTION_RENAMED_OLD_NAME:
            path_event = PathMonitor::PathEvent::Deleted;
            break;

        case FILE_ACTION_MODIFIED:
            path_event = PathMonitor::PathEvent::Changed;
            break;

        default:
            break;
    }

    return path_event;
}

//==================================================================================================
PathMonitorImpl::PathInfoImpl::PathInfoImpl(HANDLE iocp, const std::filesystem::path &path) noexcept
    :
    PathMonitorImpl::PathInfo(),
    m_valid(false),
    m_handle(INVALID_HANDLE_VALUE),
    m_file_info(new FILE_NOTIFY_INFORMATION[s_buff_size])
{
    ::memset(&m_overlapped, 0, sizeof(m_overlapped));

    m_handle = ::CreateFile(
        path.string().c_str(),
        s_access_flags,
        s_share_flags,
        nullptr,
        s_disposition_flags,
        s_attribute_flags,
        nullptr);

    if (m_handle == INVALID_HANDLE_VALUE)
    {
        LOGS("Could not create file for \"%s\"", path);
        return;
    }

    HANDLE port = ::CreateIoCompletionPort(m_handle, iocp, (ULONG_PTR)m_handle, 0);

    if (port == nullptr)
    {
        LOGS("Could not create IOCP info for \"%s\"", path);
        return;
    }

    m_valid = refresh(path);
}

//==================================================================================================
PathMonitorImpl::PathInfoImpl::~PathInfoImpl()
{
    if (m_file_info != nullptr)
    {
        delete[] m_file_info;
        m_file_info = nullptr;
    }

    if (m_handle != INVALID_HANDLE_VALUE)
    {
        ::CancelIo(m_handle);
        ::CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }
}

//==================================================================================================
bool PathMonitorImpl::PathInfoImpl::is_valid() const noexcept
{
    return m_valid && (m_handle != INVALID_HANDLE_VALUE);
}

//==================================================================================================
bool PathMonitorImpl::PathInfoImpl::refresh(const std::filesystem::path &path) noexcept
{
    static const DWORD size = (s_buff_size * sizeof(FILE_NOTIFY_INFORMATION));
    DWORD bytes = 0;

    BOOL success = ::ReadDirectoryChangesW(
        m_handle,
        m_file_info,
        size,
        FALSE,
        s_change_flags,
        &bytes,
        &m_overlapped,
        nullptr);

    if (success == FALSE)
    {
        LOGS("Could not check events for \"%s\"", path);
    }

    return success == TRUE;
}

} // namespace fly

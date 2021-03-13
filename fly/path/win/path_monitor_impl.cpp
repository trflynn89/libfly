#include "fly/path/win/path_monitor_impl.hpp"

#include "fly/logger/logger.hpp"
#include "fly/task/task_runner.hpp"
#include "fly/types/numeric/literals.hpp"

#include <string>

namespace fly {

namespace {

    constexpr const DWORD s_access_flags = FILE_LIST_DIRECTORY;

    constexpr const DWORD s_share_flags = FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE;

    constexpr const DWORD s_disposition_flags = OPEN_EXISTING;

    constexpr const DWORD s_attribute_flags = FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED;

    constexpr const DWORD s_change_flags = FILE_NOTIFY_CHANGE_FILE_NAME |
        FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;

} // namespace

//==================================================================================================
PathMonitorImpl::PathMonitorImpl(
    std::shared_ptr<fly::task::SequencedTaskRunner> task_runner,
    std::shared_ptr<PathConfig> config) noexcept :
    PathMonitor(std::move(task_runner), std::move(config))
{
}

//==================================================================================================
bool PathMonitorImpl::is_valid() const
{
    return true;
}

//==================================================================================================
void PathMonitorImpl::poll(std::chrono::milliseconds timeout)
{
    // Hold onto the paths to be removed until the mutex is released.
    std::vector<std::filesystem::path> paths_to_remove;
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        for (const PathInfoMap::value_type &value : m_path_info)
        {
            auto *info = static_cast<PathInfoImpl *>(value.second.get());
            DWORD bytes = 0;

            if (::GetOverlappedResult(info->m_handle, &info->m_overlapped, &bytes, FALSE))
            {
                handle_events(info, value.first);

                if (!info->refresh(value.first))
                {
                    paths_to_remove.push_back(value.first);
                }
            }
        }
    }

    for (const auto &path_to_remove : paths_to_remove)
    {
        remove_path(path_to_remove);
    }

    std::this_thread::sleep_for(timeout);
}

//==================================================================================================
std::unique_ptr<PathMonitor::PathInfo>
PathMonitorImpl::create_path_info(const std::filesystem::path &path) const
{
    return std::make_unique<PathInfoImpl>(path);
}

//==================================================================================================
void PathMonitorImpl::handle_events(const PathInfoImpl *info, const std::filesystem::path &path)
    const
{
    const FILE_NOTIFY_INFORMATION *file_info = info->m_file_info.data();

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
                auto full_path = path / file;

                LOGI("Handling event {} for {}", path_event, full_path);
                std::invoke(std::move(callback), std::move(full_path), path_event);
            }
        }

        if (file_info->NextEntryOffset == 0_u64)
        {
            file_info = nullptr;
        }
        else
        {
            file_info = reinterpret_cast<const FILE_NOTIFY_INFORMATION *>(
                reinterpret_cast<const BYTE *>(file_info) + file_info->NextEntryOffset);
        }
    }
}

//==================================================================================================
PathMonitor::PathEvent PathMonitorImpl::convert_to_event(DWORD action) const
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
PathMonitorImpl::PathInfoImpl::PathInfoImpl(const std::filesystem::path &path) noexcept
{
    m_handle = ::CreateFileW(
        path.wstring().c_str(),
        s_access_flags,
        s_share_flags,
        nullptr,
        s_disposition_flags,
        s_attribute_flags,
        nullptr);

    if (m_handle == INVALID_HANDLE_VALUE)
    {
        LOGS("Could not create file for \"{}\"", path);
        return;
    }

    m_valid = refresh(path);
}

//==================================================================================================
PathMonitorImpl::PathInfoImpl::~PathInfoImpl()
{
    if (m_handle != INVALID_HANDLE_VALUE)
    {
        ::CancelIo(m_handle);
        ::CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }
}

//==================================================================================================
bool PathMonitorImpl::PathInfoImpl::is_valid() const
{
    return m_valid && (m_handle != INVALID_HANDLE_VALUE);
}

//==================================================================================================
bool PathMonitorImpl::PathInfoImpl::refresh(const std::filesystem::path &path)
{
    static const auto s_file_info_size =
        static_cast<DWORD>(m_file_info.size() * sizeof(FILE_NOTIFY_INFORMATION));

    BOOL success = ::ReadDirectoryChangesW(
        m_handle,
        m_file_info.data(),
        s_file_info_size,
        FALSE,
        s_change_flags,
        nullptr,
        &m_overlapped,
        nullptr);

    if (success == FALSE)
    {
        LOGS("Could not check events for \"{}\"", path);
    }

    return success == TRUE;
}

} // namespace fly

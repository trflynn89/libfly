#include "win_file_monitor.h"

#include <fly/logging/logger.h>
#include <fly/system/system.h>

namespace fly {

namespace
{
    static const DWORD s_accessFlags = (
        FILE_LIST_DIRECTORY |
        FILE_READ_ATTRIBUTES
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
        FILE_NOTIFY_CHANGE_SIZE |
        FILE_NOTIFY_CHANGE_LAST_WRITE |
        FILE_NOTIFY_CHANGE_CREATION
    );
}

//==============================================================================
FileMonitorImpl::FileMonitorImpl(
    FileEventCallback handler,
    const std::string &path,
    const std::string &file
) :
    FileMonitor(handler, path, file),
    m_monitorHandle(INVALID_HANDLE_VALUE)
{
    m_overlapped.hEvent = INVALID_HANDLE_VALUE;

    m_monitorHandle = ::CreateFile(
        m_path.c_str(), s_accessFlags, s_shareFlags, NULL, s_dispositionFlags,
        s_attributeFlags, NULL
    );

    if (m_monitorHandle == INVALID_HANDLE_VALUE)
    {
        LOGW(-1, "Could not initialize monitor for \"%s\": %s",
            m_path, fly::System::GetLastError()
        );
    }
    else
    {
        m_overlapped.hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

        if (m_overlapped.hEvent == INVALID_HANDLE_VALUE)
        {
            LOGW(-1, "Could not create overlapped for \"%s\": %s",
                m_path, fly::System::GetLastError()
            );
        }
    }
}

//==============================================================================
FileMonitorImpl::~FileMonitorImpl()
{
    close();
}

//==============================================================================
bool FileMonitorImpl::IsValid() const
{
    return (m_overlapped.hEvent != INVALID_HANDLE_VALUE);
}

//==============================================================================
void FileMonitorImpl::Poll(const std::chrono::milliseconds &timeout)
{
    static const DWORD buffSize = (8 << 10);
    PBYTE buff = new BYTE[buffSize];

    DWORD bytes = 0;

    std::string errorStr;
    int error = 0;

    BOOL success = ::ReadDirectoryChangesW(
        m_monitorHandle, buff, buffSize, FALSE, s_changeFlags, &bytes,
        &m_overlapped, NULL
    );

    if (success == TRUE)
    {
        DWORD millis = static_cast<DWORD>(timeout.count());

        success = ::GetOverlappedResultEx(
            m_monitorHandle, &m_overlapped, &bytes, millis, FALSE
        );

        errorStr = fly::System::GetLastError(&error);

        if ((success == FALSE) && (error == WAIT_TIMEOUT))
        {
            success = ::CancelIoEx(m_monitorHandle, &m_overlapped);
            errorStr = fly::System::GetLastError(&error);

            if ((success == TRUE) || (error != ERROR_NOT_FOUND))
            {
                success = ::GetOverlappedResult(
                    m_monitorHandle, &m_overlapped, &bytes, TRUE
                );

                errorStr = fly::System::GetLastError(&error);
            }
        }
    }

    if (success == TRUE)
    {
        handleEvents(buff);
    }
    else if ((error != WAIT_TIMEOUT) && (error != ERROR_OPERATION_ABORTED))
    {
        LOGW(-1, "Could not check events for \"%s\": %s", m_path, errorStr);
        close();
    }

    delete[] buff;
}

//==============================================================================
void FileMonitorImpl::handleEvents(PBYTE pBuffer)
{
    PFILE_NOTIFY_INFORMATION info = reinterpret_cast<PFILE_NOTIFY_INFORMATION>(pBuffer);

    do {
        std::wstring wFileName(info->FileName, info->FileNameLength / sizeof(wchar_t));
        std::string fileName(wFileName.begin(), wFileName.end());

        if (fileName.compare(m_file) == 0)
        {
            HandleEvent(convertToEvent(info->Action));
        }

        info = reinterpret_cast<PFILE_NOTIFY_INFORMATION>(
            reinterpret_cast<PBYTE >(info) + info->NextEntryOffset
        );
    } while (info->NextEntryOffset > 0);
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

//==============================================================================
void FileMonitorImpl::close()
{
    if (m_monitorHandle != INVALID_HANDLE_VALUE)
    {
        if (m_overlapped.hEvent != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(m_overlapped.hEvent);
            m_overlapped.hEvent = INVALID_HANDLE_VALUE;
        }

        ::CloseHandle(m_monitorHandle);
        m_monitorHandle = INVALID_HANDLE_VALUE;
    }
}

}

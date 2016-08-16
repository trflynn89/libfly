#pragma once

#include <chrono>
#include <string>

#include <Windows.h>

#include <fly/fly.h>
#include <fly/file/file_monitor.h>

namespace fly {

/**
 * Windows implementation of the FileMonitor interface.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 21, 2016
 */
class FileMonitorImpl : public FileMonitor
{
public:
    FileMonitorImpl(FileEventCallback, const std::string &, const std::string &);
    virtual ~FileMonitorImpl();
    bool IsValid() const;

protected:
    void Poll(const std::chrono::milliseconds &);

private:
    void handleEvents(PBYTE);
    FileMonitor::FileEvent convertToEvent(DWORD);
    void close();

    OVERLAPPED m_overlapped;
    HANDLE m_monitorHandle;
};

}

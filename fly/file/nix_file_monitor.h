#pragma once

#include <chrono>
#include <poll.h>
#include <string>

#include <fly/file/file_monitor.h>

namespace fly {

/**
 * Linux implementation of the FileMonitor interface.
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
    bool handleEvents();
    FileMonitor::FileEvent convertToEvent(int);
    void close();

    int m_monitorDescriptor;
    int m_watchDescriptor;
};

}

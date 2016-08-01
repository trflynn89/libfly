#include "file_monitor.h"

#include <fly/logging/logger.h>

namespace fly {

namespace
{
    static const std::chrono::milliseconds s_pollTimeout(1000);
}

//==============================================================================
FileMonitor::FileMonitor(
    FileEventCallback handler,
    const std::string &path,
    const std::string &file
) :
    Runner("FileMonitor", 1),
    m_path(path),
    m_file(file),
    m_handler(handler),
    m_interval(0)
{
}

//==============================================================================
FileMonitor::~FileMonitor()
{
}

//==============================================================================
bool FileMonitor::StartRunner()
{
    return IsValid();
}

//==============================================================================
void FileMonitor::StopRunner()
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_handler = nullptr;
}

//==============================================================================
bool FileMonitor::DoWork()
{
    static const std::chrono::seconds noWait(std::chrono::seconds::zero());
    static const size_t updateInterval(5);

    if (IsValid())
    {
        Poll(s_pollTimeout);

        if ((++m_interval % updateInterval) == 0)
        {
            FileEvent event = FileMonitor::FILE_NO_CHANGE;
            m_eventStack.Pop(event, noWait, true);

            if (event != FileMonitor::FILE_NO_CHANGE)
            {
                std::lock_guard<std::mutex> lock(m_callbackMutex);

                if (m_handler != nullptr)
                {
                    LOGI(-1, "Handling event %d for \"%s\"", event, m_file);
                    m_handler(event);
                }
            }
        }
    }

    return IsValid();
}

//==============================================================================
void FileMonitor::HandleEvent(FileEvent event)
{
    if (event != FileMonitor::FILE_NO_CHANGE)
    {
        m_eventStack.Push(event);
    }
}

}

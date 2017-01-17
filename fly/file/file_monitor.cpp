#include "file_monitor.h"

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

#pragma once

#include <chrono>
#include <string>
#include <sys/inotify.h>

#include <fly/file/file_monitor.h>

namespace fly {

DEFINE_CLASS_PTRS(FileMonitorImpl);

/**
 * Linux implementation of the FileMonitor interface. Uses the inotify API to
 * detect path changes.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version January 19, 2017
 */
class FileMonitorImpl : public FileMonitor
{
public:
    FileMonitorImpl();
    virtual ~FileMonitorImpl();

    /**
     * Check if the file monitor's inotify handle was successfully created.
     *
     * @return bool True if the handle is valid.
     */
    virtual bool IsValid() const;

protected:
    virtual FileMonitor::PathInfoPtr CreatePathInfo(const std::string &);
    virtual void Poll(const std::chrono::milliseconds &);
    virtual void Close();

private:
    DEFINE_STRUCT_PTRS(PathInfoImpl);

    /**
     * Linux implementation of the PathInfo interface. Stores the file monitor's
     * inotify handle for adding/removing the monitored path, as well as a
     * handle to the added inotify watch.
     */
    struct PathInfoImpl : FileMonitor::PathInfo
    {
        PathInfoImpl(int, const std::string &);
        virtual ~PathInfoImpl();

        /**
         * @return bool True if initialization was sucessful.
         */
        virtual bool IsValid() const;

        int m_monitorDescriptor;
        int m_watchDescriptor;
    };

    /**
     * Read the inotify file monitor handle for any events.
     *
     * @return bool True if any events were read.
     */
    bool readEvents() const;

    /**
     * Handle a single inotify event. Find a monitored file that corresponds
     * to the event and trigger its callback. If no file was found, drop the
     * event.
     */
    void handleEvent(const struct inotify_event *) const;

    /**
     * Convert an inotify event mask to a FileEvent.
     *
     * @param int The inotify event mask.
     *
     * @return FileEvent A FileEvent that matches the event mask.
     */
    FileMonitor::FileEvent convertToEvent(int) const;

    int m_monitorDescriptor;
};

}

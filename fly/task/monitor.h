#pragma once

#include <chrono>

#include "fly/fly.h"
#include "fly/task/runner.h"

namespace fly {

FLY_CLASS_PTRS(ConfigManager);
FLY_CLASS_PTRS(Monitor);
FLY_CLASS_PTRS(TaskConfig);

/**
 * Helper class to simplify creating a task to do some poll-based monitoring.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version September 17, 2017
 */
class Monitor : public Runner
{
public:
    /**
     * Constructor.
     *
     * @param string Name to use for this monitor.
     * @param ConfigManagerPtr Reference to the configuration manager.
     */
    Monitor(const std::string &, ConfigManagerPtr &);

    /**
     * Destructor.
     */
    virtual ~Monitor();

protected:
    /**
     * Start the monitor.
     */
    virtual void StartMonitor() = 0;

    /**
     * Stop the monitor.
     */
    virtual void StopMonitor() = 0;

    /**
     * Check if the monitor implementation is in a good state.
     *
     * @return bool True if the monitor is healthy.
     */
    virtual bool IsValid() const = 0;

    /**
     * Run one interation of the monitor.
     *
     * @param milliseconds Time to sleep between poll intervals.
     */
    virtual void Poll(const std::chrono::milliseconds &) = 0;

    /**
     * Start the monitor.
     *
     * @return bool True.
     */
    virtual bool StartRunner();

    /**
     * Stop the monitor.
     */
    virtual void StopRunner();

    /**
     * Run one iteration of the monitor if it is in a good state.
     *
     * @return bool True if the monitor is healthy.
     */
    virtual bool DoWork();

private:
    TaskConfigPtr m_spConfig;
};

}

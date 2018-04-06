#pragma once

#include <atomic>
#include <future>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "fly/fly.h"

namespace fly {

FLY_CLASS_PTRS(Runner);

/**
 * Class to simplify running tasks. Other classes may inherit from this class
 * to setup any number of worker threads, which repeatedly do some work until
 * they fail a health check or are explicitly asked to stop.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 21, 2016
 */
class Runner : public std::enable_shared_from_this<Runner>
{
public:
    /**
     * Constructor.
     *
     * @param string Name to use for this task runner.
     * @param int Number of worker threads to start.
     */
    Runner(const std::string &, int);

    /**
     * Destructor. Stop the running tasks if necessary.
     */
    virtual ~Runner();

    /**
     * Initialize the task and start the configured number of worker threads.
     */
    bool Start();

    /**
     * Deinitialize the task and stop the worker threads.
     */
    void Stop();

protected:
    /**
     * Classes which inherit from this class should implement this method to
     * perform any initialization which is required before the worker threads
     * can begin.
     *
     * @return True if the task could be initialized.
     */
    virtual bool StartRunner() = 0;

    /**
     * Classes which inherit from this class should implement this method to
     * perform any deinitialization which is required after the worker threads
     * have terminated.
     */
    virtual void StopRunner() = 0;

    /**
     * Classes which inherit from this class should implement this method to
     * perform the work required by the worker threads.
     *
     * @return True if the task is in a healthy state.
     */
    virtual bool DoWork() = 0;

    /**
     * Helper to downcast Runner's shared_from_this() value to the given type.
     *
     * @tparam T Type to downcast to.
     *
     * @return The downcast shared pointer to this object.
     */
    template <typename T>
    std::shared_ptr<T> SharedFromThis();

private:
    /**
     * Thread to perform the work requried for this task.
     */
    void workerThread();

    std::atomic_bool m_aKeepRunning;

    std::vector<std::future<void>> m_futures;

    const std::string m_name;

    int m_numWorkers;
};

//==============================================================================
template <typename T>
std::shared_ptr<T> Runner::SharedFromThis()
{
    return std::dynamic_pointer_cast<T>(shared_from_this());
}

}

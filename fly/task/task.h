#pragma once

namespace fly {

class TaskManager;

/**
 * Virtual class to represent a task to be run by the task system.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version August 12, 2018
 */
class Task
{
    friend class TaskManager;

public:
    /**
     * Destructor.
     */
    virtual ~Task() = default;

protected:
    /**
     * Classes which inherit from this class should implement this method to
     * perform the work required by the task.
     */
    virtual void Run() noexcept = 0;
};

} // namespace fly

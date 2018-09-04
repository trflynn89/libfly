#include "fly/task/task_manager.h"

#include <thread>

#include "fly/logger/logger.h"
#include "fly/task/task.h"
#include "fly/task/task_runner.h"

namespace fly {

//==============================================================================
TaskManager::TaskManager(int numWorkers) :
    m_aKeepRunning(false),
    m_numWorkers(numWorkers)
{
}

//==============================================================================
bool TaskManager::Start()
{
    bool expected = false;

    if (m_aKeepRunning.compare_exchange_strong(expected, true))
    {
        LOGI(-1, "Starting %d workers", m_numWorkers);
        TaskManagerPtr spTaskManager = shared_from_this();

        for (int i = 0; i < m_numWorkers; ++i)
        {
            m_futures.push_back(std::async(
                std::launch::async, &TaskManager::workerThread, spTaskManager
            ));
        }

        m_futures.push_back(std::async(
            std::launch::async, &TaskManager::timerThread, spTaskManager
        ));

        return true;
    }

    return false;
}

//==============================================================================
bool TaskManager::Stop()
{
    bool expected = true;

    if (m_aKeepRunning.compare_exchange_strong(expected, false))
    {
        LOGI(-1, "Stopping %d workers", m_numWorkers);

        for (auto &future : m_futures)
        {
            if (future.valid())
            {
                future.get();
            }
        }

        return true;
    }

    return false;
}

//==============================================================================
void TaskManager::postTask(
    const TaskWPtr &wpTask,
    const TaskRunnerWPtr &wpTaskRunner
)
{
    TaskHolder task { wpTask, wpTaskRunner, std::chrono::steady_clock::now() };
    m_tasks.Push(task);
}

//==============================================================================
void TaskManager::postTaskWithDelay(
    const TaskWPtr &wpTask,
    const TaskRunnerWPtr &wpTaskRunner,
    std::chrono::milliseconds delay
)
{
    auto schedule = std::chrono::steady_clock::now() + delay;
    TaskHolder task { wpTask, wpTaskRunner, schedule };

    std::unique_lock<std::mutex> lock(m_delayedTasksMutex);
    m_delayedTasks.push_back(task);
}

//==============================================================================
void TaskManager::workerThread()
{
    static std::chrono::milliseconds wait(100);
    TaskHolder task;

    while (m_aKeepRunning.load())
    {
        if (m_tasks.Pop(task, wait) && m_aKeepRunning.load())
        {
            TaskRunnerPtr spTaskRunner = task.m_wpTaskRunner.lock();
            TaskPtr spTask = task.m_wpTask.lock();

            if (spTaskRunner)
            {
                if (spTask)
                {
                    spTask->Run();
                }

                spTaskRunner->TaskComplete(spTask);
            }
        }
    }
}

//==============================================================================
void TaskManager::timerThread()
{
    static std::chrono::milliseconds delay(100);

    while (m_aKeepRunning.load())
    {
        auto now = std::chrono::steady_clock::now();
        {
            std::unique_lock<std::mutex> lock(m_delayedTasksMutex);

            for (auto it = m_delayedTasks.begin(); it != m_delayedTasks.end(); )
            {
                if (it->m_schedule <= now)
                {
                    TaskRunnerPtr spTaskRunner = it->m_wpTaskRunner.lock();

                    if (spTaskRunner)
                    {
                        spTaskRunner->PostTask(it->m_wpTask);
                    }

                    it = m_delayedTasks.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }

        std::this_thread::sleep_for(delay);
    }
}

}

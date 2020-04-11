#include "fly/task/task_manager.hpp"

#include "fly/logger/logger.hpp"
#include "fly/task/task.hpp"
#include "fly/task/task_runner.hpp"

#include <thread>

namespace fly {

namespace {

    const std::chrono::milliseconds s_delay(10);

} // namespace

//==============================================================================
TaskManager::TaskManager(int numWorkers) noexcept :
    m_aKeepRunning(false),
    m_numWorkers(numWorkers)
{
}

//==============================================================================
bool TaskManager::Start() noexcept
{
    bool expected = false;

    if (m_aKeepRunning.compare_exchange_strong(expected, true))
    {
        LOGI("Starting %d workers", m_numWorkers);
        std::shared_ptr<TaskManager> spTaskManager = shared_from_this();

        for (int i = 0; i < m_numWorkers; ++i)
        {
            m_futures.push_back(std::async(
                std::launch::async,
                &TaskManager::workerThread,
                spTaskManager));
        }

        m_futures.push_back(std::async(
            std::launch::async,
            &TaskManager::timerThread,
            spTaskManager));

        return true;
    }

    return false;
}

//==============================================================================
bool TaskManager::Stop() noexcept
{
    bool expected = true;

    if (m_aKeepRunning.compare_exchange_strong(expected, false))
    {
        LOGI("Stopping %d workers", m_numWorkers);

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
    std::weak_ptr<Task> wpTask,
    std::weak_ptr<TaskRunner> wpTaskRunner) noexcept
{
    TaskHolder task {wpTask, wpTaskRunner, std::chrono::steady_clock::now()};
    m_tasks.Push(std::move(task));
}

//==============================================================================
void TaskManager::postTaskWithDelay(
    std::weak_ptr<Task> wpTask,
    std::weak_ptr<TaskRunner> wpTaskRunner,
    std::chrono::milliseconds delay) noexcept
{
    auto schedule = std::chrono::steady_clock::now() + delay;
    TaskHolder task {wpTask, wpTaskRunner, schedule};

    std::unique_lock<std::mutex> lock(m_delayedTasksMutex);
    m_delayedTasks.push_back(std::move(task));
}

//==============================================================================
void TaskManager::workerThread() noexcept
{
    TaskHolder task;

    while (m_aKeepRunning.load())
    {
        if (m_tasks.Pop(task, s_delay) && m_aKeepRunning.load())
        {
            auto spTaskRunner = task.m_wpTaskRunner.lock();
            auto spTask = task.m_wpTask.lock();

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
void TaskManager::timerThread() noexcept
{
    while (m_aKeepRunning.load())
    {
        auto now = std::chrono::steady_clock::now();
        {
            std::unique_lock<std::mutex> lock(m_delayedTasksMutex);

            for (auto it = m_delayedTasks.begin(); it != m_delayedTasks.end();)
            {
                if (it->m_schedule <= now)
                {
                    auto spTaskRunner = it->m_wpTaskRunner.lock();

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

        std::this_thread::sleep_for(s_delay);
    }
}

} // namespace fly

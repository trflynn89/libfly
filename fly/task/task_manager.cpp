#include "fly/task/task_manager.hpp"

#include "fly/logger/logger.hpp"
#include "fly/task/task.hpp"
#include "fly/task/task_runner.hpp"

#include <thread>

namespace fly {

namespace {

    const std::chrono::milliseconds s_delay(10);

} // namespace

//==================================================================================================
TaskManager::TaskManager(std::uint32_t num_workers) noexcept :
    m_keep_running(false),
    m_num_workers(num_workers)
{
}

//==================================================================================================
bool TaskManager::start()
{
    bool expected = false;

    if (m_keep_running.compare_exchange_strong(expected, true))
    {
        LOGI("Starting %d workers", m_num_workers);
        std::shared_ptr<TaskManager> task_manager = shared_from_this();

        for (std::uint32_t i = 0; i < m_num_workers; ++i)
        {
            m_futures.push_back(
                std::async(std::launch::async, &TaskManager::worker_thread, task_manager));
        }

        m_futures.push_back(
            std::async(std::launch::async, &TaskManager::timer_thread, task_manager));

        return true;
    }

    return false;
}

//==================================================================================================
bool TaskManager::stop()
{
    bool expected = true;

    if (m_keep_running.compare_exchange_strong(expected, false))
    {
        LOGI("Stopping %d workers", m_num_workers);

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

//==================================================================================================
void TaskManager::post_task(
    std::weak_ptr<Task> weak_task,
    std::weak_ptr<TaskRunner> weak_task_runner)
{
    const auto now = std::chrono::steady_clock::now();
    TaskHolder task {weak_task, weak_task_runner, now};

    m_tasks.push(std::move(task));
}

//==================================================================================================
void TaskManager::post_task_with_delay(
    std::weak_ptr<Task> weak_task,
    std::weak_ptr<TaskRunner> weak_task_runner,
    std::chrono::milliseconds delay)
{
    const auto schedule = std::chrono::steady_clock::now() + delay;
    TaskHolder task {weak_task, weak_task_runner, schedule};

    std::unique_lock<std::mutex> lock(m_delayed_tasks_mutex);
    m_delayed_tasks.push_back(std::move(task));
}

//==================================================================================================
void TaskManager::worker_thread()
{
    TaskHolder task_holder;

    while (m_keep_running.load())
    {
        if (m_tasks.pop(task_holder, s_delay) && m_keep_running.load())
        {
            auto task_runner = task_holder.m_weak_task_runner.lock();
            auto task = task_holder.m_weak_task.lock();

            if (task_runner)
            {
                if (task)
                {
                    task->run();
                }

                task_runner->task_complete(task);
            }
        }
    }
}

//==================================================================================================
void TaskManager::timer_thread()
{
    while (m_keep_running.load())
    {
        auto now = std::chrono::steady_clock::now();
        {
            std::unique_lock<std::mutex> lock(m_delayed_tasks_mutex);

            for (auto it = m_delayed_tasks.begin(); it != m_delayed_tasks.end();)
            {
                if (it->m_schedule <= now)
                {
                    auto task_runner = it->m_weak_task_runner.lock();

                    if (task_runner)
                    {
                        task_runner->post_task(it->m_weak_task);
                    }

                    it = m_delayed_tasks.erase(it);
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

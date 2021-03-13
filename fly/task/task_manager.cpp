#include "fly/task/task_manager.hpp"

#include "fly/task/task_runner.hpp"

#include <thread>

namespace fly::task {

namespace {

    const std::chrono::milliseconds s_delay(10);

} // namespace

//==================================================================================================
std::shared_ptr<TaskManager> TaskManager::create(std::uint32_t thread_count)
{
    // TaskManager has a private constructor, thus cannot be used with std::make_shared. This class
    // is used to expose the private constructor locally.
    struct TaskManagerImpl final : public TaskManager
    {
        explicit TaskManagerImpl(std::uint32_t thread_count) noexcept : TaskManager(thread_count)
        {
        }
    };

    auto task_manager = std::make_shared<TaskManagerImpl>(thread_count);
    return task_manager->start() ? task_manager : nullptr;
}

//==================================================================================================
TaskManager::TaskManager(std::uint32_t thread_count) noexcept :
    m_keep_running(false),
    m_thread_count(thread_count)
{
}

//==================================================================================================
bool TaskManager::start()
{
    bool expected = false;

    if (m_keep_running.compare_exchange_strong(expected, true))
    {
        std::shared_ptr<TaskManager> task_manager = shared_from_this();

        for (std::uint32_t i = 0; i < m_thread_count; ++i)
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
    TaskLocation &&location,
    Task &&task,
    std::weak_ptr<TaskRunner> weak_task_runner)
{
    TaskHolder wrapped_task {
        std::move(location),
        std::move(task),
        std::move(weak_task_runner),
        std::chrono::steady_clock::now()};

    m_tasks.push(std::move(wrapped_task));
}

//==================================================================================================
void TaskManager::post_task_with_delay(
    TaskLocation &&location,
    Task &&task,
    std::weak_ptr<TaskRunner> weak_task_runner,
    std::chrono::milliseconds delay)
{
    TaskHolder wrapped_task {
        std::move(location),
        std::move(task),
        std::move(weak_task_runner),
        std::chrono::steady_clock::now() + delay};

    std::unique_lock<std::mutex> lock(m_delayed_tasks_mutex);
    m_delayed_tasks.push_back(std::move(wrapped_task));
}

//==================================================================================================
void TaskManager::worker_thread()
{
    TaskHolder task_holder;

    while (m_keep_running.load())
    {
        if (m_tasks.pop(task_holder, s_delay) && m_keep_running.load())
        {
            if (auto task_runner = task_holder.m_weak_task_runner.lock(); task_runner)
            {
                TaskLocation location = std::move(task_holder.m_location);
                Task task = std::move(task_holder.m_task);

                task_runner->execute(std::move(location), std::move(task));
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
                    if (auto task_runner = it->m_weak_task_runner.lock(); task_runner)
                    {
                        TaskLocation location = std::move(it->m_location);
                        Task task = std::move(it->m_task);

                        task_runner->post_task_internal(std::move(location), std::move(task));
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

} // namespace fly::task

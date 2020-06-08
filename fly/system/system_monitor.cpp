#include "fly/system/system_monitor.hpp"

#include "fly/system/system_config.hpp"
#include "fly/task/task_runner.hpp"

#include <chrono>
#include <thread>

namespace fly {

//==================================================================================================
SystemMonitor::SystemMonitor(
    const std::shared_ptr<SequencedTaskRunner> &task_runner,
    const std::shared_ptr<SystemConfig> &config) noexcept :
    m_system_cpu_count(0),
    m_system_cpu_usage(0.0),
    m_process_cpu_usage(0.0),
    m_total_system_memory(0),
    m_system_memory_usage(0),
    m_process_memory_usage(0),
    m_task_runner(task_runner),
    m_config(config)
{
}

//==================================================================================================
bool SystemMonitor::start() noexcept
{
    update_system_cpu_count();

    if (is_valid())
    {
        std::shared_ptr<SystemMonitor> system_monitor = shared_from_this();

        m_task = std::make_shared<SystemMonitorTask>(system_monitor);
        m_task_runner->post_task(m_task);

        return true;
    }

    return false;
}

//==================================================================================================
std::uint32_t SystemMonitor::get_system_cpu_count() const noexcept
{
    return m_system_cpu_count.load();
}

//==================================================================================================
double SystemMonitor::get_system_cpu_usage() const noexcept
{
    return m_system_cpu_usage.load();
}

//==================================================================================================
double SystemMonitor::get_process_cpu_usage() const noexcept
{
    return m_process_cpu_usage.load();
}

//==================================================================================================
std::uint64_t SystemMonitor::get_total_system_memory() const noexcept
{
    return m_total_system_memory.load();
}

//==================================================================================================
std::uint64_t SystemMonitor::get_system_memory_usage() const noexcept
{
    return m_system_memory_usage.load();
}

//==================================================================================================
std::uint64_t SystemMonitor::get_process_memory_usage() const noexcept
{
    return m_process_memory_usage.load();
}

//==================================================================================================
bool SystemMonitor::is_valid() const noexcept
{
    return get_system_cpu_count() > 0;
}

//==================================================================================================
void SystemMonitor::poll() noexcept
{
    update_system_cpu_count();
    update_system_cpu_usage();
    update_process_cpu_usage();

    update_system_memory_usage();
    update_process_memory_usage();
}

//==================================================================================================
SystemMonitorTask::SystemMonitorTask(std::weak_ptr<SystemMonitor> weak_system_monitor) noexcept :
    Task(),
    m_weak_system_monitor(weak_system_monitor)
{
}

//==================================================================================================
void SystemMonitorTask::run() noexcept
{
    std::shared_ptr<SystemMonitor> system_monitor = m_weak_system_monitor.lock();

    if (system_monitor && system_monitor->is_valid())
    {
        system_monitor->poll();

        if (system_monitor->is_valid())
        {
            system_monitor->m_task_runner->post_task_with_delay(
                system_monitor->m_task,
                system_monitor->m_config->poll_interval());
        }
    }
}

} // namespace fly

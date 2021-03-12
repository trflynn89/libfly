#include "fly/system/system_monitor.hpp"

#include "fly/fly.hpp"
#include "fly/system/system_config.hpp"
#include "fly/task/task_runner.hpp"

#include <chrono>
#include <thread>

#include FLY_OS_IMPL_PATH(system, system_monitor)

namespace fly {

//==================================================================================================
std::shared_ptr<SystemMonitor> SystemMonitor::create(
    std::shared_ptr<SequencedTaskRunner> task_runner,
    std::shared_ptr<SystemConfig> config)
{
    auto system_monitor =
        std::make_shared<SystemMonitorImpl>(std::move(task_runner), std::move(config));
    return system_monitor->start() ? system_monitor : nullptr;
}

//==================================================================================================
SystemMonitor::SystemMonitor(
    std::shared_ptr<SequencedTaskRunner> task_runner,
    std::shared_ptr<SystemConfig> config) noexcept :
    m_task_runner(task_runner),
    m_config(config)
{
}

//==================================================================================================
bool SystemMonitor::start()
{
    update_system_cpu_count();
    return poll_system_later();
}

//==================================================================================================
std::uint32_t SystemMonitor::get_system_cpu_count() const
{
    return m_system_cpu_count.load();
}

//==================================================================================================
double SystemMonitor::get_system_cpu_usage() const
{
    return m_system_cpu_usage.load();
}

//==================================================================================================
double SystemMonitor::get_process_cpu_usage() const
{
    return m_process_cpu_usage.load();
}

//==================================================================================================
std::uint64_t SystemMonitor::get_total_system_memory() const
{
    return m_total_system_memory.load();
}

//==================================================================================================
std::uint64_t SystemMonitor::get_system_memory_usage() const
{
    return m_system_memory_usage.load();
}

//==================================================================================================
std::uint64_t SystemMonitor::get_process_memory_usage() const
{
    return m_process_memory_usage.load();
}

//==================================================================================================
bool SystemMonitor::is_valid() const
{
    return get_system_cpu_count() > 0;
}

//==================================================================================================
bool SystemMonitor::poll_system_later()
{
    if (!is_valid())
    {
        return false;
    }

    auto task = [](std::shared_ptr<SystemMonitor> self)
    {
        self->update_system_cpu_count();
        self->update_system_cpu_usage();
        self->update_process_cpu_usage();

        self->update_system_memory_usage();
        self->update_process_memory_usage();

        self->poll_system_later();
    };

    std::weak_ptr<SystemMonitor> weak_self = shared_from_this();
    return m_task_runner->post_task_with_delay(
        FROM_HERE,
        std::move(task),
        std::move(weak_self),
        m_config->poll_interval());
}

} // namespace fly

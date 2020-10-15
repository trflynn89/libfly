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

    std::weak_ptr<SystemMonitor> weak_self = shared_from_this();

    auto task = [weak_self]() {
        if (auto self = weak_self.lock(); self && self->is_valid())
        {
            self->update_system_cpu_count();
            self->update_system_cpu_usage();
            self->update_process_cpu_usage();

            self->update_system_memory_usage();
            self->update_process_memory_usage();

            self->poll_system_later();
        }
    };

    m_task_runner->post_task_with_delay(FROM_HERE, std::move(task), m_config->poll_interval());
    return true;
}

} // namespace fly

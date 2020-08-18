#include "fly/system/mac/system_monitor_impl.hpp"

#include "fly/system/mac/mach_api.h"
#include "fly/system/system_config.hpp"
#include "fly/task/task_runner.hpp"

#import <Foundation/Foundation.h>

namespace fly {

namespace {

    std::uint64_t time_value_to_microseconds(const time_value_t &time_value)
    {
        return (static_cast<std::uint64_t>(time_value.seconds) * 1'000'000) +
            static_cast<std::uint64_t>(time_value.microseconds);
    }

} // namespace

//==================================================================================================
SystemMonitorImpl::SystemMonitorImpl(
    const std::shared_ptr<SequencedTaskRunner> &task_runner,
    const std::shared_ptr<SystemConfig> &config) noexcept :
    SystemMonitor(task_runner, config)
{
}

//==================================================================================================
void SystemMonitorImpl::update_system_cpu_count()
{
    NSUInteger cpu_count = [[NSProcessInfo processInfo] processorCount];
    m_system_cpu_count.store(static_cast<std::uint32_t>(cpu_count));
}

//==================================================================================================
void SystemMonitorImpl::update_system_cpu_usage()
{
    host_cpu_load_info_data_t cpu_load {};
    if (!detail::host_cpu_load(cpu_load))
    {
        return;
    }

    const auto user = static_cast<std::uint64_t>(cpu_load.cpu_ticks[CPU_STATE_USER]);
    const auto system = static_cast<std::uint64_t>(cpu_load.cpu_ticks[CPU_STATE_SYSTEM]);
    const auto idle = static_cast<std::uint64_t>(cpu_load.cpu_ticks[CPU_STATE_IDLE]);
    const auto nice = static_cast<std::uint64_t>(cpu_load.cpu_ticks[CPU_STATE_NICE]);

    if ((user >= m_prev_system_user_time) && (system >= m_prev_system_system_time) &&
        (idle >= m_prev_system_idle_time) && (nice >= m_prev_system_nice_time))
    {
        const std::uint64_t active = (user - m_prev_system_user_time) +
            (system - m_prev_system_system_time) + (nice - m_prev_system_nice_time);
        const std::uint64_t total = active + (idle - m_prev_system_idle_time);

        m_system_cpu_usage.store(100.0 * active / total);
    }

    m_prev_system_user_time = user;
    m_prev_system_system_time = system;
    m_prev_system_idle_time = idle;
    m_prev_system_nice_time = nice;
}

//==================================================================================================
void SystemMonitorImpl::update_process_cpu_usage()
{
    task_thread_times_info_data_t thread_times {};
    if (!detail::task_thread_times(thread_times))
    {
        return;
    }

    const auto now = std::chrono::high_resolution_clock::now();
    const auto user = time_value_to_microseconds(thread_times.user_time);
    const auto system = time_value_to_microseconds(thread_times.system_time);

    if ((m_prev_process_user_time != 0) && (now > m_prev_time) &&
        (user >= m_prev_process_user_time) && (system >= m_prev_process_system_time))
    {
        const std::uint64_t cpu =
            (user - m_prev_process_user_time) + (system - m_prev_process_system_time);
        const auto time = std::chrono::duration_cast<std::chrono::microseconds>(now - m_prev_time);

        m_process_cpu_usage.store(100.0 * cpu / time.count() / m_system_cpu_count.load());
    }

    m_prev_process_user_time = user;
    m_prev_process_system_time = system;
    m_prev_time = now;
}

//==================================================================================================
void SystemMonitorImpl::update_system_memory_usage()
{
    host_basic_info_data_t basic_info {};
    if (!detail::host_basic_info(basic_info))
    {
        return;
    }

    vm_statistics64_data_t vm_info {};
    if (!detail::host_vm_info(vm_info))
    {
        return;
    }

    vm_size_t page_size = 0;
    if (!detail::host_page_size(page_size))
    {
        return;
    }

    const auto total_memory = static_cast<std::uint64_t>(basic_info.max_mem);
    const auto free_memory = static_cast<std::uint64_t>(vm_info.free_count) * page_size;

    m_total_system_memory.store(total_memory);
    m_system_memory_usage.store(total_memory - free_memory);
}

//==================================================================================================
void SystemMonitorImpl::update_process_memory_usage()
{
    task_basic_info_64_data_t task_info {};
    if (!detail::task_basic_info(task_info))
    {
        return;
    }

    m_process_memory_usage.store(static_cast<std::uint64_t>(task_info.resident_size));
}

} // namespace fly

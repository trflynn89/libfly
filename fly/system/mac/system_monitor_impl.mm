#include "fly/system/mac/system_monitor_impl.hpp"

#include "fly/logger/logger.hpp"
#include "fly/system/system_config.hpp"
#include "fly/task/task_runner.hpp"

#import <Foundation/Foundation.h>
#include <mach/mach.h>

#include <thread>

namespace fly {

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
    const mach_port_t host = mach_host_self();
    kern_return_t status = KERN_SUCCESS;

    host_cpu_load_info_data_t host_info {};
    mach_msg_type_number_t host_info_count = HOST_CPU_LOAD_INFO_COUNT;

    status = ::host_statistics(
        host,
        HOST_CPU_LOAD_INFO,
        reinterpret_cast<host_info_t>(&host_info),
        &host_info_count);
    if (status != KERN_SUCCESS)
    {
        LOGW("Could not poll system CPU (%d): %s", status, ::mach_error_string(status));
        return;
    }

    const auto user = static_cast<std::uint64_t>(host_info.cpu_ticks[CPU_STATE_USER]);
    const auto system = static_cast<std::uint64_t>(host_info.cpu_ticks[CPU_STATE_SYSTEM]);
    const auto idle = static_cast<std::uint64_t>(host_info.cpu_ticks[CPU_STATE_IDLE]);
    const auto nice = static_cast<std::uint64_t>(host_info.cpu_ticks[CPU_STATE_NICE]);

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
    const mach_port_t task = mach_task_self();
    kern_return_t status = KERN_SUCCESS;

    task_thread_times_info_data_t task_info {};
    mach_msg_type_number_t task_info_count = TASK_THREAD_TIMES_INFO_COUNT;

    status = ::task_info(
        task,
        TASK_THREAD_TIMES_INFO,
        reinterpret_cast<task_info_t>(&task_info),
        &task_info_count);
    if (status != KERN_SUCCESS)
    {
        LOGW("Could not poll process CPU (%d): %s", status, ::mach_error_string(status));
        return;
    }

    // Convert a time_value_t to microseconds.
    auto time_value_to_microseconds = [](const time_value_t &time_value) -> std::uint64_t {
        return (static_cast<std::uint64_t>(time_value.seconds) * 1'000'000) +
            static_cast<std::uint64_t>(time_value.microseconds);
    };

    const auto now = std::chrono::high_resolution_clock::now();
    const auto user = time_value_to_microseconds(task_info.user_time);
    const auto system = time_value_to_microseconds(task_info.system_time);

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
    const mach_port_t host = mach_host_self();
    kern_return_t status = KERN_SUCCESS;

    host_basic_info host_info {};
    mach_msg_type_number_t host_info_count = HOST_BASIC_INFO_COUNT;

    status = ::host_info(
        host,
        HOST_BASIC_INFO,
        reinterpret_cast<host_info_t>(&host_info),
        &host_info_count);
    if (status != KERN_SUCCESS)
    {
        LOGW("Could not poll system host memory (%d): %s", status, ::mach_error_string(status));
        return;
    }

    vm_size_t page_size = 0;

    status = ::host_page_size(host, &page_size);
    if (status != KERN_SUCCESS)
    {
        LOGW("Could not poll system page size (%d): %s", status, ::mach_error_string(status));
        return;
    }

    vm_statistics64_data_t host_stats {};
    mach_msg_type_number_t host_stats_count = HOST_VM_INFO64_COUNT;

    status = ::host_statistics64(
        host,
        HOST_VM_INFO,
        reinterpret_cast<host_info64_t>(&host_stats),
        &host_stats_count);
    if (status != KERN_SUCCESS)
    {
        LOGW("Could not poll system memory (%d): %s", status, ::mach_error_string(status));
        return;
    }

    const auto total_memory = static_cast<std::uint64_t>(host_info.max_mem);
    const auto free_memory = static_cast<std::uint64_t>(host_stats.free_count) * page_size;

    m_total_system_memory.store(total_memory);
    m_system_memory_usage.store(total_memory - free_memory);
}

//==================================================================================================
void SystemMonitorImpl::update_process_memory_usage()
{
    const mach_port_t task = mach_task_self();
    kern_return_t status = KERN_SUCCESS;

    task_basic_info_64_data_t task_info {};
    mach_msg_type_number_t task_info_count = TASK_BASIC_INFO_64_COUNT;

    status = ::task_info(
        task,
        TASK_BASIC_INFO_64,
        reinterpret_cast<task_info_t>(&task_info),
        &task_info_count);
    if (status != KERN_SUCCESS)
    {
        LOGW("Could not poll process memory (%d): %s", status, ::mach_error_string(status));
        return;
    }

    m_process_memory_usage.store(static_cast<std::uint64_t>(task_info.resident_size));
}

} // namespace fly

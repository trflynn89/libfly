#pragma once

#include "fly/fly.hpp"

#include <atomic>
#include <cstdint>
#include <memory>

namespace fly {

class SequencedTaskRunner;
class SystemConfig;

/**
 * Virtual interface for monitoring system-level resources. Provides CPU and memory monitoring. This
 * interface is platform independent - OS dependent implementations should inherit from this class.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version September 15, 2017
 */
class SystemMonitor : public std::enable_shared_from_this<SystemMonitor>
{
public:
    /**
     * Constructor.
     *
     * @param task_runner Task runner for posting monitor-related tasks onto.
     * @param config Reference to system configuration.
     */
    SystemMonitor(
        const std::shared_ptr<SequencedTaskRunner> &task_runner,
        const std::shared_ptr<SystemConfig> &config) noexcept;

    /**
     * Destructor.
     */
    virtual ~SystemMonitor() = default;

    /**
     * Queue a task to poll system-level resources.
     *
     * @return True if the system monitor is in a valid state.
     */
    bool start();

    /**
     * Get the system's CPU count.
     *
     * @return System CPU count.
     */
    std::uint32_t get_system_cpu_count() const;

    /**
     * Get the system's CPU usage percentage (0-100%) over the last second.
     *
     * @return Current system CPU usage.
     */
    double get_system_cpu_usage() const;

    /**
     * Get the process's CPU usage percentage (0-100%) over the last second.
     *
     * @return Current process CPU usage.
     */
    double get_process_cpu_usage() const;

    /**
     * Get the system's total physical memory available in bytes.
     *
     * @return Total system memory.
     */
    std::uint64_t get_total_system_memory() const;

    /**
     * Get the system's physical memory usage in bytes.
     *
     * @return Current system memory usage.
     */
    std::uint64_t get_system_memory_usage() const;

    /**
     * Get the process's physical memory usage in bytes.
     *
     * @return Current process memory usage.
     */
    std::uint64_t get_process_memory_usage() const;

protected:
    /**
     * Update the system's current CPU count.
     */
    virtual void update_system_cpu_count() = 0;

    /**
     * Update the system's current CPU usage.
     */
    virtual void update_system_cpu_usage() = 0;

    /**
     * Update the process's current CPU usage.
     */
    virtual void update_process_cpu_usage() = 0;

    /**
     * Update the system's current memory usage.
     */
    virtual void update_system_memory_usage() = 0;

    /**
     * Update the process's current memory usage.
     */
    virtual void update_process_memory_usage() = 0;

    std::atomic<std::uint32_t> m_system_cpu_count {0};
    std::atomic<double> m_system_cpu_usage {0.0};
    std::atomic<double> m_process_cpu_usage {0.0};

    std::atomic<std::uint64_t> m_total_system_memory {0};
    std::atomic<std::uint64_t> m_system_memory_usage {0};
    std::atomic<std::uint64_t> m_process_memory_usage {0};

private:
    /**
     * Check if the system CPU count was successfully set.
     *
     * @return True if the CPU count is valid.
     */
    bool is_valid() const;

    /**
     * Queue a task to poll system-level resources. When the task is completed, it re-arms itself
     * (if the system monitor is still in a valid state).
     *
     * @return True if task was able to be queued.
     */
    bool poll_system_later();

    std::shared_ptr<SequencedTaskRunner> m_task_runner;
    std::shared_ptr<SystemConfig> m_config;
};

} // namespace fly

#include FLY_OS_IMPL_PATH(system, system_monitor)

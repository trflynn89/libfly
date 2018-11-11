#pragma once

#include <atomic>
#include <cstdint>
#include <memory>

#include "fly/fly.h"
#include "fly/task/task.h"

namespace fly {

class SequencedTaskRunner;
class SystemConfig;
class SystemMonitorTask;

/**
 * Virtual interface for monitoring system-level resources. Provides CPU and
 * memory monitoring. This interface is platform independent - OS dependent
 * implementations should inherit from this class.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version September 15, 2017
 */
class SystemMonitor : public std::enable_shared_from_this<SystemMonitor>
{
    friend class SystemMonitorTask;

public:
    /**
     * Constructor.
     *
     * @param TaskRunner Task runner for posting monitor-related tasks onto.
     * @param SystemConfig Reference to system configuration.
     */
    SystemMonitor(
        const std::shared_ptr<SequencedTaskRunner> &,
        const std::shared_ptr<SystemConfig> &
    );

    /**
     * Destructor.
     */
    virtual ~SystemMonitor() = default;

    /**
     * Initialize the path monitor task.
     *
     * @return bool True if the path monitor is in a valid state.
     */
    bool Start();

    /**
     * Get the system's CPU count.
     *
     * @return uint32_t System CPU count.
     */
    uint32_t GetSystemCpuCount() const;

    /**
     * Get the system's CPU usage percentage (0-100%) over the last second.
     *
     * @return double Current system CPU usage.
     */
    double GetSystemCpuUsage() const;

    /**
     * Get the process's CPU usage percentage (0-100%) over the last second.
     *
     * @return double Current process CPU usage.
     */
    double GetProcessCpuUsage() const;

    /**
     * Get the system's total physical memory available in bytes.
     *
     * @return uint64_t Total system memory.
     */
    uint64_t GetTotalSystemMemory() const;

    /**
     * Get the system's physical memory usage in bytes.
     *
     * @return uint64_t Current system memory usage.
     */
    uint64_t GetSystemMemoryUsage() const;

    /**
     * Get the process's physical memory usage in bytes.
     *
     * @return uint64_t Current process memory usage.
     */
    uint64_t GetProcessMemoryUsage() const;

protected:
    /**
     * Update the system's current CPU count.
     */
    virtual void UpdateSystemCpuCount() = 0;

    /**
     * Update the system's current CPU usage.
     */
    virtual void UpdateSystemCpuUsage() = 0;

    /**
     * Update the process's current CPU usage.
     */
    virtual void UpdateProcessCpuUsage() = 0;

    /**
     * Update the system's current memory usage.
     */
    virtual void UpdateSystemMemoryUsage() = 0;

    /**
     * Update the process's current memory usage.
     */
    virtual void UpdateProcessMemoryUsage() = 0;

    std::atomic<uint32_t> m_systemCpuCount;
    std::atomic<double> m_systemCpuUsage;
    std::atomic<double> m_processCpuUsage;

    std::atomic<uint64_t> m_totalSystemMemory;
    std::atomic<uint64_t> m_systemMemoryUsage;
    std::atomic<uint64_t> m_processMemoryUsage;

private:
    /**
     * Check if the system CPU count was successfully set.
     *
     * @return bool True if the CPU count is valid.
     */
    bool isValid() const;

    /**
     * Update the system-level resources.
     */
    void poll();

    std::shared_ptr<SequencedTaskRunner> m_spTaskRunner;
    std::shared_ptr<Task> m_spTask;

    std::shared_ptr<SystemConfig> m_spConfig;
};

/**
 * Task to be executed to update system-level resources.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version August 12, 2018
 */
class SystemMonitorTask : public Task
{
public:
    SystemMonitorTask(const std::weak_ptr<SystemMonitor> &);

protected:
    /**
     * Call back into the system monitor to update system-level resources. If
     * the system monitor implementation is still valid, the task re-arms
     * itself.
     */
    void Run() override;

private:
    std::weak_ptr<SystemMonitor> m_wpSystemMonitor;
};

}

#include FLY_OS_IMPL_PATH(system, system_monitor)

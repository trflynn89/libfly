#pragma once

#include <atomic>
#include <cstdint>

#include "fly/fly.h"
#include "fly/task/runner.h"

namespace fly {

DEFINE_CLASS_PTRS(SystemMonitor);

/**
 * Virtual interface for monitoring system-level resources. Provides CPU and
 * memory monitoring. This interface is platform independent - OS dependent
 * implementations should inherit from this class.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version September 15, 2017
 */
class SystemMonitor : public Runner
{
public:
    /**
     * Default constructor. Constructs a system monitor with default
     * configuration.
     */
    SystemMonitor();

    /**
     * Destructor
     */
    virtual ~SystemMonitor();

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
     * Start the system monitor.
     *
     * @return bool True.
     */
    virtual bool StartRunner();

    /**
     * Stop the system monitor.
     */
    virtual void StopRunner();

    /**
     * Update the monitored system resources.
     *
     * @return bool True.
     */
    virtual bool DoWork();

    /**
     * Check if the monitor implementation is in a good state.
     *
     * @return bool True if the monitor is healthy.
     */
    virtual bool IsValid() const = 0;

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

    /**
     * Close any open handles.
     */
    virtual void Close() = 0;

    std::atomic<double> m_systemCpuUsage;
    std::atomic<double> m_processCpuUsage;

    std::atomic<uint64_t> m_totalSystemMemory;
    std::atomic<uint64_t> m_systemMemoryUsage;
    std::atomic<uint64_t> m_processMemoryUsage;
};

}

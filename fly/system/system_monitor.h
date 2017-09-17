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
     * Get the CPU usage percentage (from 0-100%) over the last second. Scaled
     * by the number of CPUs on the system.
     *
     * @return Current CPU usage percentage.
     */
    float GetCpuUsage() const;

    /**
     * Get the total system physical memory available in bytes.
     *
     * @return Total system memory.
     */
    uint64_t GetTotalMemory() const;

    /**
     * Get the free system physical memory available in bytes.
     *
     * @return Free system memory.
     */
    uint64_t GetFreeMemory() const;

    /**
     * Get the amount of physical memory used by the calling process in bytes.
     *
     * @return Memory used by process.
     */
    uint64_t GetProcessMemory() const;

protected:
    /**
     * Start the system monitor.
     *
     * @return True.
     */
    virtual bool StartRunner();

    /**
     * Stop the system monitor.
     */
    virtual void StopRunner();

    /**
     * Update the monitored system resources.
     *
     * @return True.
     */
    virtual bool DoWork();

    /**
     * Update the system's current CPU usage.
     */
    virtual void UpdateCpuUsage() = 0;

    /**
     * Update the system's current memory usage.
     */
    virtual void UpdateMemoryUsage() = 0;

    // atomic<float> may not be fully implemented, so instead store CPU as
    // percentage * 100.0; the getter will convert back to normal percentage
    std::atomic<uint64_t> m_cpuUsage;

    std::atomic<uint64_t> m_totalMemory;
    std::atomic<uint64_t> m_freeMemory;
    std::atomic<uint64_t> m_processMemory;
};

}

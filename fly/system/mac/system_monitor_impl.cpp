#include "fly/system/mac/system_monitor_impl.hpp"

#include "fly/system/system_config.hpp"
#include "fly/task/task_runner.hpp"

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
}

//==================================================================================================
void SystemMonitorImpl::update_system_cpu_usage()
{
}

//==================================================================================================
void SystemMonitorImpl::update_process_cpu_usage()
{
}

//==================================================================================================
void SystemMonitorImpl::update_system_memory_usage()
{
}

//==================================================================================================
void SystemMonitorImpl::update_process_memory_usage()
{
}

} // namespace fly

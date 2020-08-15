#include "fly/path/mac/path_monitor_impl.hpp"

#include "fly/task/task_runner.hpp"

namespace fly {

//==================================================================================================
PathMonitorImpl::PathMonitorImpl(
    const std::shared_ptr<SequencedTaskRunner> &task_runner,
    const std::shared_ptr<PathConfig> &config) noexcept :
    PathMonitor(task_runner, config)
{
}

//==================================================================================================
bool PathMonitorImpl::is_valid() const
{
    return true;
}

//==================================================================================================
void PathMonitorImpl::poll(const std::chrono::milliseconds &)
{
}

//==================================================================================================
std::unique_ptr<PathMonitor::PathInfo>
PathMonitorImpl::create_path_info(const std::filesystem::path &) const
{
    return nullptr;
}

//==================================================================================================
bool PathMonitorImpl::PathInfoImpl::is_valid() const
{
    return true;
}

} // namespace fly

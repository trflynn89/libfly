#include "test/util/waitable_task_runner.hpp"

#include "fly/task/task_manager.hpp"

namespace fly::test {

//==================================================================================================
void WaitableTaskRunner::task_complete(fly::task::TaskLocation &&location)
{
    m_completed_tasks.push(std::string(location.m_file.data(), location.m_file.size()));
}

//==================================================================================================
void WaitableTaskRunner::wait_for_task_to_complete(std::string const &location)
{
    std::string completed_location;

    do
    {
        m_completed_tasks.pop(completed_location);
    } while (completed_location.find(location) == std::string::npos);
}

//==================================================================================================
std::shared_ptr<WaitableParallelTaskRunner>
WaitableParallelTaskRunner::create(std::shared_ptr<fly::task::TaskManager> task_manager)
{
    // WaitableParallelTaskRunner has a private constructor, thus cannot be used with
    // std::make_shared. This class is used to expose the private constructor locally.
    struct WaitableParallelTaskRunnerImpl final : public WaitableParallelTaskRunner
    {
        explicit WaitableParallelTaskRunnerImpl(
            std::shared_ptr<fly::task::TaskManager> task_manager) noexcept :
            WaitableParallelTaskRunner(std::move(task_manager))
        {
        }
    };

    return std::make_shared<WaitableParallelTaskRunnerImpl>(std::move(task_manager));
}

//==================================================================================================
WaitableParallelTaskRunner::WaitableParallelTaskRunner(
    std::shared_ptr<fly::task::TaskManager> task_manager) noexcept :
    fly::task::ParallelTaskRunner(std::move(task_manager))
{
}

//==================================================================================================
void WaitableParallelTaskRunner::task_complete(fly::task::TaskLocation &&location)
{
    fly::task::TaskLocation copy = location;
    ParallelTaskRunner::task_complete(std::move(location));
    WaitableTaskRunner::task_complete(std::move(copy));
}

//==================================================================================================
std::shared_ptr<WaitableSequencedTaskRunner>
WaitableSequencedTaskRunner::create(std::shared_ptr<fly::task::TaskManager> task_manager)
{
    // WaitableSequencedTaskRunner has a private constructor, thus cannot be used with
    // std::make_shared. This class is used to expose the private constructor locally.
    struct WaitableSequencedTaskRunnerImpl final : public WaitableSequencedTaskRunner
    {
        explicit WaitableSequencedTaskRunnerImpl(
            std::shared_ptr<fly::task::TaskManager> task_manager) noexcept :
            WaitableSequencedTaskRunner(std::move(task_manager))
        {
        }
    };

    return std::make_shared<WaitableSequencedTaskRunnerImpl>(std::move(task_manager));
}

//==================================================================================================
WaitableSequencedTaskRunner::WaitableSequencedTaskRunner(
    std::shared_ptr<fly::task::TaskManager> task_manager) noexcept :
    fly::task::SequencedTaskRunner(std::move(task_manager))
{
}

//==================================================================================================
void WaitableSequencedTaskRunner::task_complete(fly::task::TaskLocation &&location)
{
    fly::task::TaskLocation copy = location;
    SequencedTaskRunner::task_complete(std::move(location));
    WaitableTaskRunner::task_complete(std::move(copy));
}

} // namespace fly::test

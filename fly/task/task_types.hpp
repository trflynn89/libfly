#pragma once

#include <cstdint>
#include <functional>

namespace fly {

class TaskRunner;

/**
 * Structure to store basic information about from where a task was posted.
 */
struct TaskLocation
{
    const char *m_file {nullptr};
    const char *m_function {nullptr};
    std::uint32_t m_line {0};
};

/**
 * Tasks posted to a task runner are wrapped in a generic lambda to be agnostic to return types.
 */
using Task = std::function<void(TaskRunner *, TaskLocation)>;

} // namespace fly

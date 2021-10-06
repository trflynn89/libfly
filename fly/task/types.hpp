#pragma once

#include <cstdint>
#include <functional>
#include <string_view>

namespace fly::task {

class TaskRunner;

/**
 * Structure to store basic information about from where a task was posted.
 *
 * N.B. This may be replaced by std::source_location when generally available.
 */
struct TaskLocation
{
    std::string_view m_file;
    std::string_view m_function;
    std::uint32_t m_line {0};
};

/**
 * Tasks posted to a task runner are wrapped in a generic lambda to be agnostic to return types.
 */
using Task = std::function<void(TaskRunner *, TaskLocation)>;

} // namespace fly::task

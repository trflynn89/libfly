#pragma once

#include <memory>

namespace fly {
class TaskManager;
} // namespace fly

namespace fly::test {

/**
 * To help tests run a bit quicker, only create a single task manager for the entire unit test
 * suite. This way, every section of every test does not need to wait for the thread pool to exit.
 * The task manager will be created with as many worker threads as there are CPU cores.
 *
 * @return A pointer to the single task manager instance.
 */
std::shared_ptr<fly::TaskManager> task_manager();

} // namespace fly::test

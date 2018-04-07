#pragma once

#include <string>

#include "test/mock/mock_system.h"

namespace fly {

/**
 * Enumerated list of mocked system calls.
 */
enum class MockCall: unsigned int
{
    INOTIFY_ADD_WATCH,
    INOTIFY_INIT1,
    POLL,
    READ,
};

/**
 * Check if a mocked system call is valid.
 *
 * @param MockCall Mocked system call to check.
 *
 * @return bool True if the mocked system call is valid.
 */
bool MockCallValid(MockCall);

/**
 * Get the name of a mocked system.
 *
 * @param MockCall Mocked system call to get.
 *
 * @return string The name of the mocked system call.
 */
std::string MockCallName(MockCall);

}

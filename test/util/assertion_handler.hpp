#pragma once

#include "fly/assert/assert.hpp"

namespace fly::test {

/**
 * Temporarily change the application's assertion handler, restoring the original handler upon
 * destruction.
 */
class ScopedAssertionHandler
{
public:
    explicit ScopedAssertionHandler(fly::assert::AssertionHandler handler) noexcept :
        m_original_handler(fly::assert::set_assertion_handler(handler))
    {
    }

    ~ScopedAssertionHandler()
    {
        fly::assert::set_assertion_handler(m_original_handler);
    }

private:
    fly::assert::AssertionHandler m_original_handler {nullptr};
};

} // namespace fly::test

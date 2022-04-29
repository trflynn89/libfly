#include "fly/assert/assert.hpp"

#include <atomic>
#include <cstdlib>

namespace {

//==================================================================================================
void abort(std::string_view, std::string_view, std::string_view, std::uint32_t)
{
    std::abort();
}

//==================================================================================================
std::atomic<fly::assert::AssertionHandler> s_assertion_handler {abort};

} // namespace

namespace fly::assert {

//==================================================================================================
AssertionHandler set_assertion_handler(AssertionHandler handler)
{
    return s_assertion_handler.exchange(handler);
}

//==================================================================================================
AssertionHandler assertion_handler()
{
    return s_assertion_handler.load();
}

} // namespace fly::assert

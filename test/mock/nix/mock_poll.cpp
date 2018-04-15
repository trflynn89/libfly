#include <cerrno>
#include <chrono>
#include <thread>

#include <poll.h>

#include "test/mock/mock_system.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int __real_poll(struct pollfd *fds, nfds_t nfds, int timeout);

    int __wrap_poll(struct pollfd *fds, nfds_t nfds, int timeout)
    {
        if (fly::MockSystem::MockEnabled(fly::MockCall::POLL))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
            errno = 0;
            return -1;
        }

        return __real_poll(fds, nfds, timeout);
    }

#ifdef __cplusplus
}
#endif

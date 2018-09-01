#include <cerrno>

#include <sys/socket.h>
#include <sys/types.h>

#include "test/mock/mock_system.h"

#ifdef __cplusplus
extern "C"
{
#endif

    ssize_t  __real_send(int sockfd, const void *buf, size_t len, int flags);

    ssize_t  __wrap_send(int sockfd, const void *buf, size_t len, int flags)
    {
        if (fly::MockSystem::MockEnabled(fly::MockCall::Send))
        {
            errno = 0;
            return -1;
        }
        else if (fly::MockSystem::MockEnabled(fly::MockCall::Send_Blocking))
        {
            // On the first call, send all but the last byte, setting errno to
            // indicate that the send would have "blocked". Subsequent calls
            // will have len == 1; go ahead and finish the send at that point.
            if (len > 1)
            {
                errno = EWOULDBLOCK;
                len -= 1;
            }
        }

        return __real_send(sockfd, buf, len, flags);
    }

#ifdef __cplusplus
}
#endif

#include <cerrno>

#include <sys/socket.h>
#include <sys/types.h>

#include "test/mock/mock_system.h"

#ifdef __cplusplus
extern "C"
{
#endif

    ssize_t  __real_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);

    ssize_t  __wrap_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
    {
        if (fly::MockSystem::MockEnabled(fly::MockCall::Sendto))
        {
            errno = 0;
            return -1;
        }

        return __real_sendto(sockfd, buf, len, flags, dest_addr, addrlen);
    }

#ifdef __cplusplus
}
#endif

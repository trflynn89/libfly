#include <cerrno>

#include <sys/socket.h>
#include <sys/types.h>

#include "test/mock/mock_system.h"

#ifdef __cplusplus
extern "C"
{
#endif

    ssize_t  __real_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);

    ssize_t  __wrap_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
    {
        if (fly::MockSystem::MockEnabled(fly::MockCall::RECVFROM))
        {
            errno = 0;
            return -1;
        }

        return __real_recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
    }

#ifdef __cplusplus
}
#endif

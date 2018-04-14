#include <sys/socket.h>
#include <sys/types.h>

#include "test/mock/mock_system.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int __real_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

    int __wrap_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
    {
        if (fly::MockSystem::MockEnabled(fly::MockCall::ACCEPT))
        {
            return -1;
        }

        return __real_accept(sockfd, addr, addrlen);
    }

#ifdef __cplusplus
}
#endif

#include <cerrno>

#include "test/mock/mock_system.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int __real_listen(int sockfd, int backlog);

    int __wrap_listen(int sockfd, int backlog)
    {
        if (fly::MockSystem::MockEnabled(fly::MockCall::Listen))
        {
            errno = 0;
            return -1;
        }

        return __real_listen(sockfd, backlog);
    }

#ifdef __cplusplus
}
#endif

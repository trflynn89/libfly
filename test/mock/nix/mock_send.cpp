#include <cerrno>

#include <sys/socket.h>
#include <sys/types.h>

#include "test/mock/mock_system.h"

namespace
{
    // This is a hack to be able to test send() being called multiple times in
    // SocketTest::Send_Async_MockSendBlock.
    //
    // On the first call to send() when mocked blocking is enabled, send half of
    // the bytes, simulating packet fragmentation.  On the second call, send 0
    // bytes and set errno to EWOULDBLOCK to make SocketImpl break out of its
    // send loop after the packet fragmentation. On the third call, send the
    // remaining bytes, completing the send.
    static int s_callCount = 0;
}

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
            switch (s_callCount++)
            {
            case 0:
                len /= 2;
                break;

            case 1:
                errno = EWOULDBLOCK;
                return -1;

            case 2:
                s_callCount = 0;
                break;
            }
        }

        return __real_send(sockfd, buf, len, flags);
    }

#ifdef __cplusplus
}
#endif

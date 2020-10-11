#include "test/mock/nix/mock_calls.hpp"

#include "test/mock/mock_system.hpp"

#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <sys/times.h>
#include <sys/types.h>

#include <algorithm>
#include <chrono>
#include <thread>
#include <vector>

namespace {

// This is a hack to be able to test fcntl() being called multiple times in
// socket/nix/socket_impl.cpp::SetAsync().
//
// The socket_test unit test will test SetAsync() twice. In the first test, fcntl() will fail on its
// first invocation. In the second test, fcntl() will behave normally on its first invocation, and
// fail on the second.
int s_fcntl_call_count = 0;
int s_fcntl_next_call = 1;

// This is a hack to be able to test send() being called multiple times in
// SocketTest::Send_Async_MockSendBlock.
//
// On the first call to send() when mocked blocking is enabled, send half of the bytes, simulating
// packet fragmentation.  On the second call, send 0 bytes and set errno to EWOULDBLOCK to make
// SocketImpl break out of its send loop after the packet fragmentation. On the third call, send the
// remaining bytes, completing the send.
int s_send_call_count = 0;

// This is a hack to be able to test sendto() being called multiple times in
// SocketTest::Send_Async_MockSendtoBlock.
//
// On the first call to sendto() when mocked blocking is enabled, send half of the bytes, simulating
// packet fragmentation.  On the second call, send 0 bytes and set errno to EWOULDBLOCK to make
// SocketImpl break out of its send loop after the packet fragmentation. On the third call, send the
// remaining bytes, completing the send.
int s_sendto_call_count = 0;

} // namespace

#ifdef __cplusplus
extern "C"
{
#endif

    //==============================================================================================
    int __real_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

    int __wrap_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
    {
        if (fly::test::MockSystem::mock_enabled(fly::test::MockCall::Accept))
        {
            errno = 0;
            return -1;
        }

        return __real_accept(sockfd, addr, addrlen);
    }

    //==============================================================================================
    int __real_backtrace(void **buffer, int size);

    int __wrap_backtrace(void **buffer, int size)
    {
        if (fly::test::MockSystem::mock_enabled(fly::test::MockCall::Backtrace))
        {
            errno = 0;
            return 0;
        }

        return __real_backtrace(buffer, size);
    }

    //==============================================================================================
    void __real_backtrace_symbols_fd(void *const *buffer, int size, int fd);

    void __wrap_backtrace_symbols_fd(void *const *buffer, int size, int fd)
    {
        if (fly::test::MockSystem::mock_enabled(fly::test::MockCall::BacktraceSymbols))
        {
            errno = 0;
            return;
        }

        return __real_backtrace_symbols_fd(buffer, size, fd);
    }

    //==============================================================================================
    int __real_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

    int __wrap_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
    {
        if (fly::test::MockSystem::mock_enabled(fly::test::MockCall::Bind))
        {
            errno = 0;
            return -1;
        }

        return __real_bind(sockfd, addr, addrlen);
    }

    //==============================================================================================
    int __real_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

    int __wrap_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
    {
        bool fail;

        if (fly::test::MockSystem::mock_enabled(fly::test::MockCall::Connect, fail))
        {
            errno = 0;
            return (fail ? -1 : 0);
        }

        return __real_connect(sockfd, addr, addrlen);
    }

    //==============================================================================================
    int __real_fcntl(int fd, int cmd, int args);

    int __wrap_fcntl(int fd, int cmd, int args)
    {
        if (fly::test::MockSystem::mock_enabled(fly::test::MockCall::Fcntl))
        {
            if (++s_fcntl_call_count == s_fcntl_next_call)
            {
                s_fcntl_call_count = 0;
                ++s_fcntl_next_call;

                errno = 0;
                return -1;
            }
        }
        else
        {
            s_fcntl_call_count = 0;
            s_fcntl_next_call = 1;
        }

        return __real_fcntl(fd, cmd, args);
    }

    //==============================================================================================
    struct hostent *__real_gethostbyname(const char *name);

    struct hostent *__wrap_gethostbyname(const char *name)
    {
        if (fly::test::MockSystem::mock_enabled(fly::test::MockCall::Gethostbyname))
        {
            errno = 0;
            return nullptr;
        }

        return __real_gethostbyname(name);
    }

    //==============================================================================================
    int __real_getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);

    int __wrap_getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen)
    {
        if (fly::test::MockSystem::mock_enabled(fly::test::MockCall::Getsockopt))
        {
            errno = 0;
            return -1;
        }

        return __real_getsockopt(sockfd, level, optname, optval, optlen);
    }

    //==============================================================================================
    int __real_inotify_add_watch(int fd, const char *pathname, uint32_t mask);

    int __wrap_inotify_add_watch(int fd, const char *pathname, uint32_t mask)
    {
        if (fly::test::MockSystem::mock_enabled(fly::test::MockCall::InotifyAddWatch))
        {
            errno = 0;
            return -1;
        }

        return __real_inotify_add_watch(fd, pathname, mask);
    }

    //==============================================================================================
    int __real_inotify_init1(int flags);

    int __wrap_inotify_init1(int flags)
    {
        if (fly::test::MockSystem::mock_enabled(fly::test::MockCall::InotifyInit1))
        {
            errno = 0;
            return -1;
        }

        return __real_inotify_init1(flags);
    }

    //==============================================================================================
    int __real_isatty(int fd);

    int __wrap_isatty(int fd)
    {
        bool fail;

        if (fly::test::MockSystem::mock_enabled(fly::test::MockCall::IsATTY, fail))
        {
            errno = 0;
            return fail ? 0 : 1;
        }

        return __real_isatty(fd);
    }

    //==============================================================================================
    int __real_listen(int sockfd, int backlog);

    int __wrap_listen(int sockfd, int backlog)
    {
        if (fly::test::MockSystem::mock_enabled(fly::test::MockCall::Listen))
        {
            errno = 0;
            return -1;
        }

        return __real_listen(sockfd, backlog);
    }

    //==============================================================================================
    struct tm *__real_localtime_r(const time_t *timep, struct tm *result);

    struct tm *__wrap_localtime_r(const time_t *timep, struct tm *result)
    {
        if (fly::test::MockSystem::mock_enabled(fly::test::MockCall::LocalTime))
        {
            errno = 0;
            return nullptr;
        }

        return __real_localtime_r(timep, result);
    }

    //==============================================================================================
    int __real_poll(struct pollfd *fds, nfds_t nfds, int timeout);

    int __wrap_poll(struct pollfd *fds, nfds_t nfds, int timeout)
    {
        if (fly::test::MockSystem::mock_enabled(fly::test::MockCall::Poll))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
            errno = 0;
            return -1;
        }

        return __real_poll(fds, nfds, timeout);
    }

    //==============================================================================================
    ssize_t __real_read(int fd, void *buf, size_t count);

    ssize_t __wrap_read(int fd, void *buf, size_t count)
    {
        if (fly::test::MockSystem::mock_enabled(fly::test::MockCall::Read))
        {
            errno = 0;
            return -1;
        }

        return __real_read(fd, buf, count);
    }

    //==============================================================================================
    ssize_t __real_recv(int sockfd, void *buf, size_t len, int flags);

    ssize_t __wrap_recv(int sockfd, void *buf, size_t len, int flags)
    {
        if (fly::test::MockSystem::mock_enabled(fly::test::MockCall::Recv))
        {
            errno = 0;
            return -1;
        }

        return __real_recv(sockfd, buf, len, flags);
    }

    //==============================================================================================
    ssize_t __real_recvfrom(
        int sockfd,
        void *buf,
        size_t len,
        int flags,
        struct sockaddr *src_addr,
        socklen_t *addrlen);

    ssize_t __wrap_recvfrom(
        int sockfd,
        void *buf,
        size_t len,
        int flags,
        struct sockaddr *src_addr,
        socklen_t *addrlen)
    {
        if (fly::test::MockSystem::mock_enabled(fly::test::MockCall::Recvfrom))
        {
            errno = 0;
            return -1;
        }

        return __real_recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
    }

    //==============================================================================================
    ssize_t __real_send(int sockfd, const void *buf, size_t len, int flags);

    ssize_t __wrap_send(int sockfd, const void *buf, size_t len, int flags)
    {
        if (fly::test::MockSystem::mock_enabled(fly::test::MockCall::Send))
        {
            errno = 0;
            return -1;
        }
        else if (fly::test::MockSystem::mock_enabled(fly::test::MockCall::SendBlocking))
        {
            switch (s_send_call_count++)
            {
                case 0:
                    len /= 2;
                    break;

                case 1:
                    errno = EWOULDBLOCK;
                    return -1;

                case 2:
                    s_send_call_count = 0;
                    break;
            }
        }

        return __real_send(sockfd, buf, len, flags);
    }

    //==============================================================================================
    ssize_t __real_sendto(
        int sockfd,
        const void *buf,
        size_t len,
        int flags,
        const struct sockaddr *dest_addr,
        socklen_t addrlen);

    ssize_t __wrap_sendto(
        int sockfd,
        const void *buf,
        size_t len,
        int flags,
        const struct sockaddr *dest_addr,
        socklen_t addrlen)
    {
        if (fly::test::MockSystem::mock_enabled(fly::test::MockCall::Sendto))
        {
            errno = 0;
            return -1;
        }
        else if (fly::test::MockSystem::mock_enabled(fly::test::MockCall::SendtoBlocking))
        {
            switch (s_sendto_call_count++)
            {
                case 0:
                    len /= 2;
                    break;

                case 1:
                    errno = EWOULDBLOCK;
                    return -1;

                case 2:
                    s_sendto_call_count = 0;
                    break;
            }
        }

        return __real_sendto(sockfd, buf, len, flags, dest_addr, addrlen);
    }

    //==============================================================================================
    int __real_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);

    int __wrap_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
    {
        if (fly::test::MockSystem::mock_enabled(fly::test::MockCall::Setsockopt))
        {
            errno = 0;
            return -1;
        }

        return __real_setsockopt(sockfd, level, optname, optval, optlen);
    }

    //==============================================================================================
    int __real_socket(int domain, int type, int protocol);

    int __wrap_socket(int domain, int type, int protocol)
    {
        if (fly::test::MockSystem::mock_enabled(fly::test::MockCall::Socket))
        {
            errno = 0;
            return -1;
        }

        return __real_socket(domain, type, protocol);
    }

    //==============================================================================================
    int __real_sysinfo(struct sysinfo *info);

    int __wrap_sysinfo(struct sysinfo *info)
    {
        if (fly::test::MockSystem::mock_enabled(fly::test::MockCall::Sysinfo))
        {
            errno = 0;
            return -1;
        }

        return __real_sysinfo(info);
    }

    //==============================================================================================
    clock_t __real_times(struct tms *buf);

    clock_t __wrap_times(struct tms *buf)
    {
        if (fly::test::MockSystem::mock_enabled(fly::test::MockCall::Times))
        {
            errno = 0;
            return static_cast<clock_t>(-1);
        }

        return __real_times(buf);
    }

    //==============================================================================================
    ssize_t __real_write(int fd, const void *buf, size_t count);

    ssize_t __wrap_write(int fd, const void *buf, size_t count)
    {
        if (fly::test::MockSystem::mock_enabled(fly::test::MockCall::Write))
        {
            errno = 0;
            return -1;
        }

        return __real_write(fd, buf, count);
    }

#ifdef __cplusplus
}
#endif

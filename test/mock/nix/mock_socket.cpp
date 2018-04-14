#include "test/mock/mock_system.h"

#ifdef __cplusplus
extern "C"
{
#endif

    int __real_socket(int domain, int type, int protocol);

    int __wrap_socket(int domain, int type, int protocol)
    {
        if (fly::MockSystem::MockEnabled(fly::MockCall::SOCKET))
        {
            return -1;
        }

        return __real_socket(domain, type, protocol);
    }

#ifdef __cplusplus
}
#endif

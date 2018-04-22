#include <cerrno>

#include <netdb.h>

#include "test/mock/mock_system.h"

#ifdef __cplusplus
extern "C"
{
#endif

    struct hostent *__real_gethostbyname(const char *name);

    struct hostent *__wrap_gethostbyname(const char *name)
    {
        if (fly::MockSystem::MockEnabled(fly::MockCall::Gethostbyname))
        {
            errno = 0;
            return NULL;
        }

        return __real_gethostbyname(name);
    }

#ifdef __cplusplus
}
#endif

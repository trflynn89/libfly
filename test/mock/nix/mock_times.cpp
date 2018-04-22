#include <cerrno>

#include <sys/times.h>

#include "test/mock/mock_system.h"

#ifdef __cplusplus
extern "C"
{
#endif

    clock_t __real_times(struct tms *buf);

    clock_t __wrap_times(struct tms *buf)
    {
        if (fly::MockSystem::MockEnabled(fly::MockCall::Times))
        {
            errno = 0;
            return static_cast<clock_t>(-1);
        }

        return __real_times(buf);
    }

#ifdef __cplusplus
}
#endif

#include <algorithm>
#include <cerrno>
#include <string>
#include <vector>

#include "test/mock/mock_system.h"

namespace
{
    static const std::vector<std::string> s_tmpEnvs = {
        "TMPDIR", "TMP", "TEMP", "TEMPDIR"
    };
}

#ifdef __cplusplus
extern "C"
{
#endif

    char *__real_getenv(const char *name);

    char *__wrap_getenv(const char *name)
    {
        if (fly::MockSystem::MockEnabled(fly::MockCall::Getenv))
        {
            errno = 0;

            if (std::find(s_tmpEnvs.begin(), s_tmpEnvs.end(), name) != s_tmpEnvs.end())
            {
                return (char *)("/tmp/");
            }

            return NULL;
        }

        return __real_getenv(name);
    }

#ifdef __cplusplus
}
#endif

#include <algorithm>
#include <string>
#include <vector>

#include "test/mock/mock_system.h"

#ifdef __cplusplus
extern "C"
{
#endif

    char *__real_getenv(const char *name);

    char *__wrap_getenv(const char *name)
    {
        if (fly::MockSystem::MockEnabled(fly::MockCall::GETENV))
        {
            static const std::vector<std::string> tmpEnvs = {
                "TMPDIR", "TMP", "TEMP", "TEMPDIR"
            };

            if (std::find(tmpEnvs.begin(), tmpEnvs.end(), name) != tmpEnvs.end())
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

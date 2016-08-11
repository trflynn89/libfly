#pragma once

#include <string>

#include <fly/fly.h>
#include <fly/config/config.h>

namespace fly {

DEFINE_CLASS_PTRS(TaskConfig);

/**
 * Class to hold configuration values related to runnable tasks.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 21, 2016
 */
class TaskConfig : public Config
{
public:
    /**
     * Constructor.
     */
    FLY_API TaskConfig();

    /**
     * Destructor.
     */
    FLY_API virtual ~TaskConfig();

    /**
     * Get the name to associate with this configuration.
     */
    FLY_API static std::string GetName();

    /**
     * @return Number of worker threads, used if could not find number of cores.
     */
    FLY_API int DefaultWorkerCount() const;
};

}

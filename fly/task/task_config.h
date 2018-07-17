#pragma once

#include <chrono>
#include <string>

#include "fly/fly.h"
#include "fly/config/config.h"

namespace fly {

FLY_CLASS_PTRS(TaskConfig);

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
     * Get the name to associate with this configuration.
     */
    static std::string GetName();

    /**
     * @return Delay between monitor poll intervals.
     */
    std::chrono::milliseconds PollInterval() const;
};

}

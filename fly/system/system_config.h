#pragma once

#include <chrono>
#include <string>

#include "fly/fly.h"
#include "fly/config/config.h"

namespace fly {

FLY_CLASS_PTRS(SystemConfig);

/**
 * Class to hold configuration values related to the system interface.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version August 12, 2018
 */
class SystemConfig : public Config
{
public:
    /**
     * Get the name to associate with this configuration.
     */
    static std::string GetName();

    /**
     * @return Delay between system monitor poll intervals.
     */
    virtual std::chrono::milliseconds PollInterval() const;
};

}

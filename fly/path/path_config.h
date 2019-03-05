#pragma once

#include "fly/config/config.h"

#include <chrono>
#include <string>

namespace fly {

/**
 * Class to hold configuration values related to paths.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version August 12, 2018
 */
class PathConfig : public Config
{
public:
    /**
     * Constructor.
     */
    PathConfig();

    /**
     * Get the name to associate with this configuration.
     */
    static std::string GetName();

    /**
     * @return Delay between path monitor poll intervals.
     */
    virtual std::chrono::milliseconds PollInterval() const;

protected:
    std::chrono::milliseconds::rep m_defaultPollInterval;
};

} // namespace fly

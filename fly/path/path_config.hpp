#pragma once

#include "fly/config/config.hpp"

#include <chrono>

namespace fly {

/**
 * Class to hold configuration values related to paths.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version August 12, 2018
 */
class PathConfig : public Config
{
public:
    static constexpr const char *identifier = "path";

    /**
     * Constructor.
     */
    PathConfig() noexcept;

    /**
     * @return Delay between path monitor poll intervals.
     */
    virtual std::chrono::milliseconds PollInterval() const noexcept;

protected:
    std::chrono::milliseconds::rep m_defaultPollInterval;
};

} // namespace fly

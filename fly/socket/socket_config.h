#pragma once

#include <chrono>
#include <string>

#include "fly/fly.h"
#include "fly/config/config.h"

namespace fly {

DEFINE_CLASS_PTRS(SocketConfig);

/**
 * Class to hold networking-related configuration values.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 19, 2016
 */
class SocketConfig : public Config
{
public:
    /**
     * Constructor.
     */
    SocketConfig();

    /**
     * Destructor.
     */
    virtual ~SocketConfig();

    /**
     * Get the name to associate with this configuration.
     */
    static std::string GetName();

    /**
     * @return Sleep time for socket IO thread.
     */
    std::chrono::microseconds IoWaitTime() const;

    /**
     * @return Character signifying the end of a message received over a socket.
     */
    char EndOfMessage() const;

    /**
     * Size of packet to use for send/receive operations.
     */
    size_t PacketSize() const;
};

}

#pragma once

#include <chrono>
#include <string>

#include <fly/fly.h>
#include <fly/config/config.h>

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
    FLY_API SocketConfig();

    /**
     * Destructor.
     */
    FLY_API virtual ~SocketConfig();

    /**
     * Get the name to associate with this configuration.
     */
    FLY_API static std::string GetName();

    /**
     * @return Sleep time for socket IO thread.
     */
    FLY_API std::chrono::microseconds IoWaitTime() const;

    /**
     * @return Character signifying the end of a message received over a socket.
     */
    FLY_API char EndOfMessage() const;

    /**
     * Size of packet to use for send/receive operations.
     */
    FLY_API size_t PacketSize() const;
};

}

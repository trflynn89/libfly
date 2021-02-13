#include "fly/net/ipv4_address.hpp"

#include "fly/types/string/string.hpp"

namespace fly::net {

//==================================================================================================
std::string IPv4Address::to_string() const
{
    return fly::String::format(
        "{}.{}.{}.{}",
        m_address & 0xff,
        (m_address >> 8) & 0xff,
        (m_address >> 16) & 0xff,
        (m_address >> 24) & 0xff);
}

//==================================================================================================
std::ostream &operator<<(std::ostream &stream, const IPv4Address &address)
{
    stream << address.to_string();
    return stream;
}

} // namespace fly::net

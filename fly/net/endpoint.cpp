#include "fly/net/endpoint.hpp"

#include "fly/net/ipv4_address.hpp"
#include "fly/net/ipv6_address.hpp"
#include "fly/types/string/string.hpp"

namespace fly::net {

//==================================================================================================
template <>
std::string Endpoint<fly::net::IPv4Address>::to_string() const
{
    return fly::String::format("{}:{}", m_address, m_port);
}

//==================================================================================================
template <>
std::string Endpoint<fly::net::IPv6Address>::to_string() const
{
    return fly::String::format("[{}]:{}", m_address, m_port);
}

} // namespace fly::net

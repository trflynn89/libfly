#include "fly/net/ipv6_address.hpp"

#include "fly/types/string/string.hpp"

namespace fly::net {

//==================================================================================================
std::string IPv6Address::to_string() const
{
    return fly::String::format("{}", *this);
}

//==================================================================================================
void IPv6Address::copy(address_type::value_type (&address)[16]) const
{
    std::copy(m_address.begin(), m_address.end(), std::begin(address));
}

//==================================================================================================
std::ostream &operator<<(std::ostream &stream, const IPv6Address &address)
{
    auto join_segments = [&address](std::size_t index) -> std::uint16_t
    {
        return (address.m_address[index] << 8) | address.m_address[index + 1];
    };

    bool used_short_form = false;

    for (std::size_t i = 0; i < IPv6Address::s_address_size;)
    {
        const std::uint16_t segment = join_segments(i);

        if ((segment == 0) && !used_short_form)
        {
            do
            {
                i += 2;
            } while ((i < IPv6Address::s_address_size) && (join_segments(i) == 0));

            stream << ((i < IPv6Address::s_address_size) ? ":" : "::");
            used_short_form = true;
        }
        else
        {
            stream << fly::String::format("{:.{}}{:x}", ":", (i > 0) ? 1 : 0, segment);
            i += 2;
        }
    }

    return stream;
}

} // namespace fly::net

#include "fly/net/endpoint.hpp"

#include "fly/net/ipv4_address.hpp"
#include "fly/net/ipv6_address.hpp"
#include "fly/types/string/format.hpp"

#include "catch2/catch_template_test_macros.hpp"
#include "catch2/catch_test_macros.hpp"

#include <type_traits>

CATCH_TEMPLATE_TEST_CASE("Endpoint", "[net]", fly::net::IPv4Address, fly::net::IPv6Address)
{
    using IPAddressType = TestType;
    using EndpointType = fly::net::Endpoint<IPAddressType>;

    CATCH_SECTION("Endpoints may be default constructed")
    {
        EndpointType const endpoint;

        CATCH_CHECK(endpoint.address() == IPAddressType());
        CATCH_CHECK(endpoint.port() == 0);

        CATCH_CHECK(endpoint.is_ipv4() == std::is_same_v<IPAddressType, fly::net::IPv4Address>);
        CATCH_CHECK(endpoint.is_ipv6() == std::is_same_v<IPAddressType, fly::net::IPv6Address>);
    }

    CATCH_SECTION("Endpoints may be constructed from existing IP addresses")
    {
        auto const address1 = IPAddressType::in_addr_loopback();
        EndpointType const endpoint1(address1, 1);

        CATCH_CHECK(endpoint1.address() == address1);
        CATCH_CHECK(endpoint1.port() == 1);

        auto address2 = IPAddressType::in_addr_loopback();
        EndpointType const endpoint2(std::move(address2), 2);

        CATCH_CHECK(endpoint2.address() == address1);
        CATCH_CHECK(endpoint2.port() == 2);
    }

    CATCH_SECTION("Endpoints may be copied")
    {
        auto const endpoint1 = EndpointType(IPAddressType::in_addr_loopback(), 1);
        auto const endpoint2 = endpoint1;

        CATCH_CHECK(endpoint1 == endpoint2);
    }

    CATCH_SECTION("Endpoints may be moved")
    {
        auto endpoint1 = EndpointType(IPAddressType::in_addr_loopback(), 1);
        auto const endpoint2 = std::move(endpoint1);

        CATCH_CHECK(endpoint2.address() == IPAddressType::in_addr_loopback());
        CATCH_CHECK(endpoint2.port() == 1);
    }

    CATCH_SECTION("Endpoints may be compared")
    {
        auto const endpoint1 = EndpointType(IPAddressType::in_addr_loopback(), 1);
        auto const endpoint2 = EndpointType(IPAddressType::in_addr_loopback(), 2);
        auto const endpoint3 = EndpointType(IPAddressType::in_addr_loopback(), 3);

        CATCH_CHECK(endpoint2 == endpoint2);
        CATCH_CHECK(endpoint1 != endpoint2);
        CATCH_CHECK(endpoint1 < endpoint2);
        CATCH_CHECK(endpoint1 <= endpoint2);
        CATCH_CHECK(endpoint3 > endpoint2);
        CATCH_CHECK(endpoint3 >= endpoint2);
    }

    CATCH_SECTION("Endpoints may be used in constant expressions")
    {
        constexpr auto endpoint1 = EndpointType(IPAddressType::in_addr_loopback(), 1);
        constexpr auto endpoint2 = EndpointType(IPAddressType::in_addr_loopback(), 2);
        constexpr auto endpoint3 = EndpointType(IPAddressType::in_addr_loopback(), 3);

        static_assert(endpoint1.address() == IPAddressType::in_addr_loopback());
        static_assert(endpoint2.address() == IPAddressType::in_addr_loopback());
        static_assert(endpoint3.address() == IPAddressType::in_addr_loopback());

        static_assert(endpoint1.port() == 1);
        static_assert(endpoint2.port() == 2);
        static_assert(endpoint3.port() == 3);

        static_assert(endpoint1 < endpoint2);
        static_assert(endpoint2 < endpoint3);
    }

    if constexpr (EndpointType::is_ipv4())
    {
        CATCH_SECTION("Endpoints may be converted to a string")
        {
            auto const endpoint = EndpointType(IPAddressType::in_addr_loopback(), 1);
            CATCH_CHECK(fly::string::format("{}", endpoint) == "127.0.0.1:1");
        }

        CATCH_SECTION("Endpoints may be created from a string")
        {
            auto const endpoint = EndpointType::from_string("127.0.0.1:123");
            CATCH_REQUIRE(endpoint);

            CATCH_CHECK(endpoint->address() == IPAddressType::in_addr_loopback());
            CATCH_CHECK(endpoint->port() == 123);
        }

        CATCH_SECTION("String parsing fails if port separator doesn't appear in middle of string")
        {
            CATCH_CHECK_FALSE(EndpointType::from_string("127.0.0.1"));
            CATCH_CHECK_FALSE(EndpointType::from_string("127.0.0.1:"));
            CATCH_CHECK_FALSE(EndpointType::from_string(":1"));
        }

        CATCH_SECTION("String parsing fails if address is not an IPv4 address")
        {
            CATCH_CHECK_FALSE(EndpointType::from_string("127.a.0.1:123"));
            CATCH_CHECK_FALSE(EndpointType::from_string("[::1]:123"));
        }

        CATCH_SECTION("String parsing fails if port is not a decimal number")
        {
            CATCH_CHECK_FALSE(EndpointType::from_string("127.0.0.1:ab"));
        }

        CATCH_SECTION("String parsing fails if port is larger than 16 btis")
        {
            CATCH_CHECK_FALSE(EndpointType::from_string("127.0.0.1:65536"));
        }

        CATCH_SECTION("String parsing fails if entire string is not consumed")
        {
            CATCH_CHECK_FALSE(EndpointType::from_string("127.0.0.1:123a"));
        }

        CATCH_SECTION("String parsing fails if address does not begin with IPv4 address")
        {
            CATCH_CHECK_FALSE(EndpointType::from_string("a127.0.0.1:123"));
        }
    }
    else if constexpr (EndpointType::is_ipv6())
    {
        CATCH_SECTION("Endpoints may be converted to a string")
        {
            auto const endpoint = EndpointType(IPAddressType::in_addr_loopback(), 1);
            CATCH_CHECK(fly::string::format("{}", endpoint) == "[::1]:1");
        }

        CATCH_SECTION("Endpoints may be created from a string")
        {
            auto const endpoint = EndpointType::from_string("[::1]:123");
            CATCH_REQUIRE(endpoint);

            CATCH_CHECK(endpoint->address() == IPAddressType::in_addr_loopback());
            CATCH_CHECK(endpoint->port() == 123);
        }

        CATCH_SECTION("String parsing fails if port separator doesn't appear in middle of string")
        {
            CATCH_CHECK_FALSE(EndpointType::from_string("[::1]"));
            CATCH_CHECK_FALSE(EndpointType::from_string("[::1]:"));
            CATCH_CHECK_FALSE(EndpointType::from_string(":1"));
        }

        CATCH_SECTION("String parsing fails if IPv6 address isn't surrounded by brackets")
        {
            CATCH_CHECK_FALSE(EndpointType::from_string("::1:1"));
            CATCH_CHECK_FALSE(EndpointType::from_string("[::1:1"));
            CATCH_CHECK_FALSE(EndpointType::from_string("::1]:1"));
            CATCH_CHECK_FALSE(EndpointType::from_string("[]:1"));
            CATCH_CHECK_FALSE(EndpointType::from_string("[:1"));
            CATCH_CHECK_FALSE(EndpointType::from_string("]:1"));
        }

        CATCH_SECTION("String parsing fails if address is not an IPv6 address")
        {
            CATCH_CHECK_FALSE(EndpointType::from_string("[::xy]:123"));
            CATCH_CHECK_FALSE(EndpointType::from_string("127.0.0.1:123"));
        }

        CATCH_SECTION("String parsing fails if port is not a decimal number")
        {
            CATCH_CHECK_FALSE(EndpointType::from_string("[::1]:ab"));
        }

        CATCH_SECTION("String parsing fails if port is larger than 16 btis")
        {
            CATCH_CHECK_FALSE(EndpointType::from_string("[::1]:65536"));
        }

        CATCH_SECTION("String parsing fails if entire string is not consumed")
        {
            CATCH_CHECK_FALSE(EndpointType::from_string("[::1]:123a"));
        }

        CATCH_SECTION("String parsing fails if address does not begin with IPv6 address")
        {
            CATCH_CHECK_FALSE(EndpointType::from_string("a[::1]:123"));
        }
    }
}

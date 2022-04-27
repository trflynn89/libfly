#include "fly/net/ipv4_address.hpp"

#include "fly/fly.hpp"
#include "fly/types/numeric/endian.hpp"
#include "fly/types/string/format.hpp"

#include "catch2/catch_test_macros.hpp"

#if defined(FLY_LINUX) || defined(FLY_MACOS)
#    include <netinet/in.h>
#elif defined(FLY_WINDOWS)
#    include <WinSock2.h>
#endif

CATCH_TEST_CASE("IPv4Address", "[net]")
{
    CATCH_SECTION("INADDR_ANY should be equivalent to system value")
    {
        auto const address = fly::net::IPv4Address::in_addr_any();
        CATCH_CHECK(address.host_order() == INADDR_ANY);
    }

    CATCH_SECTION("INADDR_BROADCAST should be equivalent to system value")
    {
        auto const address = fly::net::IPv4Address::in_addr_broadcast();
        CATCH_CHECK(address.host_order() == INADDR_BROADCAST);
    }

    CATCH_SECTION("INADDR_LOOPBACK should be equivalent to system value")
    {
        auto const address = fly::net::IPv4Address::in_addr_loopback();
        CATCH_CHECK(address.host_order() == INADDR_LOOPBACK);
    }

    CATCH_SECTION("Default constructed IPv4 addresses are initialized to 0.0.0.0")
    {
        fly::net::IPv4Address const address;
        CATCH_CHECK(address == fly::net::IPv4Address::in_addr_any());
    }

    CATCH_SECTION("Construction from an array is interpreted as host order")
    {
        fly::net::IPv4Address const address({0x11, 0x22, 0x33, 0x44});
        fly::net::IPv4Address::int_type const ip = 0x11'22'33'44;

        CATCH_CHECK(
            address.network_order() == fly::endian_swap_if_non_native<std::endian::big>(ip));
        CATCH_CHECK(address.host_order() == ip);
    }

    CATCH_SECTION("Construction from an integer is interpreted as network order")
    {
        fly::net::IPv4Address const address(0x11'22'33'44);
        fly::net::IPv4Address::int_type const ip = 0x11'22'33'44;

        CATCH_CHECK(address.network_order() == ip);
        CATCH_CHECK(address.host_order() == fly::endian_swap_if_non_native<std::endian::big>(ip));
    }

    CATCH_SECTION("Network ordered address is big-endian")
    {
        fly::net::IPv4Address::int_type const ip = 0x11'22'33'44;
        fly::net::IPv4Address const address(ip);

        CATCH_CHECK(address.network_order() == 0x11'22'33'44);
    }

    CATCH_SECTION("Host ordered address is little-endian")
    {
        fly::net::IPv4Address::int_type const ip = 0x11'22'33'44;
        fly::net::IPv4Address const address(ip);

        CATCH_CHECK(address.host_order() == fly::endian_swap_if_non_native<std::endian::big>(ip));
    }

    CATCH_SECTION("IPv4 addresses may be copied")
    {
        auto const address1 = fly::net::IPv4Address::in_addr_loopback();
        auto const address2 = address1;

        CATCH_CHECK(address1 == address2);
    }

    CATCH_SECTION("IPv4 addresses may be moved")
    {
        auto address1 = fly::net::IPv4Address::in_addr_loopback();
        auto const address2 = std::move(address1);

        CATCH_CHECK(address2 == fly::net::IPv4Address::in_addr_loopback());
    }

    CATCH_SECTION("IPv4 addresses may be compared")
    {
        auto const address1 = fly::net::IPv4Address::in_addr_any();
        auto const address2 = fly::net::IPv4Address::in_addr_loopback();
        auto const address3 = fly::net::IPv4Address::in_addr_broadcast();

        CATCH_CHECK(address2 == address2);
        CATCH_CHECK(address1 != address2);
        CATCH_CHECK(address1 < address2);
        CATCH_CHECK(address1 <= address2);
        CATCH_CHECK(address3 > address2);
        CATCH_CHECK(address3 >= address2);
    }

    CATCH_SECTION("IPv4 addresses may be used in constant expressions")
    {
        constexpr auto address1 = fly::net::IPv4Address::in_addr_any();
        constexpr auto address2 = fly::net::IPv4Address::in_addr_loopback();
        constexpr auto address3 = fly::net::IPv4Address::in_addr_broadcast();

        static_assert(address1.host_order() == INADDR_ANY);
        static_assert(address2.host_order() == INADDR_LOOPBACK);
        static_assert(address3.host_order() == INADDR_BROADCAST);

        static_assert(
            address1.network_order() ==
            fly::endian_swap_if_non_native<std::endian::big>(INADDR_ANY));
        static_assert(
            address2.network_order() ==
            fly::endian_swap_if_non_native<std::endian::big>(INADDR_LOOPBACK));
        static_assert(
            address3.network_order() ==
            fly::endian_swap_if_non_native<std::endian::big>(INADDR_BROADCAST));

        static_assert(address1 < address2);
        static_assert(address2 < address3);
    }

    CATCH_SECTION("IPv4 addresses may be converted to a four-octect string")
    {
        constexpr auto address1 = fly::net::IPv4Address::in_addr_any();
        constexpr auto address2 = fly::net::IPv4Address::in_addr_loopback();
        constexpr auto address3 = fly::net::IPv4Address::in_addr_broadcast();

        CATCH_CHECK(fly::string::format("{}", address1) == "0.0.0.0");
        CATCH_CHECK(fly::string::format("{}", address2) == "127.0.0.1");
        CATCH_CHECK(fly::string::format("{}", address3) == "255.255.255.255");
    }

    CATCH_SECTION("Single-part strings are parsed as 32-bit values")
    {
        auto const address1 = fly::net::IPv4Address::from_string("0");
        CATCH_REQUIRE(address1.has_value());
        CATCH_CHECK(address1->host_order() == INADDR_ANY);

        auto const address2 = fly::net::IPv4Address::from_string("2130706433");
        CATCH_REQUIRE(address2.has_value());
        CATCH_CHECK(address2->host_order() == INADDR_LOOPBACK);

        auto const address3 = fly::net::IPv4Address::from_string("4294967295");
        CATCH_REQUIRE(address3.has_value());
        CATCH_CHECK(address3->host_order() == INADDR_BROADCAST);
    }

    CATCH_SECTION("Two-part strings are parsed as an octet and a 24-bit value")
    {
        auto const address1 = fly::net::IPv4Address::from_string("0.0");
        CATCH_REQUIRE(address1.has_value());
        CATCH_CHECK(address1->host_order() == 0U);

        auto const address2 = fly::net::IPv4Address::from_string("127.1");
        CATCH_REQUIRE(address2.has_value());
        CATCH_CHECK(address2->host_order() == INADDR_LOOPBACK);

        auto const address3 = fly::net::IPv4Address::from_string("255.16777215");
        CATCH_REQUIRE(address3.has_value());
        CATCH_CHECK(address3->host_order() == INADDR_BROADCAST);
    }

    CATCH_SECTION("Three-part strings are parsed as two octet and a 16-bit value")
    {
        auto const address1 = fly::net::IPv4Address::from_string("0.0.0");
        CATCH_REQUIRE(address1.has_value());
        CATCH_CHECK(address1->host_order() == 0U);

        auto const address2 = fly::net::IPv4Address::from_string("127.0.1");
        CATCH_REQUIRE(address2.has_value());
        CATCH_CHECK(address2->host_order() == INADDR_LOOPBACK);

        auto const address3 = fly::net::IPv4Address::from_string("255.255.65535");
        CATCH_REQUIRE(address3.has_value());
        CATCH_CHECK(address3->host_order() == INADDR_BROADCAST);
    }

    CATCH_SECTION("Four-part strings are parsed as four octets")
    {
        auto const address1 = fly::net::IPv4Address::from_string("0.0.0.0");
        CATCH_REQUIRE(address1.has_value());
        CATCH_CHECK(address1->host_order() == 0U);

        auto const address2 = fly::net::IPv4Address::from_string("127.0.0.1");
        CATCH_REQUIRE(address2.has_value());
        CATCH_CHECK(address2->host_order() == INADDR_LOOPBACK);

        auto const address3 = fly::net::IPv4Address::from_string("255.255.255.255");
        CATCH_REQUIRE(address3.has_value());
        CATCH_CHECK(address3->host_order() == INADDR_BROADCAST);
    }

    CATCH_SECTION("String parsing fails if any octet is larger than 32 bits")
    {
        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("4294967296"));
        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("1.4294967296"));
        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("1.1.4294967296"));
        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("1.1.1.4294967296"));
    }

    CATCH_SECTION("String parsing fails if any leading octet is larger than 8 bits")
    {
        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("256.1"));

        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("256.1.1"));
        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("1.256.1"));

        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("256.1.1.1"));
        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("1.256.1.1"));
        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("1.1.256.1"));
    }

    CATCH_SECTION("String parsing fails if last octet is larger than the number of remaining bits")
    {
        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("4294967296"));
        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("1.16777216"));
        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("1.1.65536"));
        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("1.1.1.256"));
    }

    CATCH_SECTION("String parsing fails if a decimal number does not follow a decimal point")
    {
        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("1."));
        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("1.a"));

        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("1.1."));
        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("1.1.a"));

        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("1.1.1."));
        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("1.1.1.a"));
    }

    CATCH_SECTION("String parsing fails if entire string is not consumed")
    {
        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("0a"));
        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("0.0a"));
        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("0.0.0a"));
        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("0.0.0.0a"));
        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("0.0.0.0.123"));
    }

    CATCH_SECTION("String parsing fails if address does not begin with IPv4 address")
    {
        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("a0"));
        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("a0.0"));
        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("a0.0.0"));
        CATCH_CHECK_FALSE(fly::net::IPv4Address::from_string("a0.0.0.0"));
    }
}

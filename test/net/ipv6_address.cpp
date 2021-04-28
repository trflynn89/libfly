#include "fly/net/ipv6_address.hpp"

#include "fly/fly.hpp"

#include "catch2/catch_test_macros.hpp"

#if defined(FLY_LINUX) || defined(FLY_MACOS)
#    include <netinet/in.h>
#elif defined(FLY_WINDOWS)
#    include <WinSock2.h>
#    include <ws2ipdef.h>
#endif

CATCH_TEST_CASE("IPv6Address", "[net]")
{
    CATCH_SECTION("IN6ADDR_ANY should be equivalent to system value")
    {
        const auto address = fly::net::IPv6Address::in_addr_any();
        in6_addr system = in6addr_any;

        CATCH_CHECK(address == fly::net::IPv6Address(system.s6_addr));
    }

    CATCH_SECTION("IN6ADDR_LOOPBACK should be equivalent to system value")
    {
        const auto address = fly::net::IPv6Address::in_addr_loopback();
        in6_addr system = in6addr_loopback;

        CATCH_CHECK(address == fly::net::IPv6Address(system.s6_addr));
    }

    CATCH_SECTION("Default constructed IPv6 addresses are initialized to ::")
    {
        const fly::net::IPv6Address address;
        CATCH_CHECK(address == fly::net::IPv6Address::in_addr_any());
    }

    CATCH_SECTION("IPv6 addresses may be constructed from compatible array types")
    {
        const fly::net::IPv6Address::address_type address1 {};
        fly::net::IPv6Address::address_type address2 {};
        const fly::net::IPv6Address::address_type::value_type address3[16] {};

        const auto any = fly::net::IPv6Address::in_addr_any();

        CATCH_CHECK(fly::net::IPv6Address(address1) == any);
        CATCH_CHECK(fly::net::IPv6Address(std::move(address2)) == any);
        CATCH_CHECK(fly::net::IPv6Address(address3) == any);
    }

    CATCH_SECTION("IPv6 addresses may be copied")
    {
        const auto address1 = fly::net::IPv6Address::in_addr_loopback();
        const auto address2 = address1;

        CATCH_CHECK(address1 == address2);
    }

    CATCH_SECTION("IPv6 addresses may be moved")
    {
        auto address1 = fly::net::IPv6Address::in_addr_loopback();
        const auto address2 = std::move(address1);

        CATCH_CHECK(address2 == fly::net::IPv6Address::in_addr_loopback());
    }

    CATCH_SECTION("IPv6 addresses may be compared")
    {
        const auto address1 = fly::net::IPv6Address::in_addr_any();
        const auto address2 = fly::net::IPv6Address::in_addr_loopback();

        fly::net::IPv6Address::address_type data {};
        data.fill(0xff);
        const fly::net::IPv6Address address3(std::move(data));

        CATCH_CHECK(address2 == address2);
        CATCH_CHECK(address1 != address2);
        CATCH_CHECK(address1 < address2);
        CATCH_CHECK(address1 <= address2);
        CATCH_CHECK(address3 > address2);
        CATCH_CHECK(address3 >= address2);
    }

    CATCH_SECTION("IPv6 addresses may be used in constant expressions")
    {
        constexpr auto address1 = fly::net::IPv6Address::in_addr_any();
        constexpr auto address2 = fly::net::IPv6Address::in_addr_loopback();
        constexpr auto address3 =
            fly::net::IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff");

        static_assert(address1 < address2);
        static_assert(address2 < address3);
    }

    CATCH_SECTION("IPv6 addresses may be converted to a string")
    {
        const fly::net::IPv6Address address1({
            0xa1,
            0xa2,
            0xa3,
            0xa4,
            0xa5,
            0xa6,
            0xa7,
            0xa8,
            0xa9,
            0xb0,
            0xb1,
            0xb2,
            0xb3,
            0xb4,
            0xb5,
            0xb6,
        });

        CATCH_CHECK(
            fly::String::format("{}", address1) == "a1a2:a3a4:a5a6:a7a8:a9b0:b1b2:b3b4:b5b6");
    }

    CATCH_SECTION("IPv6 addresses may be copied to an array")
    {
        const fly::net::IPv6Address address1({1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6});

        fly::net::IPv6Address::address_type::value_type data[16] {};
        address1.copy(data);

        CATCH_CHECK(address1 == fly::net::IPv6Address(data));
    }

    CATCH_SECTION("IPv6 addresses may be converted to a string with leading zeros removed")
    {
        const fly::net::IPv6Address address1({1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6});
        CATCH_CHECK(fly::String::format("{}", address1) == "102:304:506:708:900:102:304:506");
    }

    CATCH_SECTION("IPv6 addresses may be converted to a string with consecutive zeros removed once")
    {
        const fly::net::IPv6Address address1({1, 2, 0, 0, 0, 0, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6});
        CATCH_CHECK(fly::String::format("{}", address1) == "102::708:900:102:304:506");

        const fly::net::IPv6Address address2({1, 2, 0, 0, 0, 0, 7, 8, 0, 0, 0, 0, 3, 4, 5, 6});
        CATCH_CHECK(fly::String::format("{}", address2) == "102::708:0:0:304:506");

        const fly::net::IPv6Address address3({1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 0, 0});
        CATCH_CHECK(fly::String::format("{}", address3) == "102:304:506:708:900:102:304::");

        const fly::net::IPv6Address address4({0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
        CATCH_CHECK(fly::String::format("{}", address4) == "1::");

        const fly::net::IPv6Address address5({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1});
        CATCH_CHECK(fly::String::format("{}", address5) == "::1");

        const fly::net::IPv6Address address6({0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1});
        CATCH_CHECK(fly::String::format("{}", address6) == "1::1");
    }

    CATCH_SECTION("IPv6 addresses may be created from full form strings")
    {
        const auto address1 = fly::net::IPv6Address::from_string("0:0:0:0:0:0:0:0");
        CATCH_REQUIRE(address1);
        CATCH_CHECK(
            *address1 == fly::net::IPv6Address({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}));

        const auto address2 = fly::net::IPv6Address::from_string("1:2:3:4:5:6:7:8");
        CATCH_REQUIRE(address2);
        CATCH_CHECK(
            *address2 == fly::net::IPv6Address({0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8}));

        const auto address3 =
            fly::net::IPv6Address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff");
        CATCH_REQUIRE(address3);
        CATCH_CHECK(
            *address3 ==
            fly::net::IPv6Address({
                0xff,
                0xff,
                0xff,
                0xff,
                0xff,
                0xff,
                0xff,
                0xff,
                0xff,
                0xff,
                0xff,
                0xff,
                0xff,
                0xff,
                0xff,
                0xff,
            }));
    }

    CATCH_SECTION("IPv6 addresses may be created from prefixed short form strings")
    {
        const auto address1 = fly::net::IPv6Address::from_string("::1");
        CATCH_REQUIRE(address1);
        CATCH_CHECK(
            *address1 == fly::net::IPv6Address({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}));

        const auto address2 = fly::net::IPv6Address::from_string("::1:2");
        CATCH_REQUIRE(address2);
        CATCH_CHECK(
            *address2 == fly::net::IPv6Address({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 2}));

        const auto address3 = fly::net::IPv6Address::from_string("::1:2:3");
        CATCH_REQUIRE(address3);
        CATCH_CHECK(
            *address3 == fly::net::IPv6Address({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 2, 0, 3}));

        const auto address4 = fly::net::IPv6Address::from_string("::1:2:3:4");
        CATCH_REQUIRE(address4);
        CATCH_CHECK(
            *address4 == fly::net::IPv6Address({0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 2, 0, 3, 0, 4}));

        const auto address5 = fly::net::IPv6Address::from_string("::1:2:3:4:5");
        CATCH_REQUIRE(address5);
        CATCH_CHECK(
            *address5 == fly::net::IPv6Address({0, 0, 0, 0, 0, 0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5}));

        const auto address6 = fly::net::IPv6Address::from_string("::1:2:3:4:5:6");
        CATCH_REQUIRE(address6);
        CATCH_CHECK(
            *address6 == fly::net::IPv6Address({0, 0, 0, 0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6}));

        const auto address7 = fly::net::IPv6Address::from_string("::1:2:3:4:5:6:7");
        CATCH_REQUIRE(address7);
        CATCH_CHECK(
            *address7 == fly::net::IPv6Address({0, 0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7}));
    }

    CATCH_SECTION("IPv6 addresses may be created from suffixed short form strings")
    {
        const auto address1 = fly::net::IPv6Address::from_string("1::");
        CATCH_REQUIRE(address1);
        CATCH_CHECK(
            *address1 == fly::net::IPv6Address({0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}));

        const auto address2 = fly::net::IPv6Address::from_string("1:2::");
        CATCH_REQUIRE(address2);
        CATCH_CHECK(
            *address2 == fly::net::IPv6Address({0, 1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}));

        const auto address3 = fly::net::IPv6Address::from_string("1:2:3::");
        CATCH_REQUIRE(address3);
        CATCH_CHECK(
            *address3 == fly::net::IPv6Address({0, 1, 0, 2, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}));

        const auto address4 = fly::net::IPv6Address::from_string("1:2:3:4::");
        CATCH_REQUIRE(address4);
        CATCH_CHECK(
            *address4 == fly::net::IPv6Address({0, 1, 0, 2, 0, 3, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0}));

        const auto address5 = fly::net::IPv6Address::from_string("1:2:3:4:5::");
        CATCH_REQUIRE(address5);
        CATCH_CHECK(
            *address5 == fly::net::IPv6Address({0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 0, 0, 0, 0, 0}));

        const auto address6 = fly::net::IPv6Address::from_string("1:2:3:4:5:6::");
        CATCH_REQUIRE(address6);
        CATCH_CHECK(
            *address6 == fly::net::IPv6Address({0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 0, 0, 0}));

        const auto address7 = fly::net::IPv6Address::from_string("1:2:3:4:5:6:7::");
        CATCH_REQUIRE(address7);
        CATCH_CHECK(
            *address7 == fly::net::IPv6Address({0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 0}));
    }

    CATCH_SECTION("IPv6 addresses may be created from mid-string short form strings")
    {
        const auto address1 = fly::net::IPv6Address::from_string("::");
        CATCH_REQUIRE(address1);
        CATCH_CHECK(
            *address1 == fly::net::IPv6Address({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}));

        const auto address2 = fly::net::IPv6Address::from_string("1::1");
        CATCH_REQUIRE(address2);
        CATCH_CHECK(
            *address2 == fly::net::IPv6Address({0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}));

        const auto address3 = fly::net::IPv6Address::from_string("1:2::3");
        CATCH_REQUIRE(address3);
        CATCH_CHECK(
            *address3 == fly::net::IPv6Address({0, 1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3}));

        const auto address4 = fly::net::IPv6Address::from_string("1::2:3");
        CATCH_REQUIRE(address4);
        CATCH_CHECK(
            *address4 == fly::net::IPv6Address({0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 3}));

        const auto address5 = fly::net::IPv6Address::from_string("1:2:3::4");
        CATCH_REQUIRE(address5);
        CATCH_CHECK(
            *address5 == fly::net::IPv6Address({0, 1, 0, 2, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4}));

        const auto address6 = fly::net::IPv6Address::from_string("1:2::3:4");
        CATCH_REQUIRE(address6);
        CATCH_CHECK(
            *address6 == fly::net::IPv6Address({0, 1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 4}));

        const auto address7 = fly::net::IPv6Address::from_string("1::2:3:4");
        CATCH_REQUIRE(address7);
        CATCH_CHECK(
            *address7 == fly::net::IPv6Address({0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 3, 0, 4}));
    }

    CATCH_SECTION("String parsing fails if short form is used more than once")
    {
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("1::1::"));
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("1::1::1"));
    }

    CATCH_SECTION("String parsing fails if string is not full or short form")
    {
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("1"));
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("1:1"));
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("1:1:1"));
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("1:1:1:1"));
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("1:1:1:1:1"));
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("1:1:1:1:1:1"));
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("1:1:1:1:1:1:1"));
    }

    CATCH_SECTION("String parsing fails if any segment is larger than 16 bits")
    {
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("1ffff:0:0:0:0:0:0:0"));
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("0:1ffff:0:0:0:0:0:0"));
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("0:0:1ffff:0:0:0:0:0"));
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("0:0:0:1ffff:0:0:0:0"));
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("0:0:0:0:1ffff:0:0:0"));
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("0:0:0:0:0:1ffff:0:0"));
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("0:0:0:0:0:0:1ffff:0"));
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("0:0:0:0:0:0:0:1ffff"));
    }

    CATCH_SECTION("String parsing fails if any segment is not a hexadecimal number")
    {
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("x:0:0:0:0:0:0:0"));
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("0:x:0:0:0:0:0:0"));
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("0:0:x:0:0:0:0:0"));
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("0:0:0:x:0:0:0:0"));
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("0:0:0:0:x:0:0:0"));
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("0:0:0:0:0:x:0:0"));
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("0:0:0:0:0:0:x:0"));
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("0:0:0:0:0:0:0:x"));
    }

    CATCH_SECTION("String parsing fails if entire string is not consumed")
    {
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("0:0:0:0:0:0:0:0:0"));
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("0:0:0:0:0:0:0:0xy"));
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("::0xy"));
    }

    CATCH_SECTION("String parsing fails if address does not begin with IPv6 address")
    {
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("xy0:0:0:0:0:0:0:0"));
        CATCH_CHECK_FALSE(fly::net::IPv6Address::from_string("xy::0"));
    }
}

#include "fly/coders/base64/base64_coder.hpp"

#include "test/util/path_util.hpp"

#include <catch2/catch.hpp>

#include <cctype>
#include <filesystem>

TEST_CASE("Base64", "[coders]")
{
    fly::Base64Coder coder;

    SECTION("Encode and decode empty stream")
    {
        const std::string raw;
        std::string enc, dec;

        REQUIRE(coder.encode_string(raw, enc));
        REQUIRE(coder.decode_string(enc, dec));

        CHECK(enc.empty());
        CHECK(raw == dec);
    }

    SECTION("Encode and decode a stream without padding")
    {
        const std::string raw = "Man";
        std::string enc, dec;

        REQUIRE(coder.encode_string(raw, enc));
        REQUIRE(coder.decode_string(enc, dec));

        CHECK(enc == "TWFu");
        CHECK(raw == dec);
    }

    SECTION("Encode and decode a stream with one padding symbol")
    {
        const std::string raw = "Ma";
        std::string enc, dec;

        REQUIRE(coder.encode_string(raw, enc));
        REQUIRE(coder.decode_string(enc, dec));

        CHECK(enc == "TWE=");
        CHECK(raw == dec);
    }

    SECTION("Encode and decode a stream with two padding symbols")
    {
        const std::string raw = "M";
        std::string enc, dec;

        REQUIRE(coder.encode_string(raw, enc));
        REQUIRE(coder.decode_string(enc, dec));

        CHECK(enc == "TQ==");
        CHECK(raw == dec);
    }

    SECTION("Cannot decode streams with invalid symbols")
    {
        for (char ch = 0x00; ch >= 0x00; ++ch)
        {
            if ((ch == '+') || (ch == '/') || (ch == '=') || std::isalnum(ch))
            {
                continue;
            }

            const std::string test(1, ch);
            const std::string fill("a");
            std::string dec;

            CHECK_FALSE(coder.decode_string(test + test + test + test, dec));
            CHECK_FALSE(coder.decode_string(test + test + test + fill, dec));
            CHECK_FALSE(coder.decode_string(test + test + fill + test, dec));
            CHECK_FALSE(coder.decode_string(test + test + fill + fill, dec));
            CHECK_FALSE(coder.decode_string(test + fill + test + test, dec));
            CHECK_FALSE(coder.decode_string(test + fill + test + fill, dec));
            CHECK_FALSE(coder.decode_string(test + fill + fill + test, dec));
            CHECK_FALSE(coder.decode_string(test + fill + fill + fill, dec));
            CHECK_FALSE(coder.decode_string(fill + test + test + test, dec));
            CHECK_FALSE(coder.decode_string(fill + test + test + fill, dec));
            CHECK_FALSE(coder.decode_string(fill + test + fill + test, dec));
            CHECK_FALSE(coder.decode_string(fill + test + fill + fill, dec));
            CHECK_FALSE(coder.decode_string(fill + fill + test + test, dec));
            CHECK_FALSE(coder.decode_string(fill + fill + test + fill, dec));
            CHECK_FALSE(coder.decode_string(fill + fill + fill + test, dec));
        }
    }

    SECTION("Cannot decode streams with invalid chunk sizes")
    {
        std::string dec;

        CHECK_FALSE(coder.decode_string("a", dec));
        CHECK_FALSE(coder.decode_string("ab", dec));
        CHECK_FALSE(coder.decode_string("abc", dec));

        CHECK_FALSE(coder.decode_string("abcde", dec));
        CHECK_FALSE(coder.decode_string("abcdef", dec));
        CHECK_FALSE(coder.decode_string("abcdefg", dec));
    }

    SECTION("Example from Wikipedia: https://en.wikipedia.org/wiki/Base64#Examples")
    {
        const std::string raw =
            "Man is distinguished, not only by his reason, but by this singular passion from other "
            "animals, which is a lust of the mind, that by a perseverance of delight in the "
            "continued and indefatigable generation of knowledge, exceeds the short vehemence of "
            "any carnal pleasure.";
        std::string enc, dec;

        REQUIRE(coder.encode_string(raw, enc));
        REQUIRE(coder.decode_string(enc, dec));

        const std::string expected =
            "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlzIHNpbmd1bG"
            "FyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQg"
            "YnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIG"
            "dlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5h"
            "bCBwbGVhc3VyZS4=";

        CHECK(enc == expected);
        CHECK(raw == dec);
    }

    SECTION("File tests")
    {
        fly::test::PathUtil::ScopedTempDirectory path;
        std::filesystem::path encoded_file = path.file();
        std::filesystem::path decoded_file = path.file();

        SECTION("Encode and decode a large file containing only ASCII symbols")
        {
            // Generated with:
            // tr -dc '[:graph:]' </dev/urandom | head -c 4194304 > test.txt
            const auto here = std::filesystem::path(__FILE__).parent_path();
            const auto raw = here / "data" / "test.txt";

            REQUIRE(coder.encode_file(raw, encoded_file));
            REQUIRE(coder.decode_file(encoded_file, decoded_file));

            // Generated with:
            // base64 -w0 test.txt > test.txt.base64
            const auto expected = here / "data" / "test.txt.base64";

            CHECK(fly::test::PathUtil::compare_files(encoded_file, expected));
            CHECK(fly::test::PathUtil::compare_files(raw, decoded_file));
        }

        SECTION("Encode and decode a PNG image file")
        {
            const auto here = std::filesystem::path(__FILE__).parent_path();
            const auto raw = here / "data" / "test.png";

            REQUIRE(coder.encode_file(raw, encoded_file));
            REQUIRE(coder.decode_file(encoded_file, decoded_file));

            // Generated with:
            // base64 -w0 test.png > test.png.base64
            const auto expected = here / "data" / "test.png.base64";

            CHECK(fly::test::PathUtil::compare_files(encoded_file, expected));
            CHECK(fly::test::PathUtil::compare_files(raw, decoded_file));
        }

        SECTION("Encode and decode a GIF image file")
        {
            const auto here = std::filesystem::path(__FILE__).parent_path();
            const auto raw = here / "data" / "test.gif";

            REQUIRE(coder.encode_file(raw, encoded_file));
            REQUIRE(coder.decode_file(encoded_file, decoded_file));

            // Generated with:
            // base64 -w0 test.gif > test.gif.base64
            const auto expected = here / "data" / "test.gif.base64";

            CHECK(fly::test::PathUtil::compare_files(encoded_file, expected));
            CHECK(fly::test::PathUtil::compare_files(raw, decoded_file));
        }
    }
}

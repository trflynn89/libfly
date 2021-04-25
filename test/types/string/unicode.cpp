#include "fly/types/string/literals.hpp"
#include "fly/types/string/string.hpp"

#include "catch2/catch_template_test_macros.hpp"
#include "catch2/catch_test_macros.hpp"

#include <optional>
#include <vector>

CATCH_TEMPLATE_TEST_CASE("BasicUnicode", "[string]", char, wchar_t, char8_t, char16_t, char32_t)
{
    using char_type = TestType;
    using string_type = std::basic_string<char_type>;
    using BasicString = fly::BasicString<string_type>;
    using codepoint_type = typename BasicString::codepoint_type;

    auto make_string = [](std::vector<codepoint_type> &&bytes) -> string_type
    {
        string_type result;
        result.reserve(bytes.size());

        for (codepoint_type ch : bytes)
        {
            result.push_back(static_cast<char_type>(ch));
        }

        return result;
    };

    auto make_escaped_unicode_string = [](auto value) -> string_type
    {
        return BasicString::format(FLY_ARR(char_type, "\\u{:04x}"), value);
    };

    auto escape_should_fail = [](string_type &&test, int line)
    {
        CATCH_CAPTURE(test);
        CATCH_CAPTURE(line);

        auto begin = test.cbegin();
        const auto end = test.cend();

        CATCH_CHECK_FALSE(BasicString::validate(test));
        CATCH_CHECK_FALSE(BasicString::escape_codepoint(begin, end));
        CATCH_CHECK_FALSE(BasicString::escape_all_codepoints(test));
    };

    auto encode_should_fail = [&](string_type &&test, codepoint_type codepoint, int line)
    {
        escape_should_fail(std::move(test), line);
        CATCH_CHECK_FALSE(BasicString::encode_codepoint(codepoint));
    };

    auto unescape_should_fail = [](string_type &&test, int line, bool whole_string = true)
    {
        CATCH_CAPTURE(test);
        CATCH_CAPTURE(line);

        auto begin = test.cbegin();
        const auto end = test.cend();

        CATCH_CHECK_FALSE(BasicString::unescape_codepoint(begin, end));

        if (whole_string)
        {
            CATCH_CHECK_FALSE(BasicString::unescape_all_codepoints(test));
        }
    };

    CATCH_SECTION("Empty strings as input")
    {
        string_type test;
        std::optional<string_type> actual;

        auto begin = test.cbegin();
        const auto end = test.cend();

        CATCH_CHECK(BasicString::validate(test));
        CATCH_CHECK_FALSE(BasicString::decode_codepoint(begin, end));

        actual = BasicString::escape_all_codepoints(test);
        CATCH_CHECK(actual == test);

        begin = test.cbegin();
        CATCH_CHECK_FALSE(BasicString::escape_codepoint(begin, end));

        actual = BasicString::unescape_all_codepoints(test);
        CATCH_CHECK(actual == test);

        begin = test.cbegin();
        CATCH_CHECK_FALSE(BasicString::unescape_codepoint(begin, end));
    }

    CATCH_SECTION("Past-the-end iterators as input")
    {
        string_type test;

        auto begin = test.cend();
        const auto end = test.cend();

        CATCH_CHECK_FALSE(BasicString::decode_codepoint(begin, end));
        CATCH_CHECK_FALSE(BasicString::escape_codepoint(begin, end));
        CATCH_CHECK_FALSE(BasicString::unescape_codepoint(begin, end));
    }

    CATCH_SECTION("Not enough data to encode")
    {
        if constexpr (sizeof(char_type) == 1)
        {
            // First byte of U+1f355.
            escape_should_fail(make_string({0xf0}), __LINE__);

            // First two bytes of U+1f355.
            escape_should_fail(make_string({0xf0, 0x9f}), __LINE__);

            // First three bytes of U+1f355.
            escape_should_fail(make_string({0xf0, 0x9f, 0x8d}), __LINE__);
        }
        else if constexpr (sizeof(char_type) == 2)
        {
            // High surrogate for U+1f355.
            escape_should_fail(make_string({0xd83c}), __LINE__);
        }
        else if constexpr (sizeof(char_type) == 4)
        {
            // UTF-32 encoding really only fails if there is no data.
            string_type test;
            auto begin = test.cend();
            const auto end = test.cend();

            CATCH_CHECK_FALSE(BasicString::escape_codepoint(begin, end));
        }
    }

    CATCH_SECTION("Not enough data to decode")
    {
        unescape_should_fail(FLY_STR(char_type, "\\u"), __LINE__);
        unescape_should_fail(FLY_STR(char_type, "\\u0"), __LINE__);
        unescape_should_fail(FLY_STR(char_type, "\\u00"), __LINE__);
        unescape_should_fail(FLY_STR(char_type, "\\u000"), __LINE__);

        unescape_should_fail(FLY_STR(char_type, "\\ud800\\u"), __LINE__);
        unescape_should_fail(FLY_STR(char_type, "\\ud800\\u0"), __LINE__);
        unescape_should_fail(FLY_STR(char_type, "\\ud800\\u00"), __LINE__);
        unescape_should_fail(FLY_STR(char_type, "\\ud800\\u000"), __LINE__);

        unescape_should_fail(FLY_STR(char_type, "\\U"), __LINE__);
        unescape_should_fail(FLY_STR(char_type, "\\U0"), __LINE__);
        unescape_should_fail(FLY_STR(char_type, "\\U00"), __LINE__);
        unescape_should_fail(FLY_STR(char_type, "\\U000"), __LINE__);
        unescape_should_fail(FLY_STR(char_type, "\\U0000"), __LINE__);
        unescape_should_fail(FLY_STR(char_type, "\\U00000"), __LINE__);
        unescape_should_fail(FLY_STR(char_type, "\\U000000"), __LINE__);
        unescape_should_fail(FLY_STR(char_type, "\\U0000000"), __LINE__);
    }

    if constexpr (sizeof(char_type) == 1)
    {
        CATCH_SECTION("UTF-8")
        {
            CATCH_SECTION("Invalid leading byte")
            {
                string_type test = make_string({0xff});

                auto begin = test.cbegin();
                const auto end = test.cend();

                CATCH_CHECK_FALSE(BasicString::validate(test));
                CATCH_CHECK_FALSE(BasicString::escape_codepoint(begin, end));
                CATCH_CHECK_FALSE(BasicString::escape_all_codepoints(test));
            }

            CATCH_SECTION("Invalid continuation byte")
            {
                // Second byte of U+1f355 masked with 0b0011'1111.
                escape_should_fail(make_string({0xf0, 0x1f, 0x8d, 0x9f}), __LINE__);

                // Third byte of U+1f355 masked with 0b0011'1111.
                escape_should_fail(make_string({0xf0, 0x9f, 0x0d, 0x9f}), __LINE__);

                // Fourth byte of U+1f355 masked with 0b0011'1111.
                escape_should_fail(make_string({0xf0, 0x9f, 0x8d, 0x1f}), __LINE__);
            }

            CATCH_SECTION("Overlong encoding")
            {
                // U+0021 2-byte overlong encoding.
                escape_should_fail(make_string({0xc0, 0xa1}), __LINE__);

                // U+0021 3-byte overlong encoding.
                escape_should_fail(make_string({0xe0, 0x80, 0xa1}), __LINE__);

                // U+0021 4-byte overlong encoding.
                escape_should_fail(make_string({0xf0, 0x80, 0x80, 0xa1}), __LINE__);
            }
        }
    }

    if constexpr (sizeof(char_type) == 2)
    {
        CATCH_SECTION("UTF-16")
        {
            CATCH_SECTION("Invalid surrogates")
            {
                // Low surrogate only.
                for (char_type ch = 0xdc00; ch <= 0xdfff; ++ch)
                {
                    escape_should_fail(make_string({ch}), __LINE__);
                }

                // High surrogate only.
                for (char_type ch = 0xd800; ch <= 0xdbff; ++ch)
                {
                    escape_should_fail(make_string({ch}), __LINE__);
                }

                // High surrogate followed by non-surrogate.
                for (char_type ch = 0xd800; ch <= 0xdbff; ++ch)
                {
                    string_type high_surrogate = make_string({ch});
                    string_type low_surrogate = make_string({0});

                    escape_should_fail(high_surrogate + low_surrogate, __LINE__);
                }

                // High surrogate followed by high surrogate.
                for (char_type ch = 0xd800; ch <= 0xdbff; ++ch)
                {
                    string_type high_surrogate = make_string({ch});

                    escape_should_fail(high_surrogate + high_surrogate, __LINE__);
                }
            }
        }
    }

    CATCH_SECTION("Invalid codepoints")
    {
        CATCH_SECTION("Reserved codepoints")
        {
            for (codepoint_type ch = 0xd800; ch <= 0xdfff; ++ch)
            {
                if constexpr (sizeof(char_type) == 1)
                {
                    string_type test = make_string({
                        0xe0 | (ch >> 12),
                        0x80 | ((ch >> 6) & 0x3f),
                        0x80 | (ch & 0x3f),
                    });

                    encode_should_fail(std::move(test), ch, __LINE__);
                }
                else
                {
                    // Note: UTF-16 doesn't actually hit the reserved codepoint error because the
                    // reserved codepoints are invalid alone, and thus fail earlier.
                    encode_should_fail(make_string({ch}), ch, __LINE__);
                }
            }
        }

        CATCH_SECTION("Out-of-range codepoints")
        {
            // Iterating all the way to numeric_limits<char_type>::max() takes way too long.
            for (codepoint_type ch = 0x110000; ch <= 0x1100ff; ++ch)
            {
                if constexpr (sizeof(char_type) == 1)
                {
                    string_type test = make_string({
                        0xf0 | (ch >> 18),
                        0x80 | ((ch >> 12) & 0x3f),
                        0x80 | ((ch >> 6) & 0x3f),
                        0x80 | (ch & 0x3f),
                    });

                    encode_should_fail(std::move(test), ch, __LINE__);
                }
                else if constexpr (sizeof(char_type) == 2)
                {
                    // Note: UTF-16 doesn't actually hit the out-of-range error because the
                    // out-of-range codepoints are invalid surrogates, and thus fail earlier.
                    string_type test = make_string({
                        0xd800 | ((ch - 0x10000) >> 10),
                        0xdc00 | ((ch - 0x10000) & 0x3ff),
                    });

                    encode_should_fail(std::move(test), ch, __LINE__);
                }
                else if constexpr (sizeof(char_type) == 4)
                {
                    encode_should_fail(make_string({ch}), ch, __LINE__);
                }
            }
        }
    }

    CATCH_SECTION("ASCII")
    {
        auto encoded_to = [&](codepoint_type ch, string_type &&expected)
        {
            CATCH_CAPTURE(ch);

            const string_type test = make_string({ch});
            std::optional<string_type> actual;

            CATCH_CHECK(BasicString::validate(test));

            actual = BasicString::encode_codepoint(ch);
            CATCH_CHECK(actual == test);

            {
                auto begin = test.cbegin();
                const auto end = test.cend();

                actual = BasicString::template escape_codepoint<'u'>(begin, end);
                CATCH_CHECK(actual == expected);

                actual = BasicString::template escape_all_codepoints<'u'>(test);
                CATCH_CHECK(actual == expected);
            }
            {
                auto begin = test.cbegin();
                const auto end = test.cend();

                actual = BasicString::template escape_codepoint<'U'>(begin, end);
                CATCH_CHECK(actual == expected);

                actual = BasicString::template escape_all_codepoints<'U'>(test);
                CATCH_CHECK(actual == expected);
            }
        };

        CATCH_SECTION("Printable ASCII never encoded")
        {
            for (codepoint_type ch = 0x20; ch < 0x7f; ++ch)
            {
                encoded_to(ch, make_string({ch}));
            }
        }

        CATCH_SECTION("Non-printable ASCII always encoded")
        {
            for (codepoint_type ch = 0; ch < 0x20; ++ch)
            {
                string_type expected = make_escaped_unicode_string(ch);
                encoded_to(ch, std::move(expected));
            }

            string_type expected = make_escaped_unicode_string(0x7f);
            encoded_to(0x7f, std::move(expected));
        }
    }

    CATCH_SECTION("Non-ASCII")
    {
        auto escaped_to =
            [](string_type &&test, string_type &&expected, auto prefix, bool one_char = true)
        {
            CATCH_CAPTURE(test);

            CATCH_CHECK(BasicString::validate(test));
            CATCH_CHECK(BasicString::validate(expected));

            auto begin = test.cbegin();
            const auto end = test.cend();
            CATCH_CAPTURE(std::distance(begin, end));

            std::optional<string_type> actual;

            if (one_char)
            {
                actual = BasicString::template escape_codepoint<prefix()>(begin, end);
                CATCH_CHECK(actual == expected);
            }

            actual = BasicString::template escape_all_codepoints<prefix()>(test);
            CATCH_CHECK(actual == expected);
        };

        CATCH_SECTION("Escape non-ASCII with 'u'")
        {
            auto u = []() constexpr->char
            {
                return 'u';
            };

            escaped_to(FLY_STR(char_type, "\U00010000"), FLY_STR(char_type, "\\ud800\\udc00"), u);
            escaped_to(FLY_STR(char_type, "\U00010e6d"), FLY_STR(char_type, "\\ud803\\ude6d"), u);
            escaped_to(FLY_STR(char_type, "\U0001d11e"), FLY_STR(char_type, "\\ud834\\udd1e"), u);
            escaped_to(FLY_STR(char_type, "\U0001f355"), FLY_STR(char_type, "\\ud83c\\udf55"), u);
            escaped_to(FLY_STR(char_type, "\U0010ffff"), FLY_STR(char_type, "\\udbff\\udfff"), u);

            escaped_to(
                FLY_STR(char_type, "All ASCII!"),
                FLY_STR(char_type, "All ASCII!"),
                u,
                false);

            escaped_to(
                FLY_STR(char_type, "\U0001f355 in the morning, \U0001f355 in the evening"),
                FLY_STR(char_type, "\\ud83c\\udf55 in the morning, \\ud83c\\udf55 in the evening"),
                u,
                false);
        }

        CATCH_SECTION("Escape non-ASCII with 'U'")
        {
            auto u = []() constexpr->char
            {
                return 'U';
            };

            escaped_to(FLY_STR(char_type, "\U00010000"), FLY_STR(char_type, "\\U00010000"), u);
            escaped_to(FLY_STR(char_type, "\U00010e6d"), FLY_STR(char_type, "\\U00010e6d"), u);
            escaped_to(FLY_STR(char_type, "\U0001d11e"), FLY_STR(char_type, "\\U0001d11e"), u);
            escaped_to(FLY_STR(char_type, "\U0001f355"), FLY_STR(char_type, "\\U0001f355"), u);
            escaped_to(FLY_STR(char_type, "\U0010ffff"), FLY_STR(char_type, "\\U0010ffff"), u);

            escaped_to(
                FLY_STR(char_type, "All ASCII!"),
                FLY_STR(char_type, "All ASCII!"),
                u,
                false);

            escaped_to(
                FLY_STR(char_type, "\U0001f355 in the morning, \U0001f355 in the evening"),
                FLY_STR(char_type, "\\U0001f355 in the morning, \\U0001f355 in the evening"),
                u,
                false);
        }
    }

    CATCH_SECTION("Invalid escape sequences")
    {
        CATCH_SECTION("Non-Unicode escape sequences")
        {
            unescape_should_fail(FLY_STR(char_type, "f"), __LINE__, false);
            unescape_should_fail(FLY_STR(char_type, "\\f"), __LINE__, false);
        }

        CATCH_SECTION("Non-hexadecimal escape sequences")
        {
            unescape_should_fail(FLY_STR(char_type, "\\u000z"), __LINE__);
            unescape_should_fail(FLY_STR(char_type, "\\ud800\\u000z"), __LINE__);
            unescape_should_fail(FLY_STR(char_type, "\\U0000000z"), __LINE__);
        }

        CATCH_SECTION("Invalid surrogates")
        {
            // Low surrogate only.
            for (codepoint_type ch = 0xdc00; ch <= 0xdfff; ++ch)
            {
                unescape_should_fail(make_escaped_unicode_string(ch), __LINE__);
            }

            // High surrogate only.
            for (codepoint_type ch = 0xd800; ch <= 0xdbff; ++ch)
            {
                unescape_should_fail(make_escaped_unicode_string(ch), __LINE__);
            }

            // High surrogate followed by non-surrogate.
            for (codepoint_type ch = 0xd800; ch <= 0xdbff; ++ch)
            {
                string_type high_surrogate(make_escaped_unicode_string(ch));
                string_type low_surrogate(make_escaped_unicode_string(0));

                unescape_should_fail(high_surrogate + low_surrogate, __LINE__);
            }

            // High surrogate followed by high surrogate.
            for (codepoint_type ch = 0xd800; ch <= 0xdbff; ++ch)
            {
                string_type high_surrogate(make_escaped_unicode_string(ch));

                unescape_should_fail(high_surrogate + high_surrogate, __LINE__);
            }
        }
    }

    CATCH_SECTION("Valid escape sequences")
    {
        auto unescaped_to = [](string_type &&test, string_type &&expected, bool one_char = true)
        {
            CATCH_CAPTURE(test);

            CATCH_CHECK(BasicString::validate(test));
            CATCH_CHECK(BasicString::validate(expected));

            auto begin = test.cbegin();
            const auto end = test.cend();

            std::optional<string_type> actual;

            if (one_char)
            {
                actual = BasicString::unescape_codepoint(begin, end);
                CATCH_CHECK(actual == expected);
            }

            actual = BasicString::unescape_all_codepoints(test);
            CATCH_CHECK(actual == expected);
        };

        CATCH_SECTION("Single escaped codepoint")
        {
            unescaped_to(FLY_STR(char_type, "\\u0040"), FLY_STR(char_type, "\u0040"));
            unescaped_to(FLY_STR(char_type, "\\u007a"), FLY_STR(char_type, "\u007a"));
            unescaped_to(FLY_STR(char_type, "\\u007a"), FLY_STR(char_type, "\u007a"));
            unescaped_to(FLY_STR(char_type, "\\u00c4"), FLY_STR(char_type, "\u00c4"));
            unescaped_to(FLY_STR(char_type, "\\u00e4"), FLY_STR(char_type, "\u00e4"));
            unescaped_to(FLY_STR(char_type, "\\u0298"), FLY_STR(char_type, "\u0298"));
            unescaped_to(FLY_STR(char_type, "\\u0800"), FLY_STR(char_type, "\u0800"));
            unescaped_to(FLY_STR(char_type, "\\uffff"), FLY_STR(char_type, "\uffff"));

            unescaped_to(FLY_STR(char_type, "All ASCII!"), FLY_STR(char_type, "All ASCII!"), false);
            unescaped_to(
                FLY_STR(char_type, "Other escape \t"),
                FLY_STR(char_type, "Other escape \t"),
                false);
            unescaped_to(
                FLY_STR(char_type, "Other escape \\t"),
                FLY_STR(char_type, "Other escape \\t"),
                false);
        }

        CATCH_SECTION("Escaped surrogate pairs")
        {
            unescaped_to(FLY_STR(char_type, "\\ud800\\udc00"), FLY_STR(char_type, "\U00010000"));
            unescaped_to(FLY_STR(char_type, "\\ud803\\ude6d"), FLY_STR(char_type, "\U00010e6d"));
            unescaped_to(FLY_STR(char_type, "\\ud834\\udd1e"), FLY_STR(char_type, "\U0001d11e"));
            unescaped_to(FLY_STR(char_type, "\\udbff\\udfff"), FLY_STR(char_type, "\U0010ffff"));

            unescaped_to(
                FLY_STR(char_type, "\\ud83c\\udf55 in the morning, \\ud83c\\udf55 in the evening"),
                FLY_STR(char_type, "\U0001f355 in the morning, \U0001f355 in the evening"),
                false);
        }

        CATCH_SECTION("Long form escaped codepoint")
        {
            unescaped_to(FLY_STR(char_type, "\\U00010000"), FLY_STR(char_type, "\U00010000"));
            unescaped_to(FLY_STR(char_type, "\\U00010e6d"), FLY_STR(char_type, "\U00010e6d"));
            unescaped_to(FLY_STR(char_type, "\\U0001d11e"), FLY_STR(char_type, "\U0001d11e"));
            unescaped_to(FLY_STR(char_type, "\\U0010ffff"), FLY_STR(char_type, "\U0010ffff"));

            unescaped_to(
                FLY_STR(char_type, "\\U0001f355 in the morning, \\U0001f355 in the evening"),
                FLY_STR(char_type, "\U0001f355 in the morning, \U0001f355 in the evening"),
                false);
        }
    }

    if constexpr (sizeof(char_type) == 1)
    {
        // http://www.cl.cam.ac.uk/~mgk25/ucs/examples/UTF-8-test.txt
        //
        // Note: Any test of 5- or 6-byte sequences have been removed here. BasicString only
        // supports up to 4-byte UTF-8 sequences (Unicode planes 1 - 16). The 5- and 6- byte
        // sequences indeed fail, but not for the reasons the stress test expects. See:
        // https://unicode.org/mail-arch/unicode-ml/Archives-Old/UML018/0332.html
        CATCH_SECTION("Markus Kuhn UTF-8 decoder capability and stress test")
        {
            auto validate_pass =
                [&](std::vector<codepoint_type> &&bytes, codepoint_type expected, int line)
            {
                string_type test = make_string(std::move(bytes));

                CATCH_CAPTURE(test);
                CATCH_CAPTURE(line);

                CATCH_CHECK(BasicString::validate(test));

                auto it = test.cbegin();
                const auto end = test.cend();

                std::optional<codepoint_type> actual = BasicString::decode_codepoint(it, end);
                CATCH_CHECK(actual == expected);
            };

            auto validate_pass_all =
                [](string_type &&test, std::vector<codepoint_type> expected, int line)
            {
                CATCH_CAPTURE(test);
                CATCH_CAPTURE(line);

                CATCH_CHECK(BasicString::validate(test));

                std::size_t index = 0;

                auto it = test.cbegin();
                const auto end = test.cend();

                for (; (it != end) && (index < expected.size()); ++index)
                {
                    std::optional<codepoint_type> actual = BasicString::decode_codepoint(it, end);
                    CATCH_CHECK(actual == expected[index]);
                }

                CATCH_CHECK(index == expected.size());
                CATCH_CHECK(it == end);
            };

            auto validate_fail_str = [](string_type &&test, std::size_t expected_failures, int line)
            {
                CATCH_CAPTURE(test);
                CATCH_CAPTURE(line);

                CATCH_CHECK_FALSE(BasicString::validate(test));

                const auto end = test.cend();
                std::size_t actual = 0;

                for (auto it = test.cbegin(); it != end;)
                {
                    if (!BasicString::decode_codepoint(it, end))
                    {
                        ++actual;
                    }
                }

                CATCH_CHECK(actual == expected_failures);
            };

            auto validate_fail =
                [&](std::vector<codepoint_type> &&bytes, std::size_t expected_failures, int line)
            {
                validate_fail_str(make_string(std::move(bytes)), expected_failures, line);
            };

            CATCH_SECTION("1  Some correct UTF-8 text")
            {
                validate_pass_all(
                    FLY_STR(char_type, "κόσμε"),
                    {0x03ba, 0x1f79, 0x03c3, 0x03bc, 0x03b5},
                    __LINE__);
            }

            CATCH_SECTION("2  Boundary condition test cases")
            {
                CATCH_SECTION("2.1  First possible sequence of a certain length")
                {
                    // 2.1.1  1 byte  (U-00000000)
                    validate_pass({0x00}, 0x0000, __LINE__);

                    // 2.1.2  2 bytes (U-00000080)
                    validate_pass({0xc2, 0x80}, 0x0080, __LINE__);

                    // 2.1.3  3 bytes (U-00000800)
                    validate_pass({0xe0, 0xa0, 0x80}, 0x0800, __LINE__);

                    // 2.1.4  4 bytes (U-00010000)
                    validate_pass({0xf0, 0x90, 0x80, 0x80}, 0x10000, __LINE__);
                }

                CATCH_SECTION("2.2  Last possible sequence of a certain length")
                {
                    // 2.2.1  1 byte  (U-0000007F)
                    validate_pass({0x7f}, 0x007f, __LINE__);

                    // 2.2.2  2 bytes (U-000007FF)
                    validate_pass({0xdf, 0xbf}, 0x07ff, __LINE__);

                    // 2.2.3  3 bytes (U-0000FFFF)
                    validate_pass({0xef, 0xbf, 0xbf}, 0xffff, __LINE__);

                    // 2.1.4  4 bytes (U-001FFFFF)
                    validate_fail({0xf7, 0xbf, 0xbf, 0xbf}, 1, __LINE__);
                }

                CATCH_SECTION("2.3  Other boundary conditions")
                {
                    // 2.3.1  U-0000D7FF = ed 9f bf
                    validate_pass({0xed, 0x9f, 0xbf}, 0xd7ff, __LINE__);

                    // 2.3.2  U-0000E000 = ee 80 80
                    validate_pass({0xee, 0x80, 0x80}, 0xe000, __LINE__);

                    // 2.3.3  U-0000FFFD = ef bf bd
                    validate_pass({0xef, 0xbf, 0xbd}, 0xfffd, __LINE__);

                    // 2.3.4  U-0010FFFF = f4 8f bf bf
                    validate_pass({0xf4, 0x8f, 0xbf, 0xbf}, 0x10ffff, __LINE__);

                    // 2.3.5  U-00110000 = f4 90 80 80
                    validate_fail({0xf4, 0x90, 0x80, 0x80}, 1, __LINE__);
                }
            }

            CATCH_SECTION("3  Malformed sequences")
            {
                CATCH_SECTION("3.1  Unexpected continuation bytes")
                {
                    // 3.1.1  First continuation byte 0x80
                    validate_fail({0x80}, 1, __LINE__);

                    // 3.1.2 Last continuation byte 0xbf
                    validate_fail({0xbf}, 1, __LINE__);

                    // 3.1.3  2 continuation bytes
                    validate_fail({0x80, 0xbf}, 2, __LINE__);

                    // 3.1.4  3 continuation bytes
                    validate_fail({0x80, 0xbf, 0x80}, 3, __LINE__);

                    // 3.1.5  4 continuation bytes
                    validate_fail({0x80, 0xbf, 0x80, 0xbf}, 4, __LINE__);

                    // 3.1.6  5 continuation bytes
                    validate_fail({0x80, 0xbf, 0x80, 0xbf, 0x80}, 5, __LINE__);

                    // 3.1.7  6 continuation bytes
                    validate_fail({0x80, 0xbf, 0x80, 0xbf, 0x80, 0xbf}, 6, __LINE__);

                    // 3.1.8  7 continuation bytes
                    validate_fail({0x80, 0xbf, 0x80, 0xbf, 0x80, 0xbf, 0x80}, 7, __LINE__);

                    // 3.1.9  Sequence of all 64 possible continuation bytes (0x80-0xbf)
                    string_type test_3_1_9;

                    for (codepoint_type ch = 0x80; ch <= 0xbf; ++ch)
                    {
                        validate_fail({ch}, 1, __LINE__);
                        test_3_1_9 += ch;
                    }

                    validate_fail_str(std::move(test_3_1_9), 64, __LINE__);
                }

                CATCH_SECTION("3.2  Lonely start characters")
                {
                    auto validate_fail_sequence =
                        [&](codepoint_type begin, codepoint_type end, int line)
                    {
                        string_type test_3_2;

                        for (codepoint_type ch = begin; ch <= end; ++ch)
                        {
                            validate_fail({ch, static_cast<codepoint_type>(' ')}, 1, line);
                            test_3_2 += ch;
                            test_3_2 += ' ';
                        }

                        validate_fail_str(std::move(test_3_2), end - begin + 1, line);
                    };

                    // 3.2.1  All 32 first bytes of 2-byte sequences (0xc0-0xdf), each followed by a
                    // space character
                    validate_fail_sequence(0xc0, 0xdf, __LINE__);

                    // 3.2.2  All 16 first bytes of 3-byte sequences (0xe0-0xef) each followed by a
                    // space character
                    validate_fail_sequence(0xe0, 0xef, __LINE__);

                    // 3.2.3  All 8 first bytes of 4-byte sequences (0xf0-0xf7), each followed by a
                    // space character
                    validate_fail_sequence(0xf0, 0xf7, __LINE__);

                    // 3.2.4  All 4 first bytes of 5-byte sequences (0xf8-0xfb), each followed by a
                    // space character
                    validate_fail_sequence(0xf8, 0xfb, __LINE__);

                    // 3.2.5  All 2 first bytes of 6-byte sequences (0xfc-0xfd), each followed by a
                    // space character
                    validate_fail_sequence(0xfc, 0xfd, __LINE__);
                }

                CATCH_SECTION("3.3  Sequences with last continuation byte missing")
                {
                    // 3.3.1  2-byte sequence with last byte missing (U+0000)
                    validate_fail({0xc0}, 1, __LINE__);

                    // 3.3.2  3-byte sequence with last byte missing (U+0000)
                    validate_fail({0xe0, 0x80}, 1, __LINE__);

                    // 3.3.3  4-byte sequence with last byte missing (U+0000)
                    validate_fail({0xf0, 0x80, 0x80}, 1, __LINE__);

                    // 3.3.6  2-byte sequence with last byte missing (U-000007FF)
                    validate_fail({0xdf}, 1, __LINE__);

                    // 3.3.7  3-byte sequence with last byte missing (U-0000FFFF)
                    validate_fail({0xef, 0xbf}, 1, __LINE__);

                    // 3.3.8  4-byte sequence with last byte missing (U-001FFFFF)
                    validate_fail({0xf7, 0xbf, 0xbf}, 1, __LINE__);
                }

                CATCH_SECTION("3.4  Concatenation of incomplete sequences")
                {
                    // All the 6 sequences of 3.3 concatenated
                    validate_fail(
                        {0xc0, 0xe0, 0x80, 0xf0, 0x80, 0x80, 0xdf, 0xef, 0xbf, 0xf7, 0xbf, 0xbf},
                        6,
                        __LINE__);
                }

                CATCH_SECTION("3.5  Impossible bytes")
                {
                    // 3.5.1  fe
                    validate_fail({0xfe}, 1, __LINE__);

                    // 3.5.2  ff
                    validate_fail({0xff}, 1, __LINE__);

                    // 3.5.3  fe fe ff ff
                    validate_fail({0xfe, 0xfe, 0xff, 0xff}, 4, __LINE__);
                }
            }

            CATCH_SECTION("4  Overlong sequences")
            {
                CATCH_SECTION("4.1  Examples of an overlong ASCII character")
                {
                    // 4.1.1 U+002F = c0 af
                    validate_fail({0xc0, 0xaf}, 1, __LINE__);

                    // 4.1.2 U+002F = e0 80 af
                    validate_fail({0xe0, 0x80, 0xaf}, 1, __LINE__);

                    // 4.1.3 U+002F = f0 80 80 af
                    validate_fail({0xf0, 0x80, 0x80, 0xaf}, 1, __LINE__);
                }

                CATCH_SECTION("4.2  Maximum overlong sequences")
                {
                    // 4.2.1  U-0000007F = c1 bf
                    validate_fail({0xc1, 0xbf}, 1, __LINE__);

                    // 4.2.2  U-000007FF = e0 9f bf
                    validate_fail({0xe0, 0x9f, 0xbf}, 1, __LINE__);

                    // 4.2.3  U-0000FFFF = f0 8f bf bf
                    validate_fail({0xf0, 0x8f, 0xbf, 0xbf}, 1, __LINE__);
                }

                CATCH_SECTION("4.3  Overlong representation of the NUL character")
                {
                    // 4.3.1  U+0000 = c0 80
                    validate_fail({0xc0, 0x80}, 1, __LINE__);

                    // 4.3.2  U+0000 = e0 80 80
                    validate_fail({0xe0, 0x80, 0x80}, 1, __LINE__);

                    // 4.3.3  U+0000 = f0 80 80 80
                    validate_fail({0xf0, 0x80, 0x80, 0x80}, 1, __LINE__);
                }
            }

            CATCH_SECTION("5  Illegal code positions")
            {
                CATCH_SECTION("5.1 Single UTF-16 surrogates")
                {
                    // 5.1.1  U+D800 = ed a0 80
                    validate_fail({0xed, 0xa0, 0x80}, 1, __LINE__);

                    // 5.1.2  U+DB7F = ed ad bf
                    validate_fail({0xed, 0xad, 0xbf}, 1, __LINE__);

                    // 5.1.3  U+DB80 = ed ae 80
                    validate_fail({0xed, 0xae, 0x80}, 1, __LINE__);

                    // 5.1.4  U+DBFF = ed af bf
                    validate_fail({0xed, 0xaf, 0xbf}, 1, __LINE__);

                    // 5.1.5  U+DC00 = ed b0 80
                    validate_fail({0xed, 0xb0, 0x80}, 1, __LINE__);

                    // 5.1.6  U+DF80 = ed be 80
                    validate_fail({0xed, 0xbe, 0x80}, 1, __LINE__);

                    // 5.1.7  U+DFFF = ed bf bf
                    validate_fail({0xed, 0xbf, 0xbf}, 1, __LINE__);
                }

                CATCH_SECTION("5.2 Paired UTF-16 surrogates")
                {
                    // 5.2.1  U+D800 U+DC00 = ed a0 80 ed b0 80
                    validate_fail({0xed, 0xa0, 0x80, 0xed, 0xb0, 0x80}, 2, __LINE__);

                    // 5.2.2  U+D800 U+DFFF = ed a0 80 ed bf bf
                    validate_fail({0xed, 0xa0, 0x80, 0xed, 0xbf, 0xbf}, 2, __LINE__);

                    // 5.2.3  U+DB7F U+DC00 = ed ad bf ed b0 80
                    validate_fail({0xed, 0xad, 0xbf, 0xed, 0xb0, 0x80}, 2, __LINE__);

                    // 5.2.4  U+DB7F U+DFFF = ed ad bf ed bf bf
                    validate_fail({0xed, 0xad, 0xbf, 0xed, 0xbf, 0xbf}, 2, __LINE__);

                    // 5.2.5  U+DB80 U+DC00 = ed ae 80 ed b0 80
                    validate_fail({0xed, 0xae, 0x80, 0xed, 0xb0, 0x80}, 2, __LINE__);

                    // 5.2.6  U+DB80 U+DFFF = ed ae 80 ed bf bf
                    validate_fail({0xed, 0xae, 0x80, 0xed, 0xbf, 0xbf}, 2, __LINE__);

                    // 5.2.7  U+DBFF U+DC00 = ed af bf ed b0 80
                    validate_fail({0xed, 0xaf, 0xbf, 0xed, 0xb0, 0x80}, 2, __LINE__);

                    // 5.2.8  U+DBFF U+DFFF = ed af bf ed bf bf
                    validate_fail({0xed, 0xaf, 0xbf, 0xed, 0xbf, 0xbf}, 2, __LINE__);
                }

                CATCH_SECTION("5.3 Noncharacter code positions")
                {
                    // 5.3.1  U+FFFE = ef bf be
                    validate_pass({0xef, 0xbf, 0xbe}, 0xfffe, __LINE__);

                    // 5.3.2  U+FFFF = ef bf bf
                    validate_pass({0xef, 0xbf, 0xbf}, 0xffff, __LINE__);

                    // 5.3.3  U+FDD0 .. U+FDEF
                    validate_pass({0xef, 0xb7, 0x90}, 0xfdd0, __LINE__);
                    validate_pass({0xef, 0xb7, 0x91}, 0xfdd1, __LINE__);
                    validate_pass({0xef, 0xb7, 0x92}, 0xfdd2, __LINE__);
                    validate_pass({0xef, 0xb7, 0x93}, 0xfdd3, __LINE__);
                    validate_pass({0xef, 0xb7, 0x94}, 0xfdd4, __LINE__);
                    validate_pass({0xef, 0xb7, 0x95}, 0xfdd5, __LINE__);
                    validate_pass({0xef, 0xb7, 0x96}, 0xfdd6, __LINE__);
                    validate_pass({0xef, 0xb7, 0x97}, 0xfdd7, __LINE__);
                    validate_pass({0xef, 0xb7, 0x98}, 0xfdd8, __LINE__);
                    validate_pass({0xef, 0xb7, 0x99}, 0xfdd9, __LINE__);
                    validate_pass({0xef, 0xb7, 0x9a}, 0xfdda, __LINE__);
                    validate_pass({0xef, 0xb7, 0x9b}, 0xfddb, __LINE__);
                    validate_pass({0xef, 0xb7, 0x9c}, 0xfddc, __LINE__);
                    validate_pass({0xef, 0xb7, 0x9d}, 0xfddd, __LINE__);
                    validate_pass({0xef, 0xb7, 0x9e}, 0xfdde, __LINE__);
                    validate_pass({0xef, 0xb7, 0x9f}, 0xfddf, __LINE__);
                    validate_pass({0xef, 0xb7, 0xa0}, 0xfde0, __LINE__);
                    validate_pass({0xef, 0xb7, 0xa1}, 0xfde1, __LINE__);
                    validate_pass({0xef, 0xb7, 0xa2}, 0xfde2, __LINE__);
                    validate_pass({0xef, 0xb7, 0xa3}, 0xfde3, __LINE__);
                    validate_pass({0xef, 0xb7, 0xa4}, 0xfde4, __LINE__);
                    validate_pass({0xef, 0xb7, 0xa5}, 0xfde5, __LINE__);
                    validate_pass({0xef, 0xb7, 0xa6}, 0xfde6, __LINE__);
                    validate_pass({0xef, 0xb7, 0xa7}, 0xfde7, __LINE__);
                    validate_pass({0xef, 0xb7, 0xa8}, 0xfde8, __LINE__);
                    validate_pass({0xef, 0xb7, 0xa9}, 0xfde9, __LINE__);
                    validate_pass({0xef, 0xb7, 0xaa}, 0xfdea, __LINE__);
                    validate_pass({0xef, 0xb7, 0xab}, 0xfdeb, __LINE__);
                    validate_pass({0xef, 0xb7, 0xac}, 0xfdec, __LINE__);
                    validate_pass({0xef, 0xb7, 0xad}, 0xfded, __LINE__);
                    validate_pass({0xef, 0xb7, 0xae}, 0xfdee, __LINE__);
                    validate_pass({0xef, 0xb7, 0xaf}, 0xfdef, __LINE__);

                    // 5.3.4  U+nFFFE U+nFFFF (for n = 1..10)
                    validate_pass({0xf0, 0x9f, 0xbf, 0xbe}, 0x1fffe, __LINE__);
                    validate_pass({0xf0, 0x9f, 0xbf, 0xbf}, 0x1ffff, __LINE__);
                    validate_pass({0xf0, 0xaf, 0xbf, 0xbe}, 0x2fffe, __LINE__);
                    validate_pass({0xf0, 0xaf, 0xbf, 0xbf}, 0x2ffff, __LINE__);
                    validate_pass({0xf0, 0xbf, 0xbf, 0xbe}, 0x3fffe, __LINE__);
                    validate_pass({0xf0, 0xbf, 0xbf, 0xbf}, 0x3ffff, __LINE__);
                    validate_pass({0xf1, 0x8f, 0xbf, 0xbe}, 0x4fffe, __LINE__);
                    validate_pass({0xf1, 0x8f, 0xbf, 0xbf}, 0x4ffff, __LINE__);
                    validate_pass({0xf1, 0x9f, 0xbf, 0xbe}, 0x5fffe, __LINE__);
                    validate_pass({0xf1, 0x9f, 0xbf, 0xbf}, 0x5ffff, __LINE__);
                    validate_pass({0xf1, 0xaf, 0xbf, 0xbe}, 0x6fffe, __LINE__);
                    validate_pass({0xf1, 0xaf, 0xbf, 0xbf}, 0x6ffff, __LINE__);
                    validate_pass({0xf1, 0xbf, 0xbf, 0xbe}, 0x7fffe, __LINE__);
                    validate_pass({0xf1, 0xbf, 0xbf, 0xbf}, 0x7ffff, __LINE__);
                    validate_pass({0xf2, 0x8f, 0xbf, 0xbe}, 0x8fffe, __LINE__);
                    validate_pass({0xf2, 0x8f, 0xbf, 0xbf}, 0x8ffff, __LINE__);
                    validate_pass({0xf2, 0x9f, 0xbf, 0xbe}, 0x9fffe, __LINE__);
                    validate_pass({0xf2, 0x9f, 0xbf, 0xbf}, 0x9ffff, __LINE__);
                    validate_pass({0xf2, 0xaf, 0xbf, 0xbe}, 0xafffe, __LINE__);
                    validate_pass({0xf2, 0xaf, 0xbf, 0xbf}, 0xaffff, __LINE__);
                }
            }
        }
    }
}

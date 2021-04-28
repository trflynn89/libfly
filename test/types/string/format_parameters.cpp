#include "fly/types/string/detail/format_parameters.hpp"

#include "fly/types/string/detail/format_context.hpp"
#include "fly/types/string/detail/string_traits.hpp"

#include "catch2/catch_approx.hpp"
#include "catch2/catch_template_test_macros.hpp"
#include "catch2/catch_test_macros.hpp"

#include <functional>
#include <iterator>
#include <string>

namespace {

struct GenericType
{
};

enum class DefaultFormattedEnum : std::uint8_t
{
    One = 1,
};

} // namespace

CATCH_TEMPLATE_TEST_CASE(
    "BasicFormatParameters",
    "[string]",
    char,
    wchar_t,
    char8_t,
    char16_t,
    char32_t)
{
    using traits = fly::detail::BasicStringTraits<TestType>;

    using string_type = typename traits::string_type;
    using char_type = typename traits::char_type;
    using view_type = typename traits::view_type;

    using FormatContext =
        fly::detail::BasicFormatContext<std::back_insert_iterator<string_type>, char_type>;
    using UserDefinedValue = fly::detail::UserDefinedValue<FormatContext>;
    using StringValue = fly::detail::StringValue<FormatContext>;

    string_type buffer;
    auto out = std::back_inserter(buffer);

    CATCH_SECTION("Empty parameters result in monostate status")
    {
        auto params = fly::detail::make_format_parameters<FormatContext>();
        FormatContext context(out, params);

        auto parameter = context.arg(0);
        CATCH_CHECK_FALSE(parameter);

        parameter.visit(
            [](auto value)
            {
                CATCH_CHECK(std::is_same_v<decltype(value), fly::detail::MonoState>);
            });

        parameter.format(context, {});
    }

    CATCH_SECTION("A single parameter can be visited, but no others")
    {
        auto params = fly::detail::make_format_parameters<FormatContext>(1);
        FormatContext context(out, params);
        {
            auto parameter = context.arg(0);
            CATCH_CHECK(parameter);

            parameter.visit(
                [](auto value)
                {
                    CATCH_CHECK(std::is_same_v<decltype(value), std::int64_t>);
                });
        }
        {
            auto parameter = context.arg(1);
            CATCH_CHECK_FALSE(parameter);

            parameter.visit(
                [](auto value)
                {
                    CATCH_CHECK(std::is_same_v<decltype(value), fly::detail::MonoState>);
                });
        }
    }

    CATCH_SECTION("User-defined values are type-erased")
    {
        GenericType generic {};

        auto verify = [&generic](auto value)
        {
            CATCH_CHECK(std::is_same_v<decltype(value), UserDefinedValue>);

            if constexpr (std::is_same_v<decltype(value), UserDefinedValue>)
            {
                CATCH_CHECK(value.m_value == &generic);
                CATCH_CHECK(value.m_format != nullptr);
            }
        };

        auto params = fly::detail::make_format_parameters<FormatContext>(generic);
        FormatContext context(out, params);

        context.arg(0).visit(verify);

        CATCH_CHECK_FALSE(context.arg(1));
    }

    CATCH_SECTION("String values are type-erased")
    {
        const auto &arr = FLY_ARR(char_type, "str");
        string_type str(FLY_STR(char_type, "str"));

        auto verify = [&str](auto value)
        {
            CATCH_CHECK(std::is_same_v<decltype(value), StringValue>);

            if constexpr (std::is_same_v<decltype(value), StringValue>)
            {
                view_type view(static_cast<const char_type *>(value.m_value), value.m_size);
                CATCH_CHECK(view == str);
                CATCH_CHECK(value.m_format != nullptr);
            }
        };

        auto params = fly::detail::make_format_parameters<FormatContext>(arr, str, view_type(str));
        FormatContext context(out, params);

        context.arg(0).visit(verify);
        context.arg(1).visit(verify);
        context.arg(2).visit(verify);

        CATCH_CHECK_FALSE(context.arg(3));
    }

    CATCH_SECTION("Pointers are coerced to the appropriate type")
    {
        int i = 0;

        std::nullptr_t p1 = nullptr;
        void *p2 = &i;
        const void *p3 = &i;

        auto params = fly::detail::make_format_parameters<FormatContext>(p1, p2, p3);
        FormatContext context(out, params);

        auto verify = [](auto expected_value, auto actual_value)
        {
            CATCH_CHECK(std::is_same_v<decltype(actual_value), const void *>);

            if constexpr (std::is_same_v<decltype(actual_value), const void *>)
            {
                CATCH_CHECK(static_cast<const void *>(expected_value) == actual_value);
            }
        };

        context.arg(0).visit(std::bind(verify, p1, std::placeholders::_1));
        context.arg(1).visit(std::bind(verify, p2, std::placeholders::_1));
        context.arg(2).visit(std::bind(verify, p3, std::placeholders::_1));

        CATCH_CHECK_FALSE(context.arg(3));
    }

    CATCH_SECTION("Floating-point values are coerced to the appropriate type")
    {
        float f = 3.14f;
        double d = 6.28;
        long double dd = 12.56;

        auto params = fly::detail::make_format_parameters<FormatContext>(f, d, dd);
        FormatContext context(out, params);

        auto verify = [](auto expected_value, auto actual_value)
        {
            CATCH_CHECK(std::is_same_v<decltype(expected_value), decltype(actual_value)>);

            if constexpr (std::is_same_v<decltype(expected_value), decltype(actual_value)>)
            {
                CATCH_CHECK(expected_value == Catch::Approx(actual_value));
            }
        };

        context.arg(0).visit(std::bind(verify, f, std::placeholders::_1));
        context.arg(1).visit(std::bind(verify, d, std::placeholders::_1));
        context.arg(2).visit(std::bind(verify, dd, std::placeholders::_1));

        CATCH_CHECK_FALSE(context.arg(3));
    }

    CATCH_SECTION("Integral values are coerced to the appropriate type")
    {
        char_type c = FLY_CHR(char_type, 'c');

        auto params = fly::detail::make_format_parameters<FormatContext>(c);
        FormatContext context(out, params);

        context.arg(0).visit(
            [c](auto value)
            {
                constexpr bool is_int = std::is_same_v<decltype(value), std::int64_t> ||
                    std::is_same_v<decltype(value), std::uint64_t>;
                CATCH_CHECK(is_int);

                if constexpr (is_int)
                {
                    CATCH_CHECK(static_cast<char_type>(value) == c);
                }
            });

        CATCH_CHECK_FALSE(context.arg(1));
    }

    CATCH_SECTION("Signed integer values are coerced to the appropriate type")
    {
        std::int8_t i1 = 1;
        std::int16_t i2 = 2;
        std::int32_t i3 = 3;
        std::int64_t i4 = 4;

        auto params = fly::detail::make_format_parameters<FormatContext>(i1, i2, i3, i4);
        FormatContext context(out, params);

        auto verify = [](auto expected_value, auto actual_value)
        {
            CATCH_CHECK(std::is_same_v<decltype(actual_value), std::int64_t>);

            if constexpr (std::is_same_v<decltype(actual_value), std::int64_t>)
            {
                CATCH_CHECK(static_cast<decltype(expected_value)>(actual_value) == expected_value);
            }
        };

        context.arg(0).visit(std::bind(verify, i1, std::placeholders::_1));
        context.arg(1).visit(std::bind(verify, i2, std::placeholders::_1));
        context.arg(2).visit(std::bind(verify, i3, std::placeholders::_1));
        context.arg(3).visit(std::bind(verify, i4, std::placeholders::_1));

        CATCH_CHECK_FALSE(context.arg(4));
    }

    CATCH_SECTION("Unsigned integer values are coerced to the appropriate type")
    {
        std::uint8_t u1 = 1;
        std::uint16_t u2 = 2;
        std::uint32_t u3 = 3;
        std::uint64_t u4 = 4;

        auto params = fly::detail::make_format_parameters<FormatContext>(u1, u2, u3, u4);
        FormatContext context(out, params);

        auto verify = [](auto expected_value, auto actual_value)
        {
            CATCH_CHECK(std::is_same_v<decltype(actual_value), std::uint64_t>);

            if constexpr (std::is_same_v<decltype(actual_value), std::uint64_t>)
            {
                CATCH_CHECK(static_cast<decltype(expected_value)>(actual_value) == expected_value);
            }
        };

        context.arg(0).visit(std::bind(verify, u1, std::placeholders::_1));
        context.arg(1).visit(std::bind(verify, u2, std::placeholders::_1));
        context.arg(2).visit(std::bind(verify, u3, std::placeholders::_1));
        context.arg(3).visit(std::bind(verify, u4, std::placeholders::_1));

        CATCH_CHECK_FALSE(context.arg(4));
    }

    CATCH_SECTION("Boolean values are coerced to the appropriate type")
    {
        bool b1 = true;
        bool b2 = false;

        auto params = fly::detail::make_format_parameters<FormatContext>(b1, b2);
        FormatContext context(out, params);

        auto verify = [](auto expected_value, auto actual_value)
        {
            CATCH_CHECK(std::is_same_v<decltype(actual_value), bool>);

            if constexpr (std::is_same_v<decltype(actual_value), bool>)
            {
                CATCH_CHECK(static_cast<decltype(expected_value)>(actual_value) == expected_value);
            }
        };

        context.arg(0).visit(std::bind(verify, b1, std::placeholders::_1));
        context.arg(1).visit(std::bind(verify, b2, std::placeholders::_1));

        CATCH_CHECK_FALSE(context.arg(2));
    }

    CATCH_SECTION("Default-formatted enumerations are coerced to the appropriate type")
    {
        DefaultFormattedEnum e = DefaultFormattedEnum::One;

        auto params = fly::detail::make_format_parameters<FormatContext>(e);
        FormatContext context(out, params);

        context.arg(0).visit(
            [e](auto value)
            {
                CATCH_CHECK(std::is_same_v<decltype(value), std::uint64_t>);

                if constexpr (std::is_same_v<decltype(value), std::uint64_t>)
                {
                    CATCH_CHECK(static_cast<DefaultFormattedEnum>(value) == e);
                }
            });

        CATCH_CHECK_FALSE(context.arg(1));
    }
}

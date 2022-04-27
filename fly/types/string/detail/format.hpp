#pragma once

#include "fly/types/string/detail/converter.hpp"
#include "fly/types/string/detail/format_context.hpp"
#include "fly/types/string/detail/format_parameters.hpp"
#include "fly/types/string/detail/format_parse_context.hpp"
#include "fly/types/string/detail/format_string.hpp"
#include "fly/types/string/detail/traits.hpp"

#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>

namespace fly::string::detail {

/**
 * Format a string with a set of format parameters to an existing output iterator. Based strongly
 * upon: https://en.cppreference.com/w/cpp/utility/format/format.
 *
 * @tparam CharType The underlying standard character type of the string to format.
 * @tparam OutputIterator The type of the output iterator.
 * @tparam ParameterTypes Variadic format parameter types.
 *
 * @param output The output iterator to write to.
 * @param format The string to format.
 * @param parameters The variadic list of format parameters to be formatted.
 *
 * @return A string that has been formatted with the given format parameters.
 */
template <StandardCharacter CharType, typename OutputIterator, typename... ParameterTypes>
void format_to(
    OutputIterator output,
    fly::detail::BasicFormatString<CharType, std::type_identity_t<ParameterTypes>...> &&format,
    ParameterTypes &&...parameters)
{
    using FormatContext = fly::detail::BasicFormatContext<OutputIterator, CharType>;
    using FormatParseContext = fly::detail::BasicFormatParseContext<CharType>;

    FormatParseContext &parse_context = format.context();
    auto const view = parse_context.view();

    if (parse_context.has_error())
    {
        format_to<CharType>(
            output,
            FLY_ARR(CharType, "Ignored invalid formatter: {}"),
            parse_context.error());
        return;
    }

    auto params = fly::detail::make_format_parameters<FormatContext>(
        std::forward<ParameterTypes>(parameters)...);
    FormatContext context(output, params);

    for (std::size_t pos = 0; pos < view.size();)
    {
        switch (auto const &ch = view[pos])
        {
            case FLY_CHR(CharType, '{'):
                if (view[pos + 1] == FLY_CHR(CharType, '{'))
                {
                    *context.out()++ = ch;
                    pos += 2;
                }
                else
                {
                    auto specifier = *std::move(format.next_specifier());
                    pos += specifier.m_size;

                    auto const parameter = context.arg(specifier.m_position);
                    parameter.format(parse_context, context, std::move(specifier));
                }
                break;

            case FLY_CHR(CharType, '}'):
                *context.out()++ = ch;
                pos += 2;
                break;

            default:
                *context.out()++ = ch;
                ++pos;
                break;
        }
    }
}

} // namespace fly::string::detail

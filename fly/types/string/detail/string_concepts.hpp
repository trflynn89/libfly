#pragma once

namespace fly::detail {

/**
 * Concept that is satisfied when the given formatter defines a |parse| method.
 */
template <typename FormatParseContext, typename Formatter>
concept FormatterWithParsing = requires(FormatParseContext parse_context, Formatter formatter)
{
    formatter.parse(parse_context);
};

} // namespace fly::detail

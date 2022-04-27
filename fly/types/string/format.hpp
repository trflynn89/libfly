#pragma once

#include "fly/types/string/detail/format.hpp"
#include "fly/types/string/detail/format_context.hpp"
#include "fly/types/string/detail/format_string.hpp"
#include "fly/types/string/detail/traits.hpp"

#include <iterator>
#include <string>
#include <type_traits>

namespace fly::string {

/**
 * A container to hold and parse a format string at compile time.
 */
template <StandardCharacter CharType, typename... ParameterTypes>
using FormatString = detail::BasicFormatString<CharType, std::type_identity_t<ParameterTypes>...>;

/**
 * Format a string with a set of format parameters, returning the formatted string. Based strongly
 * upon: https://en.cppreference.com/w/cpp/utility/format/format.
 *
 * A format string consists of:
 *
 *     1. Any character other than "{" or "}", which are copied unchanged to the output.
 *     2. Escape sequences "{{" and "}}", which are replaced with "{" and "}" in the output.
 *     3. Replacement fields.
 *
 * Replacement fields may be of the form:
 *
 *     1. An introductory "{" character.
 *     2. An optional non-negative position.
 *     3. An optional colon ":" following by formatting options.
 *     4. A final "}" character.
 *
 * For a detailed description of replacement fields, see fly::string::detail::BasicFormatSpecifier.
 *
 * This implementation differs from std::format in the following ways:
 *
 *    1. All standard string types are supported as format strings.
 *
 *    2. All standard string types are supported as format parameters, even if that type differs
 *       from the format string type. If the type differs, the format parameter is transcoded to the
 *       type of the format string.
 *
 *    3. This implementation is exceptionless. Any error encountered (such as failed transcoding in
 *       (2) above) results in the format parameter that caused the error to be dropped.
 *
 *    4. Locale-specific form is not supported. If the option appears in the format string, it will
 *       be parsed, but will be ignored.
 *
 * The format string type is implicitly constructed from a C-string literal. Callers should only
 * invoke this method accordingly:
 *
 *     fly::string::format("Format {:d}", 1);
 *
 * On compilers that support immediate functions (consteval), the format string is validated at
 * compile time against the types of the format parameters. If the format string is invalid, a
 * compile error with a diagnostic message will be raised. On other compilers, the error message
 * will be returned rather than a formatted string.
 *
 * Replacement fields for user-defined types are parsed at runtime. To format a user-defined type, a
 * fly::string::Formatter specialization must be defined, analagous to std::formatter. The
 * specialization may extend a standard fly::string::Formatter, for example:
 *
 *     template <typename CharType>
 *     struct fly::string::Formatter<MyType, CharType> :
 *         public fly::string::Formatter<int, CharType>
 *     {
 *         template <typename FormatContext>
 *         void format(MyType const &value, FormatContext &context)
 *         {
 *             fly::string::Formatter<int, CharType>::format(value.as_int(), context);
 *         }
 *     };
 *
 * Or, the formatter may be defined without without inheritence:
 *
 *     template <typename CharType>
 *     struct fly::string::Formatter<MyType, CharType>
 *     {
 *         bool m_option {false};
 *
 *         template <typename FormatParseContext>
 *         constexpr void parse(FormatParseContext &context)
 *         {
 *             if (context.lexer().consume_if(FLY_CHR(CharType, 'o')))
 *             {
 *                 m_option = true;
 *             }
 *             if (!context.lexer().consume_if(FLY_CHR(CharType, '}')))
 *             {
 *                 context.on_error("UserDefinedTypeWithParser error!");
 *             }
 *         }
 *
 *         template <typename FormatContext>
 *         void format(MyType const &value, FormatContext &context)
 *         {
 *             fly::string::format_to(context.out(), "{}", value.as_int());
 *         }
 *     };
 *
 * The |parse| method is optional. If defined, it is provided a BasicFormatParseContext which
 * contains a lexer that may be used to parse the format string. The position of the lexer will be
 * one of the following within the replacement field:
 *
 *     1. The position immediately after the colon, if there is one.
 *     2. Otherwise, the position immediately after the format parameter index, if there is one.
 *     3. Otherwise, the position immeidately after the opening brace.
 *
 * The |parse| method is expected to consume up to and including the closing "}" character. It must
 * be declared constexpr, as it will be invoked at compile time to validate the replacement field.
 * The parser may indicate any parsing errors through the parsing context; if an error occurs, the
 * error is handled the same as any standard replacement field (see above). Even though the parser
 * is invoked at compile time, the result of user-defined parsing cannot be stored generically.
 * Thus, parsing is invoked again at runtime immediately before |format|.
 *
 * @tparam ParameterTypes Variadic format parameter types.
 *
 * @param format The string to format.
 * @param parameters The variadic list of format parameters to be formatted.
 *
 * @return A string that has been formatted with the given format parameters.
 */
///@{
#define FLY_STRING_FORMAT(CharType, StringType)                                                    \
    template <typename... ParameterTypes>                                                          \
    inline StringType format(                                                                      \
        FormatString<CharType, ParameterTypes...> &&format,                                        \
        ParameterTypes &&...parameters)                                                            \
    {                                                                                              \
        typename fly::detail::BasicStringTraits<CharType>::string_type formatted;                  \
        formatted.reserve(format.context().view().size() * 2);                                     \
                                                                                                   \
        detail::format_to<CharType>(                                                               \
            std::back_inserter(formatted),                                                         \
            std::move(format),                                                                     \
            std::forward<ParameterTypes>(parameters)...);                                          \
                                                                                                   \
        return formatted;                                                                          \
    }

FLY_ENUMERATE_STANDARD_CHARACTERS(FLY_STRING_FORMAT)
///@}

/**
 * Format a string with a set of format parameters to an existing output iterator. Based strongly
 * upon: https://en.cppreference.com/w/cpp/utility/format/format.
 *
 * For a detailed description of string formatting, see fly::string::format.
 *
 * @tparam OutputIterator The type of the output iterator.
 * @tparam ParameterTypes Variadic format parameter types.
 *
 * @param output The output iterator to write to.
 * @param format The string to format.
 * @param parameters The variadic list of format parameters to be formatted.
 *
 * @return A string that has been formatted with the given format parameters.
 */
///@{
#define FLY_STRING_FORMAT_TO(CharType, StringType)                                                 \
    template <typename OutputIterator, typename... ParameterTypes>                                 \
    inline void format_to(                                                                         \
        OutputIterator output,                                                                     \
        FormatString<CharType, ParameterTypes...> &&format,                                        \
        ParameterTypes &&...parameters)                                                            \
    {                                                                                              \
        detail::format_to<CharType>(                                                               \
            std::move(output),                                                                     \
            std::move(format),                                                                     \
            std::forward<ParameterTypes>(parameters)...);                                          \
    }

FLY_ENUMERATE_STANDARD_CHARACTERS(FLY_STRING_FORMAT_TO)
///@}

} // namespace fly::string

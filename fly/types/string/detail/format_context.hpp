#pragma once

#include "fly/types/string/detail/format_parameters.hpp"
#include "fly/types/string/detail/format_specifier.hpp"

#include <cstddef>

namespace fly::detail {

/**
 * Provides access to the formatting state consisting of the format parameters, replacement fields
 * and the output iterator.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version April 4, 2021
 */
template <typename OutputIterator, typename CharType>
class BasicFormatContext
{
    using FormatParameter = BasicFormatParameter<BasicFormatContext>;
    using FormatSpecifier = BasicFormatSpecifier<CharType>;

public:
    using char_type = CharType;

    /**
     * Constructor.
     *
     * @param out The output iterator into which the formatted value should be written.
     * @param parameters The format parameters created with |make_format_parameters|.
     */
    template <typename... Parameters>
    constexpr BasicFormatContext(
        OutputIterator out,
        const BasicFormatParameters<BasicFormatContext, Parameters...> &parameters) noexcept;

    /**
     * Get the object holding the format parameter at the specified index. If the index is invalid,
     * returns a format parameter holding |MonoState|.
     *
     * @param index The index to lookup.
     *
     * @return The format parameter at the specified index.
     */
    FormatParameter arg(std::size_t index) const;

    /**
     * @return The output iterator into which the formatted value should be written.
     */
    OutputIterator &out();

    /**
     * @return The formatting replacement field currently being used for formatting.
     */
    FormatSpecifier &spec();

private:
    BasicFormatContext(const BasicFormatContext &) = delete;
    BasicFormatContext &operator=(const BasicFormatContext &) = delete;

    OutputIterator m_out;

    const FormatParameter *m_parameters;
    const std::size_t m_parameters_size;

    FormatSpecifier m_specifier {};
};

//==================================================================================================
template <typename OutputIterator, typename CharType>
template <typename... Parameters>
constexpr inline BasicFormatContext<OutputIterator, CharType>::BasicFormatContext(
    OutputIterator out,
    const BasicFormatParameters<BasicFormatContext, Parameters...> &parameters) noexcept :
    m_out(std::move(out)),
    m_parameters(parameters.m_parameters.data()),
    m_parameters_size(parameters.m_parameters.size())
{
}

//==================================================================================================
template <typename OutputIterator, typename CharType>
inline auto BasicFormatContext<OutputIterator, CharType>::arg(std::size_t index) const
    -> FormatParameter
{
    if (index < m_parameters_size)
    {
        return m_parameters[index];
    }

    return {};
}

//==================================================================================================
template <typename OutputIterator, typename CharType>
inline OutputIterator &BasicFormatContext<OutputIterator, CharType>::out()
{
    return m_out;
}

//==================================================================================================
template <typename OutputIterator, typename CharType>
inline auto BasicFormatContext<OutputIterator, CharType>::spec() -> FormatSpecifier &
{
    return m_specifier;
}

} // namespace fly::detail
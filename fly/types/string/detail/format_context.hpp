#pragma once

#include "fly/types/string/concepts.hpp"
#include "fly/types/string/detail/format_parameters.hpp"
#include "fly/types/string/formatters.hpp"

#include <cstddef>
#include <type_traits>

namespace fly::detail {

/**
 * Provides access to the formatting state consisting of the format parameters, replacement fields
 * and the output iterator.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version April 4, 2021
 */
template <typename OutputIterator, fly::StandardCharacter CharType>
class BasicFormatContext
{
    using FormatParameter = BasicFormatParameter<BasicFormatContext>;

public:
    using char_type = CharType;

    template <typename T>
    using formatter_type = fly::string::Formatter<std::remove_cvref_t<T>, CharType>;

    /**
     * Constructor.
     *
     * @param out The output iterator into which the formatted value should be written.
     * @param parameters The format parameters created with |make_format_parameters|.
     */
    template <typename... Parameters>
    constexpr BasicFormatContext(
        OutputIterator out,
        BasicFormatParameters<BasicFormatContext, Parameters...> const &parameters) noexcept;

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

private:
    BasicFormatContext(BasicFormatContext const &) = delete;
    BasicFormatContext &operator=(BasicFormatContext const &) = delete;

    OutputIterator m_out;

    FormatParameter const *m_parameters;
    std::size_t const m_parameters_size;
};

//==================================================================================================
template <typename OutputIterator, fly::StandardCharacter CharType>
template <typename... Parameters>
constexpr BasicFormatContext<OutputIterator, CharType>::BasicFormatContext(
    OutputIterator out,
    BasicFormatParameters<BasicFormatContext, Parameters...> const &parameters) noexcept :
    m_out(std::move(out)),
    m_parameters(parameters.m_parameters.data()),
    m_parameters_size(parameters.m_parameters.size())
{
}

//==================================================================================================
template <typename OutputIterator, fly::StandardCharacter CharType>
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
template <typename OutputIterator, fly::StandardCharacter CharType>
inline OutputIterator &BasicFormatContext<OutputIterator, CharType>::out()
{
    return m_out;
}

} // namespace fly::detail

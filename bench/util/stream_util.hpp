#pragma once

#include "fly/types/string/detail/string_streamer.hpp"

#include <iomanip>
#include <locale>
#include <ostream>

namespace fly::benchmark {

/**
 * Helper class to center text within a specified width within a steam.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version Decmber 12, 2020
 */
template <typename StringType>
class Center
{
public:
    /**
     * Constructor.
     *
     * @param width The width to center the text within.
     * @param value The text to be centered.
     */
    Center(std::size_t width, const StringType &value);

    /**
     * Print the centered text.
     *
     * @param stream The stream to print the text into.
     * @param center The instance holding the centering data.
     *
     * @return The same stream object.
     */
    friend std::ostream &operator<<(std::ostream &stream, const Center &center)
    {
        center.print(stream);
        return stream;
    }

private:
    /**
     * Print the centered text.
     *
     * @param stream The stream to print the text into.
     */
    void print(std::ostream &stream) const;

    const std::streamsize m_width;
    const StringType &m_value;
};

/**
 * Locale facet to format numbers with comma separators.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version Decmber 12, 2020
 */
class CommaPunctuation final : public std::numpunct<std::ios::char_type>
{
protected:
    /**
     * @return The character (comma) to use as the thousands separator.
     */
    std::ios::char_type do_thousands_sep() const override;

    /**
     * @return The number of digits between each pair of thousands separators.
     */
    std::string do_grouping() const override;
};

//==================================================================================================
template <typename StringType>
Center<StringType>::Center(std::size_t width, const StringType &value) :
    m_width(static_cast<std::streamsize>(width)),
    m_value(value)
{
}

//==================================================================================================
template <typename StringType>
void Center<StringType>::print(std::ostream &stream) const
{
    const std::streamsize length = static_cast<std::streamsize>(m_value.size());

    if (m_width > length)
    {
        const std::streamsize left = (m_width + length) / 2;

        fly::detail::BasicStreamModifiers<std::string> scoped_modifiers(stream);
        scoped_modifiers.manip(std::right);

        stream << std::setw(left) << m_value;
        stream << std::setw(m_width - left) << "";
    }
    else
    {
        stream << m_value;
    }
}

} // namespace fly::benchmark

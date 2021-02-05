#pragma once

#include <locale>
#include <ostream>

namespace fly::benchmark {

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

} // namespace fly::benchmark

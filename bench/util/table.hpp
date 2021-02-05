#pragma once

#include "bench/util/stream_util.hpp"

#include "fly/logger/styler.hpp"
#include "fly/types/numeric/literals.hpp"
#include "fly/types/string/string.hpp"

#include <array>
#include <cmath>
#include <iomanip>
#include <locale>
#include <numeric>
#include <ostream>
#include <tuple>
#include <type_traits>
#include <vector>

namespace fly::benchmark {

/**
 * Class to form and pretty-print a table of generic values. The table is both stylized and colored.
 *
 * For example, with the following invocation:
 *
 *     fly::benchmark::Table<std::string, double, std::int64_t> table(
 *         "Table Title",
 *         {"Column 1", "Column 2", "Column 3"});
 *
 *     table.append_row("Row 1", 123456789.000, 789);
 *     table.append_row("Row 2", 3.14, 99999999);
 *     table.append_row("Row 3", 2.71828, -189);
 *     table.append_row("Row 4", 0, 0);
 *
 *     std::cout << table << '\n';
 *
 * The following table will be streamed onto std::cout:
 *
 *     -------------------------------------------
 *     |               Table Title               |
 *     -------------------------------------------
 *     | Column 1 |    Column 2     |  Column 3  |
 *     -------------------------------------------
 *     | Row 1    | 123,456,789.000 |        789 |
 *     | Row 2    |           3.140 | 99,999,999 |
 *     | Row 3    |           2.718 |       -189 |
 *     | Row 4    |           0.000 |          0 |
 *     -------------------------------------------
 *
 * @tparam Args Variadic list of column types.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version Decmber 12, 2020
 */
template <class... Args>
class Table
{
    using Row = std::tuple<Args...>;

    template <typename T>
    using RowSizedArray = std::array<T, std::tuple_size_v<Row>>;

public:
    /**
     * Constructor. Create a table with a given title and column headers.
     *
     * @param title The title to apply to the table.
     * @param headers The column headers to apply to the table.
     */
    Table(std::string title, RowSizedArray<std::string> headers);

    /**
     * Append a row of data to the table.
     *
     * @param args Variadic list of column data.
     */
    void append_row(Args... args);

    /**
     * Pretty-print the table onto a stream.
     *
     * @param stream The stream to print the table into.
     * @param table The table to print.
     *
     * @return The same stream object.
     */
    friend std::ostream &operator<<(std::ostream &stream, const Table &table)
    {
        table.print_table(stream);
        return stream;
    }

private:
    /**
     * Pretty-print the table onto a stream.
     *
     * @param stream The stream to print the table into.
     */
    void print_table(std::ostream &stream) const;

    /**
     * Print the table title onto a stream. The length title may be capped depending on the total
     * width of the table (that is, the length of the title does not implicitly make the table
     * wider). The top border of the table and a separator below the tile are also streamed.
     *
     * @param stream The stream to print the title into.
     * @param table_width The width of the table.
     */
    void print_title(std::ostream &stream, std::size_t table_width) const;

    /**
     * Print the column headers onto a stream. The headers will be centered in the available width
     * of their respective columns. A separator below the headers is also streamed.
     *
     * @param stream The stream to print the headers into.
     * @param table_width The width of the table.
     */
    void print_headers(std::ostream &stream, std::size_t table_width) const;

    /**
     * Print all values in a row of data onto a stream.
     *
     * Before printing each value in the row, the stream is formatted in accordance with that
     * value's type, size, and column width. String data is left-aligned, numeric data is
     * right-aligned. The values are padded to fit within the column.
     *
     * @param stream The stream to print the values into.
     * @param row The values to print.
     */
    void print_row(std::ostream &stream, const Row &row) const;

    /**
     * Print a horizontal row separator of the given width and with the given style onto a stream.
     *
     * @param stream The stream to print the separator into.
     * @param width The width of the table.
     * @param style The style to format the separator with..
     *
     * @return The same stream object.
     */
    std::ostream &print_row_separator(
        std::ostream &stream,
        std::size_t width,
        fly::Style style = fly::Style::Default) const;

    /**
     * Print a vertical column separator with the given style onto a stream.
     *
     * @param stream The stream to print the separator into.
     * @param style The style to format the separator with..
     *
     * @return The same stream object.
     */
    std::ostream &
    print_column_separator(std::ostream &stream, fly::Style style = fly::Style::Default) const;

    /**
     * Compute the widths required to print each value in row of data.
     *
     * For string data, the size is the length of the string. For numeric data, the size takes into
     * account the number of digits, whether the value is negative, etc.
     *
     * @param row The values to size.
     *
     * @return An array holding the sizes.
     */
    RowSizedArray<std::size_t> column_widths_for_row(const Row &row);

    /**
     * Helper to determine if a numeric value is zero (if integral), or close enough to zero (if
     * floating point).
     *
     * @tparam T The numeric type of the value to test.
     *
     * @param value The numeric value to test.
     *
     * @return True if the value is zero.
     */
    template <typename T>
    static bool is_zero(T value);

    static constexpr const fly::Color s_border_color = fly::Color::Cyan;
    static constexpr const fly::Style s_border_style = fly::Style::Bold;

    static constexpr const fly::Color s_title_color = fly::Color::Green;
    static constexpr const fly::Style s_title_style = fly::Style::Bold;

    static constexpr const fly::Color s_header_color = fly::Color::Red;
    static constexpr const fly::Style s_header_style = fly::Style::Italic;

    static constexpr const fly::Color s_data_color = fly::Color::Yellow;
    static constexpr const fly::Style s_data_style = fly::Style::Default;

    static constexpr const std::size_t s_precision = 3;

    const std::string m_title;
    const RowSizedArray<std::string> m_headers;
    std::vector<Row> m_data;
    RowSizedArray<std::size_t> m_column_widths;
};

//==================================================================================================
template <class... Args>
Table<Args...>::Table(std::string title, RowSizedArray<std::string> headers) :
    m_title(std::move(title)),
    m_headers(std::move(headers))
{
    for (std::size_t i = 0; i < m_headers.size(); ++i)
    {
        m_column_widths[i] = m_headers[i].size();
    }
}

//==================================================================================================
template <class... Args>
void Table<Args...>::append_row(Args... args)
{
    m_data.emplace_back(std::make_tuple(std::forward<Args>(args)...));

    // Potentially resize each column's width based on the widths of the new row.
    const RowSizedArray<std::size_t> widths = column_widths_for_row(m_data.back());

    for (std::size_t i = 0; i < widths.size(); ++i)
    {
        m_column_widths[i] = std::max(m_column_widths[i], widths[i]);
    }
}

//==================================================================================================
template <class... Args>
void Table<Args...>::print_table(std::ostream &stream) const
{
    fly::detail::BasicStreamModifiers<std::string> scoped_modifiers(stream);
    scoped_modifiers.locale<CommaPunctuation>();

    // Compute the entire width of the table. There are 1 + the number of columns vertical
    // separators ('|'), plus the width of each column (with 2 padding spacers each).
    const std::size_t table_width = (m_column_widths.size() + 1) +
        std::accumulate(m_column_widths.begin(), m_column_widths.end(), 0_zu) +
        (2 * m_column_widths.size());

    print_title(stream, table_width);
    print_headers(stream, table_width);

    for (auto &row : m_data)
    {
        print_row(stream, row);
    }

    print_row_separator(stream, table_width, s_border_style);
}

//==================================================================================================
template <class... Args>
void Table<Args...>::print_title(std::ostream &stream, std::size_t table_width) const
{
    const auto style = fly::Styler(s_title_style, s_title_color);

    print_row_separator(stream, table_width, s_border_style);
    print_column_separator(stream, s_border_style);

    // Compute the width available for the title. The title can consume the same width of the table,
    // except for the 2 vertical separators (and their padding) for the table's outside borders.
    const std::size_t title_width = table_width - 4;

    auto title = std::string_view(m_title).substr(0, title_width);
    stream << style << fly::String::format(" {:^{}} ", title, title_width);

    print_column_separator(stream, s_border_style) << '\n';
    print_row_separator(stream, table_width);
}

//==================================================================================================
template <class... Args>
void Table<Args...>::print_headers(std::ostream &stream, std::size_t table_width) const
{
    for (std::size_t index = 0; index < m_headers.size(); ++index)
    {
        print_column_separator(stream, index == 0 ? s_border_style : fly::Style::Default);

        const auto style = fly::Styler(s_header_style, s_header_color);
        stream << style;

        stream << fly::String::format(" {:^{}} ", m_headers[index], m_column_widths[index]);
    }

    print_column_separator(stream, s_border_style) << '\n';
    print_row_separator(stream, table_width);
}

//==================================================================================================
template <class... Args>
void Table<Args...>::print_row(std::ostream &stream, const Row &row) const
{
    std::size_t index = 0;

    auto print_value = [this, &stream, &index](const auto &value)
    {
        print_column_separator(stream, index == 0 ? s_border_style : fly::Style::Default);

        const auto style = fly::Styler(s_data_style, s_data_color);
        stream << style;

        if constexpr (std::is_floating_point_v<std::remove_cvref_t<decltype(value)>>)
        {
            stream
                << fly::String::format(" {:{}.{}f} ", value, m_column_widths[index], s_precision);
        }
        else
        {
            stream << fly::String::format(" {:{}} ", value, m_column_widths[index]);
        }

        ++index;
    };

    std::apply(
        [&print_value](const auto &...e)
        {
            (print_value(e), ...);
        },
        row);

    print_column_separator(stream, s_border_style) << '\n';
}

//==================================================================================================
template <class... Args>
std::ostream &
Table<Args...>::print_row_separator(std::ostream &stream, std::size_t width, fly::Style style) const
{
    stream << fly::Styler(style, s_border_color) << fly::String::format("{:->{}}\n", '-', width);
    return stream;
}

//==================================================================================================
template <class... Args>
std::ostream &Table<Args...>::print_column_separator(std::ostream &stream, fly::Style style) const
{
    stream << fly::Styler(style, s_border_color) << '|';
    return stream;
}

//==================================================================================================
template <class... Args>
auto Table<Args...>::column_widths_for_row(const Row &row) -> RowSizedArray<std::size_t>
{
    RowSizedArray<std::size_t> widths;
    std::size_t index = 0;

    auto size_of_value = [&widths, &index](const auto &value)
    {
        using T = std::decay_t<decltype(value)>;

        if constexpr (fly::StringTraits::is_string_like_v<T>)
        {
            widths[index] = fly::String::size(value);
        }
        else if constexpr (std::is_arithmetic_v<T>)
        {
            if (is_zero(value))
            {
                // Prevent std::log10(0), which is considered an error.
                widths[index] = std::is_floating_point_v<T> ? s_precision + 2 : 1;
            }
            else
            {
                const std::size_t negative = value < 0 ? 1 : 0;
                const std::size_t digits = static_cast<std::size_t>(
                    std::abs(static_cast<std::int64_t>(std::log10(std::abs(value)))) + 1);
                const std::size_t commas = (digits - 1) / 3;
                const std::size_t decimal = std::is_floating_point_v<T> ? s_precision + 1 : 0;

                widths[index] = negative + digits + commas + decimal;
            }
        }
        else
        {
            widths[index] = 0;
        }

        ++index;
    };

    std::apply(
        [&size_of_value](const auto &...e)
        {
            (size_of_value(e), ...);
        },
        row);

    return widths;
}

//==================================================================================================
template <class... Args>
template <typename T>
bool Table<Args...>::is_zero(T value)
{
    static_assert(std::is_arithmetic_v<T>);

    if constexpr (std::is_floating_point_v<T>)
    {
        return std::abs(value) <= std::numeric_limits<T>::epsilon();
    }
    else
    {
        return value == static_cast<T>(0);
    }
}

} // namespace fly::benchmark

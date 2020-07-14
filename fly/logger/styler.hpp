#pragma once

#include "fly/logger/detail/styler_proxy.hpp"
#include "fly/traits/traits.hpp"

#include <cstdint>
#include <memory>
#include <ostream>
#include <stack>
#include <type_traits>
#include <utility>

namespace fly {

/**
 * Constants to modify the style of a std::ostream.
 *
 * Note: Windows only supports Bold and Underline.
 */
enum class Style : std::uint8_t
{
    Default,
    Blink,
    Bold,
    Dim,
    Italic,
    Strike,
    Underline,
};

/**
 * Struct to modify the foreground or background color of a std::ostream.
 */
struct Color
{
    // List of standard colors for convenience.
    static constexpr std::uint8_t Black = 0;
    static constexpr std::uint8_t Red = 1;
    static constexpr std::uint8_t Green = 2;
    static constexpr std::uint8_t Yellow = 3;
    static constexpr std::uint8_t Blue = 4;
    static constexpr std::uint8_t Magenta = 5;
    static constexpr std::uint8_t Cyan = 6;
    static constexpr std::uint8_t White = 7;

    enum class Plane
    {
        Foreground,
        Background,
    };

    /**
     * Construct a Color as either a foreground or background color.
     *
     * On Linux, the color may be an integer in the range [0, 255]. The values correspond to the
     * ANSI 256-color lookup table: https://en.wikipedia.org/wiki/ANSI_escape_code#8-bit
     *
     * On Windows, the color may only be one of the defined standard colors.
     *
     * @param color The 256-color value to apply.
     * @param plane Whether the color should apply as a foreground or background color.
     */
    Color(std::uint8_t color, Plane plane = Plane::Foreground) noexcept;

    const std::uint8_t m_color;
    const Plane m_plane;
};

/**
 * Constants to modify the cursor position of a std::ostream.
 */
enum class Position
{
    CursorUp,
    CursorDown,
    CursorForward,
    CursorBackward,
};

/**
 * IO manipulator to stylize a std::ostream with style and color. This manipulator allows for
 * applying any number of styles (e.g. bold, italic), a foreground color, and background color to
 * the std::ostream. It also allows for modifying the cursor position within the stream.
 *
 * Upon destruction, the styles and colors applied by this manipulator are reverted. Manipulations
 * of the cursor position are not reverted.
 *
 * Only standard output and error streams are supported. Any other streams will remain
 * unmanipulated.
 *
 * Not all styles and colors are supported on all platforms. Unsupported styles and colors will be
 * silently ignored.
 *
 * Callers may invoke and stream a Styler instance inline, or hold onto a Styler instance for as
 * long as desired (in which case, it will not take effect until streamed onto the std::ostream).
 *
 * Stream inline:
 *
 *     std::cout << fly::Styler(fly::Style::Bold, fly::Color::Red) << "This is bold and red\n";
 *     std::cout << "This is neither bold nor red\n";
 *
 * Scoped instance:
 *
 *     {
 *         fly::Styler styler(fly::Style::Bold, fly::Color::Red);
 *         std::cout << "This is neither bold nor red\n";
 *         std::cout << styler << "This is bold and red\n";
 *         std::cout << "This is also bold and red\n";
 *     }
 *     std::cout << "This is neither bold nor red\n";
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 11, 2020
 */
class Styler
{
public:
    /**
     * Construct a Styler with a single modifier. The template modifier type must either be one of
     * [Style, Color, Position] or an integral type. Integral types are coverted to foreground Color
     * manipulators.
     *
     * @tparam Modifier The type of the modifier to apply to the std::ostream.
     *
     * @param modifier The modifier to apply to the std::ostream.
     */
    template <typename Modifier>
    explicit Styler(Modifier &&modifier) noexcept
    {
        set_modifier(modifier);
    }

    /**
     * Construct a Styler with multiple modifiers. The template modifier types must either be one of
     * [Style, Color, Position] or an integral type. Integral types are coverted to foreground Color
     * Color manipulators.
     *
     * Any number of Style and Position instances may be used and will be combined in the Styler.
     * Only one foreground and one background Color instance may be used; if more than one of each
     * is provided, only the last instance provided will take effect.
     *
     * @tparam Modifier The type of the modifier to apply to the std::ostream.
     * @tparam Modifiers Variadic types of the remaining modifiers to apply to the std::ostream.
     *
     * @param modifier The modifier to apply to the std::ostream.
     * @param modifiers The remaining modifiers to apply to the std::ostream.
     */
    template <typename Modifier, typename... Modifiers>
    Styler(Modifier &&modifier, Modifiers &&... modifiers) noexcept :
        Styler(std::forward<Modifiers>(modifiers)...)
    {
        set_modifier(modifier);
    }

    /**
     * Stream a Styler onto a std::ostream. The Styler creates a StylerProxy instance responsible
     * for performing the underlying stream manipulations. That instance is active until the
     * streamed Styler goes out of scope.
     *
     * @param stream The stream to manipulate.
     * @param styler The Styler holding the manipulation to apply to the stream.
     *
     * @return A reference to the created StylerProxy instance.
     */
    friend detail::StylerProxy &operator<<(std::ostream &stream, const Styler &styler);

private:
    /**
     * Store a modifier as either a style, color, or position. The template modifier type must
     * either be one of [Style, Color, Position] or an integral type. Integral types are coverted to
     * foreground Color manipulators.
     *
     * @tparam Modifier The type of the modifier to apply to the std::ostream.
     *
     * @param modifier The modifier to apply to the std::ostream.
     */
    template <typename Modifier>
    void set_modifier(Modifier &&modifier)
    {
        using DecayedModifier = std::decay_t<Modifier>;

        static_assert(
            any_same_v<DecayedModifier, Style, Color, Position> ||
                std::is_integral_v<DecayedModifier>,
            "Styler can only be constructed with a valid modifier type");

        if constexpr (std::is_same_v<DecayedModifier, Style>)
        {
            m_styles.emplace(std::move(modifier));
        }
        else if constexpr (
            std::is_same_v<DecayedModifier, Color> || std::is_integral_v<DecayedModifier>)
        {
            m_colors.emplace(std::move(modifier));
        }
        else if constexpr (std::is_same_v<DecayedModifier, Position>)
        {
            m_positions.emplace(std::move(modifier));
        }
    }

    // Store modifier instances in a stack, because the variadic constructor processes parameters
    // from right-to-left (due to the delegating constructor invocation).
    std::stack<Style> m_styles;
    std::stack<Color> m_colors;
    std::stack<Position> m_positions;

    std::unique_ptr<detail::StylerProxy> m_proxy;
};

} // namespace fly

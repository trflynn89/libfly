#pragma once

#include "fly/fly.hpp"
#include "fly/logger/detail/styler_proxy.hpp"
#include "fly/traits/concepts.hpp"
#include "fly/types/numeric/literals.hpp"

#include <cstdint>
#include <memory>
#include <ostream>
#include <stack>
#include <utility>

namespace fly::logger {

/**
 * Constants to modify the style of a std::ostream.
 *
 * Note: Windows only supports Bold and Underline, and Bold can more accurately be interpreted as
 * higher intensity color.
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
    /**
     * Constants for standard colors.
     *
     * On Linux and macOS, a color may be any value in the range [0, 255]. While only the 8 standard
     * colors are listed here, any 8-bit integer value may be cast to a color. The color values
     * correspond to the ANSI 256-color lookup table:
     *
     *     https://en.wikipedia.org/wiki/ANSI_escape_code#8-bit.
     *
     * On Windows, the color may only be one of the 8 standard colors listed here.
     */
    enum StandardColor
    {
        Black,
        Red,
        Green,
        Yellow,
        Blue,
        Magenta,
        Cyan,
        White,
    };

    /**
     * Constants for the plane that should be modified.
     */
    enum Plane
    {
        Foreground,
        Background,
    };

    /**
     * Construct a Color as either a foreground or background color.
     *
     * @param color The 256-color value to apply.
     * @param plane The plane (default foreground) that should be modified.
     */
    explicit constexpr Color(std::uint8_t color, Plane plane = Foreground) noexcept :
        m_color(color),
        m_plane(plane)
    {
    }

    const std::uint8_t m_color;
    const Plane m_plane;
};

/**
 * Struct to modify the cursor position within a std::ostream.
 */
struct Cursor
{
    /**
     * Constants for the direction that the cursor should move.
     */
    enum Direction
    {
        Up,
        Down,
        Forward,
        Backward,
    };

    /**
     * Construct a Cursor instance with a direction and distance.
     *
     * @param direction The direction to move the cursor.
     * @param distance The distance to move the cursor.
     */
    constexpr Cursor(Direction direction, std::uint8_t distance = 1) noexcept :
        m_direction(direction),
        m_distance(distance > 0 ? distance : 1)
    {
    }

    const Direction m_direction;
    const std::uint8_t m_distance;
};

/**
 * Concept that is satisfied if the provided type is a valid stream modifier.
 */
template <typename T>
concept Modifier = fly::SameAsAny<T, Style, Color, Color::StandardColor, Cursor, Cursor::Direction>;

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
     * Construct a Styler with a single modifier. The template modifier type must be one of [Style,
     * Color, Color::StandardColor, Cursor, Cursor::Direction]. The enumeration [Style are coverted
     * to their parent structure types.
     *
     * @tparam ModifierType The type of the modifier to apply to the std::ostream.
     *
     * @param modifier The modifier to apply to the std::ostream.
     */
    template <Modifier ModifierType>
    explicit Styler(ModifierType &&modifier) noexcept
    {
        set_modifier(modifier);
    }

    /**
     * Construct a Styler with multiple modifiers. The template modifier types must be one of
     * [Style, Color, Color::StandardColor, Cursor, Cursor::Direction]. The enumeration types are
     * coverted to their parent structure types.
     *
     * Any number of Style and Cursor instances may be used and will be combined in the Styler. Only
     * one foreground and one background Color instance may be used; if more than one of each is
     * provided, only the last instance provided will take effect.
     *
     * @tparam ModifierType The type of the modifier to apply to the std::ostream.
     * @tparam ModifierTypes Variadic types of the remaining modifiers to apply to the std::ostream.
     *
     * @param modifier The modifier to apply to the std::ostream.
     * @param modifiers The remaining modifiers to apply to the std::ostream.
     */
    template <Modifier ModifierType, Modifier... ModifierTypes>
    Styler(ModifierType &&modifier, ModifierTypes &&...modifiers) noexcept :
        Styler(std::forward<ModifierTypes>(modifiers)...)
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
     * be one of [Style, Color, Color::StandardColor, Cursor, Cursor::Direction]. The enumeration
     * types are coverted to their parent structure types.
     *
     * @tparam ModifierType The type of the modifier to apply to the std::ostream.
     *
     * @param modifier The modifier to apply to the std::ostream.
     */
    template <Modifier ModifierType>
    void set_modifier(ModifierType &&modifier)
    {
        if constexpr (fly::SameAs<ModifierType, Style>)
        {
            m_styles.emplace(std::move(modifier));
        }
        else if constexpr (fly::SameAsAny<ModifierType, Color, Color::StandardColor>)
        {
            m_colors.emplace(std::move(modifier));
        }
        else if constexpr (fly::SameAsAny<ModifierType, Cursor, Cursor::Direction>)
        {
            m_cursors.emplace(std::move(modifier));
        }
    }

    // Store modifier instances in a stack because the variadic constructor processes parameters
    // from right-to-left (due to the delegating constructor invocation).
    std::stack<Style> m_styles;
    std::stack<Color> m_colors;
    std::stack<Cursor> m_cursors;

    std::unique_ptr<detail::StylerProxy> m_proxy;
};

} // namespace fly::logger

namespace fly {
inline namespace literals {
    inline namespace styler_literals {

        /**
         * Type-safe integer literal suffix to construct a Color as a foreground color. The integer
         * literal must be in the range [0, 255].
         *
         * @tparam Literals The numeric literals from which to construct a 256-color value.
         *
         * @return The constructed Color.
         */
        template <char... Literals>
        FLY_CONSTEVAL inline fly::logger::Color operator"" _c()
        {
            // Convert to std::uint8_t via numeric literal to ensure the provided color is valid.
            const std::uint8_t validated_color = operator"" _u8<Literals...>();
            return fly::logger::Color(validated_color);
        }

    } // namespace styler_literals
} // namespace literals
} // namespace fly

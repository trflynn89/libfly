#pragma once

#include "fly/fly.hpp"
#include "fly/types/string/format.hpp"

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

#define FLY_EXPAND(...) FLY_EXPAND0(FLY_EXPAND0(FLY_EXPAND0(FLY_EXPAND0(__VA_ARGS__))))
#define FLY_EXPAND0(...) FLY_EXPAND1(FLY_EXPAND1(FLY_EXPAND1(FLY_EXPAND1(__VA_ARGS__))))
#define FLY_EXPAND1(...) FLY_EXPAND2(FLY_EXPAND2(FLY_EXPAND2(FLY_EXPAND2(__VA_ARGS__))))
#define FLY_EXPAND2(...) FLY_EXPAND3(FLY_EXPAND3(FLY_EXPAND3(FLY_EXPAND3(__VA_ARGS__))))
#define FLY_EXPAND3(...) __VA_ARGS__

#define FLY_PARENS ()

#define FLY_STRINGIZE_ARGS(...) __VA_OPT__(FLY_EXPAND(FLY_STRINGIZE_HELPER(__VA_ARGS__)))
#define FLY_STRINGIZE_HELPER(arg, ...)                                                             \
    FLY_STRINGIZE(arg) __VA_OPT__(, FLY_STRINGIZE_RECURSE FLY_PARENS(__VA_ARGS__))
#define FLY_STRINGIZE_RECURSE() FLY_STRINGIZE_HELPER

#define FLY_ASSERT_IMPL(handler, expression, ...)                                                  \
    do                                                                                             \
    {                                                                                              \
        if (!static_cast<bool>(expression)) [[unlikely]]                                           \
        {                                                                                          \
            static constexpr auto capture_names =                                                  \
                fly::detail::make_captured_value_names_array(FLY_STRINGIZE_ARGS(__VA_ARGS__));     \
                                                                                                   \
            fly::detail::Assertion assertion(                                                      \
                #expression,                                                                       \
                __FILE__,                                                                          \
                __FUNCTION__,                                                                      \
                static_cast<std::uint32_t>(__LINE__),                                              \
                capture_names);                                                                    \
                                                                                                   \
            assertion.assertion_failed(__VA_ARGS__);                                               \
            handler();                                                                             \
        }                                                                                          \
    } while (0)

namespace fly::detail {

/**
 * @return The stringified names of any captured values converted to an array.
 */
template <typename... Ts>
consteval std::array<std::string_view, sizeof...(Ts)>
make_captured_value_names_array(Ts &&...capture_names)
{
    return {{std::forward<Ts>(capture_names)...}};
}

/**
 * A container to hold a type-erased value captured by the assertion macros.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version April 24, 2022
 */
class Capture
{
public:
    /**
     * Constructor. Initialize the capture to store a type-erased value.
     *
     * @tparam CaptureType The type of the captured value.
     *
     * @param capture The captured value.
     */
    template <typename CaptureType>
    explicit Capture(CaptureType const &capture);

    /**
     * Re-form the type-erased value and format that value to a string.
     *
     * @param capture_name The stringified name of the captured value.
     *
     * @return The formatted value.
     */
    std::string format(std::string_view capture_name) const;

private:
    /**
     * Re-form the type-erased value and format that value to a string.
     *
     * @param capture_name The stringified name of the captured value.
     * @param capture A pointer to the type-erased captured value.
     *
     * @return The formatted value.
     */
    template <typename CaptureType>
    static std::string format_capture(std::string_view capture_name, void const *capture);

    void const *m_value {nullptr};
    std::string (*m_format)(std::string_view, void const *) {nullptr};
};

/**
 * Class to log information about a failed assertion.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version April 24, 2022
 */
class Assertion
{
public:
    /**
     * Constructor.
     *
     * @param expression The stringified failed assertion expression.
     * @param file The file in which the assertion failed.
     * @param line The line on which the assertion failed.
     * @param function The function in which the assertion failed.
     * @param capture_names The stringified names of any captured values.
     */
    Assertion(
        std::string_view expression,
        std::string_view file,
        std::string_view function,
        std::uint32_t line,
        std::span<std::string_view const> capture_names);

    /**
     * Log a failed assertion and any captured values.
     *
     * @tparam CaptureTypes The types of the captured values.
     *
     * @param captures Values captured by the assertion macros.
     */
    template <typename... CaptureTypes>
    void assertion_failed(CaptureTypes &&...captures);

    /**
     * Log a failed assertion and any captured values with an additional message.
     *
     * @tparam CaptureTypes The types of the captured values.
     *
     * @param message A message to display alongside the failed assertion.
     * @param captures Values captured by the assertion macros.
     */
    template <std::size_t N, typename... CaptureTypes>
    void assertion_failed(char const (&message)[N], CaptureTypes &&...captures);

private:
    /**
     * Log a failed assertion and any captured values with an optional additional message.
     *
     * @param message An (optionally empty) message to display alongside the failed assertion.
     * @param captures Values captured by the assertion macros.
     */
    void log_assertion(std::string_view message, std::span<Capture const> captures) const;

    std::string_view m_expression;
    std::string_view m_file;
    std::string_view m_function;
    std::uint32_t m_line {0};
    std::span<std::string_view const> m_capture_names;
};

//==================================================================================================
template <typename CaptureType>
Capture::Capture(CaptureType const &capture) :
    m_value(static_cast<void const *>(&capture)),
    m_format(format_capture<CaptureType>)
{
}

//==================================================================================================
template <typename CaptureType>
std::string Capture::format_capture(std::string_view capture_name, void const *capture)
{
    auto const &reformed_capture = *static_cast<CaptureType const *>(capture);
    return fly::string::format("\t{} => {}\n", capture_name, reformed_capture);
}

//==================================================================================================
template <typename... CaptureTypes>
void Assertion::assertion_failed(CaptureTypes &&...captures)
{
    std::array<Capture, sizeof...(CaptureTypes)> parameters {
        Capture {std::forward<CaptureTypes>(captures)}...};

    log_assertion({}, parameters);
}

//==================================================================================================
template <std::size_t N, typename... CaptureTypes>
void Assertion::assertion_failed(char const (&message)[N], CaptureTypes &&...captures)
{
    std::array<Capture, sizeof...(CaptureTypes)> parameters {
        Capture {std::forward<CaptureTypes>(captures)}...};

    m_capture_names = m_capture_names.subspan(1);
    log_assertion({message, N}, parameters);
}

} // namespace fly::detail

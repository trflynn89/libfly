#pragma once

#include <ostream>

namespace fly::logger::detail {

/**
 * IO manipulator proxy responsible for performing the underlying stream manipulations.
 *
 * This class essentially exists as a common base class for OS dependent implementations to inherit.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 11, 2020
 */
class StylerProxy
{
public:
    /**
     * Constructor. Concrete implementations should use their constructor to manipulate the provided
     * stream.
     *
     * @param stream The stream to manipulate.
     */
    explicit StylerProxy(std::ostream &stream) noexcept;

    /**
     * Destructor. Concrete implementations should use their destructor to reset the stream to its
     * original state.
     */
    virtual ~StylerProxy() = default;

    /**
     * Proxy streaming implementation to stream any value onto a proxy's stored stream.
     *
     * @param proxy The StylerProxy instance holding the stream.
     * @param value The value to stream.
     *
     * @return A reference to the stored stream.
     */
    template <typename T>
    friend std::ostream &operator<<(const StylerProxy &proxy, const T &value)
    {
        return proxy.m_stream << value;
    }

protected:
    std::ostream &m_stream;

    const bool m_stream_is_stdout;
    const bool m_stream_is_stderr;
};

} // namespace fly::logger::detail

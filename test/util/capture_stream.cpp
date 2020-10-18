#include "test/util/capture_stream.hpp"

#include "fly/fly.hpp"

#include <catch2/catch.hpp>

#include <cstdio>
#include <cstdlib>

#if defined(FLY_LINUX) || defined(FLY_MACOS)
#    include <unistd.h>
#elif defined(FLY_WINDOWS)
#    include <io.h>
#    define close _close
#    define dup _dup
#    define dup2 _dup2
#    define fileno _fileno
#else
#    error Unknown file IO include.
#endif

namespace fly::test {

//==================================================================================================
CaptureStream::CaptureStream(Stream stream) noexcept :
    m_file(m_path.file()),
    m_stream(stream),
    m_stdio(-1),
    m_original(-1)
{
    FILE *target = nullptr;

#if defined(FLY_LINUX) || defined(FLY_MACOS)
    target = ::fopen(m_file.string().c_str(), "w");
#elif defined(FLY_WINDOWS)
    ::fopen_s(&target, m_file.string().c_str(), "w");
#else
#    error Unknown file open command.
#endif

    CATCH_REQUIRE(target != nullptr);
    int target_fd = ::fileno(target);

    switch (m_stream)
    {
        case Stream::Stdout:
            m_stdio = ::fileno(stdout);
            ::fflush(stdout);
            break;

        case Stream::Stderr:
            m_stdio = ::fileno(stderr);
            ::fflush(stderr);
            break;
    }

    m_original = ::dup(m_stdio);
    ::dup2(target_fd, m_stdio);

    ::fclose(target);
}

//==================================================================================================
CaptureStream::~CaptureStream()
{
    restore(false);
}

//==================================================================================================
std::string CaptureStream::operator()()
{
    return restore(true);
}

//==================================================================================================
std::string CaptureStream::restore(bool read)
{
    std::string contents;

    if (m_original != -1)
    {
        switch (m_stream)
        {
            case Stream::Stdout:
                ::fflush(stdout);
                break;

            case Stream::Stderr:
                ::fflush(stderr);
                break;
        }

        ::dup2(m_original, m_stdio);
        ::close(m_original);

        if (read)
        {
            contents = fly::test::PathUtil::read_file(m_file);
        }

        m_original = -1;
    }

    return contents;
}

} // namespace fly::test

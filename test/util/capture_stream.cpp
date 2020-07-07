#include "test/util/capture_stream.hpp"

#include "fly/fly.hpp"

#include <cstdio>
#include <cstdlib>

#if defined(FLY_WINDOWS)
#    include <io.h>

#    define close _close
#    define dup _dup
#    define dup2 _dup2
#    define fileno _fileno
#elif defined(FLY_LINUX)
#    include <unistd.h>
#endif

namespace fly {

//==================================================================================================
CaptureStream::CaptureStream(Stream stream) noexcept :
    m_file(m_path.file()),
    m_stdio(-1),
    m_original(-1)
{
    FILE *target = nullptr;

#if defined(FLY_WINDOWS)
    ::fopen_s(&target, m_file.string().c_str(), "w");
#elif defined(FLY_LINUX)
    target = ::fopen(m_file.string().c_str(), "w");
#endif

    if (target != nullptr)
    {
        int target_fd = ::fileno(target);

        switch (stream)
        {
            case Stream::Stdout:
                m_stdio = ::fileno(stdout);
                break;

            case Stream::Stderr:
                m_stdio = ::fileno(stderr);
                break;
        }

        m_original = ::dup(m_stdio);
        ::dup2(target_fd, m_stdio);

        ::fclose(target);
    }
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
        ::dup2(m_original, m_stdio);
        ::close(m_original);

        if (read)
        {
            contents = fly::PathUtil::read_file(m_file);
        }

        m_original = -1;
    }

    return contents;
}

} // namespace fly

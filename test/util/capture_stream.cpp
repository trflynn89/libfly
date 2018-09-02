#include "test/util/capture_stream.h"

#include <cstdio>
#include <cstdlib>

#include "fly/fly.h"

#if defined(FLY_WINDOWS)
    #include <io.h>

    #define dup _dup
    #define dup2 _dup2
    #define fileno _fileno
#elif defined(FLY_LINUX)
    #include <unistd.h>
#endif

#include "test/util/path_util.h"

namespace fly {

//==============================================================================
CaptureStream::CaptureStream(Stream stream) :
    m_path(fly::PathUtil::GenerateTempDirectory()),
    m_stream(stream),
    m_stdio(-1),
    m_original(-1)
{
    FILE *target = ::fopen(m_path.c_str(), "w");

    if (target != nullptr)
    {
        int targetfd = fileno(target);

        switch (m_stream)
        {
        case Stream::Stdout:
            m_stdio = fileno(stdout);
            break;

        case Stream::Stderr:
            m_stdio = fileno(stderr);
            break;
        }

        m_original = dup(m_stdio);
        dup2(targetfd, m_stdio);

        ::fclose(target);
    }
}

//==============================================================================
CaptureStream::~CaptureStream()
{
    if (m_original != -1)
    {
        ::dup2(m_original, m_stdio);
        ::close(m_original);

        std::remove(m_path.c_str());
    }
}

//==============================================================================
std::string CaptureStream::operator() () const
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

    return fly::PathUtil::ReadFile(m_path);
}

}

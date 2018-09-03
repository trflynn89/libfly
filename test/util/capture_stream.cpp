#include "test/util/capture_stream.h"

#include <cstdio>
#include <cstdlib>

#include "fly/fly.h"

#if defined(FLY_WINDOWS)
    #include <io.h>

    #define close _close
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
    FILE *target = nullptr;

#if defined(FLY_WINDOWS)
    ::fopen_s(&target, m_path.c_str(), "w");
#elif defined(FLY_LINUX)
    target = ::fopen(m_path.c_str(), "w");
#endif

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
    restore(false);
}

//==============================================================================
std::string CaptureStream::operator() ()
{
    return restore(true);
}

//==============================================================================
std::string CaptureStream::restore(bool read)
{
    std::string contents;

    if (m_original != -1)
    {
        dup2(m_original, m_stdio);
        close(m_original);

        if (read)
        {
            contents = fly::PathUtil::ReadFile(m_path);
        }

        std::remove(m_path.c_str());
        m_original = -1;
    }

    return contents;
}

}

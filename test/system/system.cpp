#include "fly/system/system.hpp"

#include "test/util/capture_stream.hpp"

#include "fly/fly.hpp"

#if defined(FLY_LINUX)
#    include "test/mock/mock_system.hpp"
#endif

#include "catch2/catch_test_macros.hpp"

#include <csignal>
#include <string>

namespace {

int s_last_signal = 0;

void handle_signal(int signal)
{
    s_last_signal = signal;
}

class ScopedSignalHandler
{
public:
    ScopedSignalHandler()
    {
        auto handler = std::bind(&ScopedSignalHandler::handle_signal, this, std::placeholders::_1);
        fly::system::set_signal_handler(std::move(handler));
    }

    ~ScopedSignalHandler()
    {
        fly::system::set_signal_handler(nullptr);
    }

    int operator()() const
    {
        return m_last_signal;
    }

private:
    void handle_signal(int signal)
    {
        m_last_signal = signal;
    }

    int m_last_signal {0};
};

class StaticSignalHandler
{
public:
    static void handle_signal(int signal)
    {
        s_last_signal = signal;
    }

    static int s_last_signal;
};

int StaticSignalHandler::s_last_signal = 0;

} // namespace

CATCH_TEST_CASE("System", "[system]")
{
    CATCH_SECTION("Print a backtrace to stderr")
    {
        fly::test::CaptureStream capture(fly::test::CaptureStream::Stream::Stderr);
        fly::system::print_backtrace();

        std::string output = capture();
        CATCH_CHECK_FALSE(output.empty());
    }

#if defined(FLY_LINUX)

    CATCH_SECTION("Printing a backtrace fails when ::backtrace() fails")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Backtrace);

        fly::test::CaptureStream capture(fly::test::CaptureStream::Stream::Stderr);
        fly::system::print_backtrace();

        std::string output = capture();
        CATCH_CHECK(output.empty());
    }

    CATCH_SECTION("Printing a backtrace fails when ::backtrace_symbols_fd() fails")
    {
        fly::test::MockSystem mock(fly::test::MockCall::BacktraceSymbols);

        fly::test::CaptureStream capture(fly::test::CaptureStream::Stream::Stderr);
        fly::system::print_backtrace();

        std::string output = capture();
        CATCH_CHECK(output.empty());
    }

#endif

    CATCH_SECTION("Capture the system's local time")
    {
        std::string time = fly::system::local_time();
        CATCH_CHECK_FALSE(time.empty());
    }

#if defined(FLY_LINUX)

    CATCH_SECTION("Capturing the system's local time fails when ::localtime() fails")
    {
        fly::test::MockSystem mock(fly::test::MockCall::LocalTime);

        std::string time = fly::system::local_time();
        CATCH_CHECK(time.empty());
    }

#endif

    CATCH_SECTION("Capture the system's last error code as an integer and string")
    {
        int code = fly::system::get_error_code();

        std::string error1 = fly::system::get_error_string();
        std::string error2 = fly::system::get_error_string(code);

        CATCH_CHECK_FALSE(error1.empty());
        CATCH_CHECK_FALSE(error2.empty());
        CATCH_CHECK(error1 == error2);
    }

    CATCH_SECTION("Setup a custom signal handler with global method")
    {
        fly::system::SignalHandler handler(&handle_signal);
        fly::system::set_signal_handler(std::move(handler));

        std::raise(SIGINT);
        CATCH_CHECK(s_last_signal == SIGINT);

        std::raise(SIGSEGV);
        CATCH_CHECK(s_last_signal == SIGSEGV);

        fly::system::set_signal_handler(nullptr);
    }

    CATCH_SECTION("Setup a custom signal handler with instance class method")
    {
        ScopedSignalHandler handler;

        std::raise(SIGINT);
        CATCH_CHECK(handler() == SIGINT);

        std::raise(SIGSEGV);
        CATCH_CHECK(handler() == SIGSEGV);
    }

    CATCH_SECTION("Setup a custom signal handler with static class method")
    {
        auto handler = std::bind(&StaticSignalHandler::handle_signal, std::placeholders::_1);
        fly::system::set_signal_handler(std::move(handler));

        std::raise(SIGINT);
        CATCH_CHECK(StaticSignalHandler::s_last_signal == SIGINT);

        std::raise(SIGSEGV);
        CATCH_CHECK(StaticSignalHandler::s_last_signal == SIGSEGV);

        fly::system::set_signal_handler(nullptr);
    }

    CATCH_SECTION("Setup a custom signal handler with lambda")
    {
        int last_signal = 0;
        fly::system::set_signal_handler(
            [&last_signal](int signal)
            {
                last_signal = signal;
            });

        std::raise(SIGINT);
        CATCH_CHECK(last_signal == SIGINT);

        std::raise(SIGSEGV);
        CATCH_CHECK(last_signal == SIGSEGV);

        fly::system::set_signal_handler(nullptr);
    }
}

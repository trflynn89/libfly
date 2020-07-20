#include "fly/system/system.hpp"

#include "fly/fly.hpp"
#include "test/util/capture_stream.hpp"

#if defined(FLY_LINUX)
#    include "test/mock/mock_system.hpp"
#endif

#include <catch2/catch.hpp>

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
        fly::System::set_signal_handler(std::move(handler));
    }

    ~ScopedSignalHandler()
    {
        fly::System::set_signal_handler(nullptr);
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

TEST_CASE("System", "[system]")
{
    SECTION("Print a backtrace to stderr")
    {
        fly::test::CaptureStream capture(fly::test::CaptureStream::Stream::Stderr);
        fly::System::print_backtrace();

        std::string output = capture();
        CHECK_FALSE(output.empty());
    }

#if defined(FLY_LINUX)

    SECTION("Printing a backtrace fails when ::backtrace() fails")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Backtrace);

        fly::test::CaptureStream capture(fly::test::CaptureStream::Stream::Stderr);
        fly::System::print_backtrace();

        std::string output = capture();
        CHECK(output.empty());
    }

    SECTION("Printing a backtrace fails when ::backtrace_symbols_fd() fails")
    {
        fly::test::MockSystem mock(fly::test::MockCall::BacktraceSymbols);

        fly::test::CaptureStream capture(fly::test::CaptureStream::Stream::Stderr);
        fly::System::print_backtrace();

        std::string output = capture();
        CHECK(output.empty());
    }

#endif

    SECTION("Capture the system's local time")
    {
        std::string time = fly::System::local_time();
        CHECK_FALSE(time.empty());
    }

#if defined(FLY_LINUX)

    SECTION("Capturing the system's local time fails when ::localtime() fails")
    {
        fly::test::MockSystem mock(fly::test::MockCall::LocalTime);

        std::string time = fly::System::local_time();
        CHECK(time.empty());
    }

#endif

    SECTION("Capture the system's last error code as an integer and string")
    {
        int code = fly::System::get_error_code();

        std::string error1 = fly::System::get_error_string();
        std::string error2 = fly::System::get_error_string(code);

        CHECK_FALSE(error1.empty());
        CHECK_FALSE(error2.empty());
        CHECK(error1 == error2);
    }

    SECTION("Setup a custom signal handler with global method")
    {
        fly::System::SignalHandler handler(&handle_signal);
        fly::System::set_signal_handler(std::move(handler));

        std::raise(SIGINT);
        CHECK(s_last_signal == SIGINT);

        std::raise(SIGSEGV);
        CHECK(s_last_signal == SIGSEGV);

        fly::System::set_signal_handler(nullptr);
    }

    SECTION("Setup a custom signal handler with instance class method")
    {
        ScopedSignalHandler handler;

        std::raise(SIGINT);
        CHECK(handler() == SIGINT);

        std::raise(SIGSEGV);
        CHECK(handler() == SIGSEGV);
    }

    SECTION("Setup a custom signal handler with static class method")
    {
        auto handler = std::bind(&StaticSignalHandler::handle_signal, std::placeholders::_1);
        fly::System::set_signal_handler(std::move(handler));

        std::raise(SIGINT);
        CHECK(StaticSignalHandler::s_last_signal == SIGINT);

        std::raise(SIGSEGV);
        CHECK(StaticSignalHandler::s_last_signal == SIGSEGV);

        fly::System::set_signal_handler(nullptr);
    }

    SECTION("Setup a custom signal handler with lambda")
    {
        int last_signal = 0;
        fly::System::set_signal_handler([&last_signal](int signal) { last_signal = signal; });

        std::raise(SIGINT);
        CHECK(last_signal == SIGINT);

        std::raise(SIGSEGV);
        CHECK(last_signal == SIGSEGV);

        fly::System::set_signal_handler(nullptr);
    }
}

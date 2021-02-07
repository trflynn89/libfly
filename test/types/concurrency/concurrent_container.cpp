#include "fly/types/concurrency/concurrent_queue.hpp"
#include "fly/types/concurrency/concurrent_stack.hpp"
#include "fly/types/numeric/literals.hpp"

#include "catch2/catch_template_test_macros.hpp"
#include "catch2/catch_test_macros.hpp"

#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <type_traits>
#include <vector>

using namespace fly::literals::numeric_literals;

CATCH_TEMPLATE_PRODUCT_TEST_CASE(
    "ConcurrentContainer",
    "[concurrency]",
    (fly::ConcurrentQueue, fly::ConcurrentStack),
    (std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t))
{
    using size_type = typename TestType::size_type;
    using value_type = typename TestType::value_type;

    TestType container;

    CATCH_SECTION("Queues must be empty upon creation")
    {
        CATCH_CHECK(container.empty());
        CATCH_CHECK(container.size() == 0);
    }

    CATCH_SECTION("Cannot pop from an empty queue")
    {
        value_type obj1;
        value_type obj2(1_u32);

        // Make sure pop is initially invalid.
        CATCH_REQUIRE_FALSE(container.pop(obj1, std::chrono::milliseconds(0)));

        // Push an item onto the queue and immediately pop it.
        container.push(std::move(obj2));
        CATCH_REQUIRE(container.pop(obj1, std::chrono::milliseconds(0)));

        // Make sure popping an item from the no-longer non-empty queue is invalid.
        CATCH_REQUIRE_FALSE(container.pop(obj1, std::chrono::milliseconds(0)));
    }

    CATCH_SECTION("Push onto and pop from a queue on a single thread")
    {
        size_type size = 0;

        value_type obj1(1_u32);
        value_type obj2(2_u32);
        value_type obj3(3_u32);

        auto push = [&container](value_type object, size_type expected_size)
        {
            container.push(std::move(object));

            CATCH_CHECK(container.size() == expected_size);
            CATCH_CHECK_FALSE(container.empty());
        };

        auto pop = [&container](value_type expected_object, size_type expected_size)
        {
            value_type object;

            CATCH_REQUIRE(container.pop(object, std::chrono::milliseconds(0)));
            CATCH_CHECK(container.size() == expected_size);
            CATCH_CHECK(object == expected_object);
        };

        push(obj1, ++size);
        push(obj1, ++size);
        pop(obj1, --size);
        push(obj2, ++size);
        push(obj3, ++size);

        if (std::is_same_v<TestType, fly::ConcurrentQueue<value_type>>)
        {
            pop(obj1, --size);
            pop(obj2, --size);
            pop(obj3, --size);
        }
        else
        {
            pop(obj3, --size);
            pop(obj2, --size);
            pop(obj1, --size);
        }
    }

    CATCH_SECTION("Push onto and pop from a queue on multiple threads")
    {
        auto run_multi_threaded_test = [&container](std::size_t writers, std::size_t readers)
        {
            auto writer_thread = [&container]() -> std::size_t
            {
                std::size_t writes = 100;

                for (std::size_t i = 0; i < writes; ++i)
                {
                    value_type value = static_cast<value_type>(i);
                    container.push(std::move(value));
                }

                return writes;
            };

            auto reader_thread = [&container](std::atomic_bool &finished_writes) -> std::size_t
            {
                std::size_t reads = 0;

                while (!finished_writes.load() || !container.empty())
                {
                    value_type value;

                    if (container.pop(value, std::chrono::milliseconds(10)))
                    {
                        ++reads;
                    }
                }

                return reads;
            };

            std::vector<std::future<std::size_t>> writer_futures;
            std::vector<std::future<std::size_t>> reader_futures;

            std::atomic_bool finished_writes(false);

            for (std::size_t i = 0; i < writers; ++i)
            {
                writer_futures.push_back(std::async(std::launch::async, writer_thread));
            }

            for (std::size_t i = 0; i < readers; ++i)
            {
                auto func = std::bind(reader_thread, std::ref(finished_writes));
                reader_futures.push_back(std::async(std::launch::async, std::move(func)));
            }

            std::size_t writes = 0;
            std::size_t reads = 0;

            for (auto &future : writer_futures)
            {
                CATCH_REQUIRE(future.valid());
                writes += future.get();
            }

            finished_writes.store(true);

            for (auto &future : reader_futures)
            {
                CATCH_REQUIRE(future.valid());
                reads += future.get();
            }

            CATCH_CHECK(writes == reads);
        };

        run_multi_threaded_test(1, 1);
        run_multi_threaded_test(1, 4);
        run_multi_threaded_test(4, 1);
        run_multi_threaded_test(4, 4);
    }

    CATCH_SECTION("Pop from a queue while blocking indefinitely")
    {
        auto infinite_wait_thread = [&container]() -> value_type
        {
            value_type value;
            container.pop(value);

            return value;
        };

        value_type obj(123_u32);

        std::future<value_type> future = std::async(std::launch::async, infinite_wait_thread);

        std::future_status status = future.wait_for(std::chrono::milliseconds(1));
        CATCH_REQUIRE(status == std::future_status::timeout);

        container.push(std::move(value_type(obj)));

        status = future.wait_for(std::chrono::milliseconds(10));
        CATCH_REQUIRE(status == std::future_status::ready);
        CATCH_CHECK(future.get() == obj);
    }
}

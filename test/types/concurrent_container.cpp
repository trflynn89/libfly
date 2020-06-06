#include "fly/types/concurrency/concurrent_queue.hpp"
#include "fly/types/numeric/literals.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <vector>

//==================================================================================================
class ConcurrencyTest : public ::testing::Test
{
public:
    using Object = unsigned int;
    using ObjectQueue = fly::ConcurrentQueue<Object>;

    unsigned int writer_thread(ObjectQueue &object_queue) noexcept
    {
        unsigned int writes = 100;

        for (unsigned int i = 0; i < writes; ++i)
        {
            Object object(i);
            object_queue.push(std::move(object));
        }

        return writes;
    }

    unsigned int
    reader_thread(ObjectQueue &object_queue, std::atomic_bool &finished_writes) noexcept
    {
        unsigned int reads = 0;

        while (!finished_writes.load() || !object_queue.empty())
        {
            Object object;

            if (object_queue.pop(object, std::chrono::milliseconds(10)))
            {
                ++reads;
            }
        }

        return reads;
    }

    Object infinite_wait_reader_thread(ObjectQueue &object_queue) noexcept
    {
        Object object;
        object_queue.pop(object);

        return object;
    }

protected:
    void run_multi_threaded_test(unsigned int writers, unsigned int readers) noexcept
    {
        ObjectQueue object_queue;

        std::vector<std::future<unsigned int>> writer_futures;
        std::vector<std::future<unsigned int>> reader_futures;

        std::atomic_bool finished_writes(false);

        for (unsigned int i = 0; i < writers; ++i)
        {
            auto func = std::bind(&ConcurrencyTest::writer_thread, this, std::ref(object_queue));
            writer_futures.push_back(std::async(std::launch::async, func));
        }

        for (unsigned int i = 0; i < readers; ++i)
        {
            auto func = std::bind(
                &ConcurrencyTest::reader_thread,
                this,
                std::ref(object_queue),
                std::ref(finished_writes));
            reader_futures.push_back(std::async(std::launch::async, func));
        }

        unsigned int writes = 0;
        unsigned int reads = 0;

        for (auto &future : writer_futures)
        {
            ASSERT_TRUE(future.valid());
            writes += future.get();
        }

        finished_writes.store(true);

        for (auto &future : reader_futures)
        {
            ASSERT_TRUE(future.valid());
            reads += future.get();
        }

        ASSERT_EQ(writes, reads);
    }

    void do_queue_push(
        ObjectQueue &object_queue,
        Object object,
        ObjectQueue::size_type expected_size) noexcept
    {
        object_queue.push(std::move(object));

        ASSERT_EQ(object_queue.size(), expected_size);
        ASSERT_FALSE(object_queue.empty());
    }

    void do_queue_pop(
        ObjectQueue &object_queue,
        const Object &expected_object,
        ObjectQueue::size_type expected_size) noexcept
    {
        Object object;

        ASSERT_TRUE(object_queue.pop(object, std::chrono::milliseconds(0)));
        ASSERT_EQ(object_queue.size(), expected_size);
        ASSERT_EQ(object, expected_object);
    }
};

//==================================================================================================
TEST_F(ConcurrencyTest, EmptyQueueUponCreation)
{
    ObjectQueue object_queue;

    ASSERT_TRUE(object_queue.empty());
    ASSERT_EQ(object_queue.size(), 0);
}

//==================================================================================================
TEST_F(ConcurrencyTest, PopFromEmptyQueue)
{
    ObjectQueue object_queue;

    Object obj1;
    Object obj2(1_u32);

    // Make sure pop is initially invalid
    ASSERT_FALSE(object_queue.pop(obj1, std::chrono::milliseconds(0)));

    // Push an item onto the queue and immediately pop it
    object_queue.push(std::move(obj2));
    ASSERT_TRUE(object_queue.pop(obj1, std::chrono::milliseconds(0)));

    // Make sure popping an item from the no-longer non-empty queue is invalid
    ASSERT_FALSE(object_queue.pop(obj1, std::chrono::milliseconds(0)));
}

//==================================================================================================
TEST_F(ConcurrencyTest, SingleThreaded)
{
    ObjectQueue object_queue;
    ObjectQueue::size_type size = 0;

    Object obj1(1_u32);
    Object obj2(2_u32);
    Object obj3(3_u32);

    do_queue_push(object_queue, obj1, ++size);
    do_queue_push(object_queue, obj1, ++size);
    do_queue_pop(object_queue, obj1, --size);
    do_queue_push(object_queue, obj2, ++size);
    do_queue_push(object_queue, obj3, ++size);
    do_queue_pop(object_queue, obj1, --size);
    do_queue_pop(object_queue, obj2, --size);
    do_queue_pop(object_queue, obj3, --size);
}

//==================================================================================================
TEST_F(ConcurrencyTest, MultiThreaded)
{
    run_multi_threaded_test(1, 1);
    run_multi_threaded_test(1, 100);
    run_multi_threaded_test(100, 1);
    run_multi_threaded_test(100, 100);
}

//==================================================================================================
TEST_F(ConcurrencyTest, InfiniteWaitReader)
{
    ObjectQueue object_queue;
    Object obj(123_u32);

    auto func =
        std::bind(&ConcurrencyTest::infinite_wait_reader_thread, this, std::ref(object_queue));
    std::future<Object> future = std::async(std::launch::async, func);

    std::future_status status = future.wait_for(std::chrono::milliseconds(10));
    ASSERT_EQ(status, std::future_status::timeout);

    object_queue.push(std::move(Object(obj)));

    status = future.wait_for(std::chrono::milliseconds(10));
    ASSERT_EQ(status, std::future_status::ready);
    ASSERT_EQ(future.get(), obj);
}

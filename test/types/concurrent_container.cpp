#include "fly/types/concurrent_queue.h"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <vector>

//==============================================================================
class ConcurrencyTest : public ::testing::Test
{
public:
    typedef int Object;
    typedef fly::ConcurrentQueue<Object> ObjectQueue;

    unsigned int WriterThread(ObjectQueue &objectQueue) noexcept
    {
        unsigned int numWrites = 100;

        for (unsigned int i = 0; i < numWrites; ++i)
        {
            Object object(i);
            objectQueue.Push(std::move(object));
        }

        return numWrites;
    }

    unsigned int ReaderThread(
        ObjectQueue &objectQueue,
        std::atomic_bool &finishedWrites) noexcept
    {
        unsigned int numReads = 0;

        while (!finishedWrites.load() || !objectQueue.IsEmpty())
        {
            Object object;

            if (objectQueue.Pop(object, std::chrono::milliseconds(10)))
            {
                ++numReads;
            }
        }

        return numReads;
    }

    Object InfiniteWaitReaderThread(ObjectQueue &objectQueue) noexcept
    {
        Object object;
        objectQueue.Pop(object);

        return object;
    }

protected:
    void RunMultiThreadedTest(
        unsigned int numWriters,
        unsigned int numReaders) noexcept
    {
        ObjectQueue objectQueue;

        std::vector<std::future<unsigned int>> writerFutures;
        std::vector<std::future<unsigned int>> readerFutures;

        std::atomic_bool finishedWrites(false);

        // Create numWriters writer threads
        for (unsigned int i = 0; i < numWriters; ++i)
        {
            auto func = std::bind(
                &ConcurrencyTest::WriterThread, this, std::ref(objectQueue));
            writerFutures.push_back(std::async(std::launch::async, func));
        }

        // Create numReaders reader threads
        for (unsigned int i = 0; i < numReaders; ++i)
        {
            auto func = std::bind(
                &ConcurrencyTest::ReaderThread,
                this,
                std::ref(objectQueue),
                std::ref(finishedWrites));
            readerFutures.push_back(std::async(std::launch::async, func));
        }

        unsigned int numWrites = 0;
        unsigned int numReads = 0;

        for (auto &future : writerFutures)
        {
            ASSERT_TRUE(future.valid());
            numWrites += future.get();
        }

        finishedWrites.store(true);

        for (auto &future : readerFutures)
        {
            ASSERT_TRUE(future.valid());
            numReads += future.get();
        }

        ASSERT_EQ(numWrites, numReads);
    }

    void DoQueuePush(
        ObjectQueue &objectQueue,
        Object object,
        ObjectQueue::size_type expectedSize) noexcept
    {
        objectQueue.Push(std::move(object));

        ASSERT_EQ(objectQueue.Size(), expectedSize);
        ASSERT_FALSE(objectQueue.IsEmpty());
    }

    void DoQueuePop(
        ObjectQueue &objectQueue,
        const Object &expectedObject,
        ObjectQueue::size_type expectedSize) noexcept
    {
        Object object;

        ASSERT_TRUE(objectQueue.Pop(object, std::chrono::milliseconds(0)));
        ASSERT_EQ(objectQueue.Size(), expectedSize);
        ASSERT_EQ(object, expectedObject);
    }
};

//==============================================================================
TEST_F(ConcurrencyTest, EmptyQueueUponCreationTest)
{
    ObjectQueue objectQueue;

    ASSERT_TRUE(objectQueue.IsEmpty());
    ASSERT_EQ(objectQueue.Size(), 0);
}

//==============================================================================
TEST_F(ConcurrencyTest, PopFromEmptyQueueTest)
{
    ObjectQueue objectQueue;

    Object obj1;
    Object obj2(1);

    // Make sure pop is initially invalid
    ASSERT_FALSE(objectQueue.Pop(obj1, std::chrono::milliseconds(0)));

    // Push an item onto the queue and immediately pop it
    objectQueue.Push(std::move(obj2));
    ASSERT_TRUE(objectQueue.Pop(obj1, std::chrono::milliseconds(0)));

    // Make sure popping an item from the no-longer non-empty queue is invalid
    ASSERT_FALSE(objectQueue.Pop(obj1, std::chrono::milliseconds(0)));
}

//==============================================================================
TEST_F(ConcurrencyTest, SingleThreadedTest)
{
    ObjectQueue objectQueue;
    ObjectQueue::size_type size = 0;

    Object obj1(1);
    Object obj2(2);
    Object obj3(3);

    DoQueuePush(objectQueue, obj1, ++size);
    DoQueuePush(objectQueue, obj1, ++size);
    DoQueuePop(objectQueue, obj1, --size);
    DoQueuePush(objectQueue, obj2, ++size);
    DoQueuePush(objectQueue, obj3, ++size);
    DoQueuePop(objectQueue, obj1, --size);
    DoQueuePop(objectQueue, obj2, --size);
    DoQueuePop(objectQueue, obj3, --size);
}

//==============================================================================
TEST_F(ConcurrencyTest, MultiThreadedTest)
{
    RunMultiThreadedTest(1, 1);
    RunMultiThreadedTest(1, 100);
    RunMultiThreadedTest(100, 1);
    RunMultiThreadedTest(100, 100);
}

//==============================================================================
TEST_F(ConcurrencyTest, InfiniteWaitReaderTest)
{
    ObjectQueue objectQueue;
    Object obj(123);

    auto func = std::bind(
        &ConcurrencyTest::InfiniteWaitReaderThread,
        this,
        std::ref(objectQueue));
    std::future<Object> future = std::async(std::launch::async, func);

    std::future_status status = future.wait_for(std::chrono::milliseconds(10));
    ASSERT_EQ(status, std::future_status::timeout);

    objectQueue.Push(Object(obj));

    status = future.wait_for(std::chrono::milliseconds(10));
    ASSERT_EQ(status, std::future_status::ready);
    ASSERT_EQ(future.get(), obj);
}

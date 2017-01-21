#include <atomic>
#include <chrono>
#include <future>
#include <vector>

#include <gtest/gtest.h>

#include <fly/concurrency/concurrent_queue.h>
#include <fly/logger/logger.h>

namespace
{
    typedef int Object;
    typedef fly::ConcurrentQueue<Object> ObjectQueue;

    //==========================================================================
    void DoQueuePush(
        ObjectQueue &objectQueue,
        const Object &object,
        ObjectQueue::size_type expectedSize
    )
    {
        objectQueue.Push(object);

        ASSERT_EQ(objectQueue.Size(), expectedSize);
        ASSERT_FALSE(objectQueue.IsEmpty());
    }

    //==========================================================================
    void DoQueuePop(
        ObjectQueue &objectQueue,
        const Object &expectedObject,
        ObjectQueue::size_type expectedSize
    )
    {
        Object object;

        ASSERT_TRUE(objectQueue.Pop(object, std::chrono::milliseconds(0)));
        ASSERT_EQ(objectQueue.Size(), expectedSize);
        ASSERT_EQ(object, expectedObject);
    }

    //==========================================================================
    unsigned int WriterThread(ObjectQueue &objectQueue)
    {
        unsigned int numWrites = 100;

        for (unsigned int i = 0; i < numWrites; ++i)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

            Object object(i);
            objectQueue.Push(object);
        }

        return numWrites;
    }

    //==========================================================================
    unsigned int ReaderThread(
        ObjectQueue &objectQueue,
        std::atomic_bool &finishedWrites
    )
    {
        unsigned int numReads = 0;

        while (!finishedWrites.load() || !objectQueue.IsEmpty())
        {
            Object object;

            if (objectQueue.Pop(object, std::chrono::seconds(1)))
            {
                ++numReads;
            }
        }

        return numReads;
    }

    //==========================================================================
    void RunMultiThreadedTest(unsigned int numWriters, unsigned int numReaders)
    {
        ObjectQueue objectQueue;

        std::vector<std::future<unsigned int>> writerFutures;
        std::vector<std::future<unsigned int>> readerFutures;

        std::atomic_bool finishedWrites(false);

        // Create numWriters writer threads
        for (unsigned int i = 0; i < numWriters; ++i)
        {
            auto func = std::bind(&WriterThread, std::ref(objectQueue));
            writerFutures.push_back(std::async(std::launch::async, func));
        }

        // Create numReaders reader threads
        for (unsigned int i = 0; i < numReaders; ++i)
        {
            auto func = std::bind(&ReaderThread, std::ref(objectQueue), std::ref(finishedWrites));
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
}

//==============================================================================
TEST(ConcurrencyTest, EmptyQueueUponCreationTest)
{
    ObjectQueue objectQueue;

    ASSERT_TRUE(objectQueue.IsEmpty());
    ASSERT_EQ(objectQueue.Size(), 0);
}

//==============================================================================
TEST(ConcurrencyTest, PopFromEmptyQueueTest)
{
    ObjectQueue objectQueue;

    Object obj1;
    Object obj2(1);

    // Make sure pop is initially invalid
    ASSERT_FALSE(objectQueue.Pop(obj1, std::chrono::milliseconds(0)));

    // Push an item onto the queue and immediately pop it
    objectQueue.Push(obj2);
    ASSERT_TRUE(objectQueue.Pop(obj1, std::chrono::milliseconds(0)));

    // Make sure popping an item from the no-longer non-empty queue is invalid
    ASSERT_FALSE(objectQueue.Pop(obj1, std::chrono::milliseconds(0)));
}

//==============================================================================
TEST(ConcurrencyTest, SingleThreadedTest)
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
TEST(ConcurrencyTest, MultiThreadedTest)
{
    RunMultiThreadedTest(1, 1);
    RunMultiThreadedTest(1, 100);
    RunMultiThreadedTest(100, 1);
    RunMultiThreadedTest(100, 100);
}

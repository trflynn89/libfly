#include "fly/task/runner.h"

#include <functional>
#include <thread>

#include "fly/logger/logger.h"

namespace fly {

//==============================================================================
Runner::Runner(const std::string &name, int numWorkers) :
    m_aKeepRunning(false),
    m_name(name),
    m_numWorkers(numWorkers)
{
}

//==============================================================================
bool Runner::Start()
{
    bool expected = false;

    if (m_aKeepRunning.compare_exchange_strong(expected, true))
    {
        const RunnerPtr spThis = shared_from_this();
        auto function = &Runner::workerThread;

        if (StartRunner())
        {
            LOGI(-1, "%s: Starting %d workers", m_name, m_numWorkers);

            for (int i = 0; i < m_numWorkers; ++i)
            {
                m_futures.push_back(
                    std::async(std::launch::async, function, spThis)
                );
            }
        }
        else
        {
            LOGE(-1, "%s: Could not start running task", m_name);
            m_aKeepRunning.store(false);
        }
    }

    return m_aKeepRunning.load();
}

//==============================================================================
void Runner::Stop()
{
    bool expected = true;

    if (m_aKeepRunning.compare_exchange_strong(expected, false))
    {
        LOGI(-1, "%s: Stopping running task", m_name);
        StopRunner();

        for (auto &future : m_futures)
        {
            if (future.valid())
            {
                future.get();
            }
        }
    }
}

//==============================================================================
void Runner::workerThread()
{
    // TODO health check
    while (m_aKeepRunning.load())
    {
        if (!DoWork())
        {
            break;
        }
    }

    LOGI(-1, "%s: Finished running task", m_name);
}

}

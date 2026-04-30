#pragma once
// =============================================================================
// Platform/MuThreadPool.h
// Lightweight thread pool for parallel game-update work on Android.
//
// Usage:
//   MuThreadPool::Get().ParallelFor(totalCount, [](int start, int end){
//       for (int i = start; i < end; ++i) { ... }
//   });
//
// The calling thread participates as the last chunk (no wasted core).
// Worker threads spin-yield while waiting; call thread spin-yields while
// waiting for workers — avoids OS scheduler overhead for sub-ms jobs.
// =============================================================================

#if defined(__ANDROID__) || defined(MU_IOS)

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <atomic>
#include <algorithm>
#include <cstring>

// Per-thread flag: true inside worker threads.
// Read by effect-creation functions to know when to take the effect mutex.
extern thread_local bool g_muIsWorkerThread;

class MuThreadPool
{
public:
    // Singleton — one pool for the whole game, created on first use.
    static MuThreadPool& Get()
    {
        // Use (hwThreads - 1) workers; the main/GL thread is the N-th worker.
        const unsigned int hw = std::thread::hardware_concurrency();
        const int workers = static_cast<int>(hw > 1u ? hw - 1u : 0u);
        static MuThreadPool inst(workers);
        return inst;
    }

    // ParallelFor — splits [0, count) into (numWorkers+1) chunks.
    // Fn signature: void(int start, int end)
    template<typename Fn>
    void ParallelFor(int count, Fn&& fn)
    {
        if (count <= 0) return;

        const int nWorkers = static_cast<int>(m_workers.size());

        // For tiny counts or no workers, run serially.
        if (nWorkers == 0 || count <= MIN_PARALLEL_COUNT)
        {
            fn(0, count);
            return;
        }

        // Total threads = nWorkers + 1 (main thread handles last chunk).
        const int nChunks = std::min(nWorkers + 1, count);
        const int base    = count / nChunks;
        const int rem     = count % nChunks;   // first `rem` chunks get +1

        std::atomic<int> doneCtr{0};

        // Dispatch worker chunks (0 .. nChunks-2)
        for (int c = 0; c < nChunks - 1; ++c)
        {
            const int start = c * base + std::min(c, rem);
            const int end   = start + base + (c < rem ? 1 : 0);

            Enqueue([start, end, &fn, &doneCtr]()
            {
                g_muIsWorkerThread = true;
                fn(start, end);
                g_muIsWorkerThread = false;
                doneCtr.fetch_add(1, std::memory_order_release);
            });
        }

        // Main thread handles last chunk (overlaps with worker execution).
        {
            const int c     = nChunks - 1;
            const int start = c * base + std::min(c, rem);
            fn(start, count);
        }

        // Spin-yield until all workers are done (typically sub-ms).
        while (doneCtr.load(std::memory_order_acquire) < nChunks - 1)
            std::this_thread::yield();
    }

    int NumWorkers() const { return static_cast<int>(m_workers.size()); }

    ~MuThreadPool()
    {
        {
            std::lock_guard<std::mutex> lk(m_mtx);
            m_quit = true;
        }
        m_cv.notify_all();
        for (auto& t : m_workers) t.join();
    }

private:
    // Minimum item count before going parallel (avoid overhead for tiny loops).
    static constexpr int MIN_PARALLEL_COUNT = 8;

    explicit MuThreadPool(int numWorkers)
    {
        m_workers.reserve(numWorkers);
        for (int i = 0; i < numWorkers; ++i)
            m_workers.emplace_back([this]{ WorkerLoop(); });
    }

    void Enqueue(std::function<void()> job)
    {
        {
            std::lock_guard<std::mutex> lk(m_mtx);
            m_queue.push_back(std::move(job));
        }
        m_cv.notify_one();
    }

    void WorkerLoop()
    {
        while (true)
        {
            std::function<void()> job;
            {
                std::unique_lock<std::mutex> lk(m_mtx);
                m_cv.wait(lk, [this]{ return m_quit || !m_queue.empty(); });
                if (m_quit && m_queue.empty()) return;
                job = std::move(m_queue.front());
                m_queue.erase(m_queue.begin());
            }
            job();
        }
    }

    std::vector<std::thread>          m_workers;
    std::vector<std::function<void()>> m_queue;
    std::mutex                         m_mtx;
    std::condition_variable            m_cv;
    bool                               m_quit = false;
};

#endif // __ANDROID__

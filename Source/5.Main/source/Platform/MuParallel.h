#pragma once
// =============================================================================
// Platform/MuParallel.h
// Shared state for parallel game-update system.
//
// Include this wherever you need to protect global effect/joint/sound creation
// from parallel worker threads.
//
// Pattern:
//   MU_EFFECT_LOCK();          // no-op on main thread, locks on workers
//   CreateEffect(...);
// =============================================================================

#if defined(__ANDROID__) || defined(MU_IOS)

#include <mutex>

// True inside MuThreadPool worker threads (thread_local).
extern thread_local bool g_muIsWorkerThread;

// Mutex protecting global effect / joint / sprite / sound creation arrays.
extern std::mutex g_muEffectMutex;

// Convenience macro — zero cost on main thread (g_muIsWorkerThread == false).
// On worker threads: RAII lock for the duration of the enclosing scope.
#define MU_EFFECT_LOCK() \
    std::unique_lock<std::mutex> _muEfLock(g_muEffectMutex, std::defer_lock); \
    if (g_muIsWorkerThread) _muEfLock.lock()

#else
#define MU_EFFECT_LOCK() ((void)0)
#endif

// =============================================================================
// Platform/MuParallel.cpp
// Definitions for parallel-update shared state.
// =============================================================================

#if defined(__ANDROID__) || defined(MU_IOS)

#include "MuThreadPool.h"
#include <mutex>

// Thread-local: set to true inside MuThreadPool worker threads.
// Read by CreateEffect/CreateJoint/CreateSprite to decide whether to lock.
thread_local bool g_muIsWorkerThread = false;

// Coarse mutex protecting global game-effect arrays when mutated from
// worker threads.  Only contended during the rare frames where characters
// or objects produce visual effects while being updated in parallel.
// In the common case (no effects this frame) it is never acquired.
std::mutex g_muEffectMutex;

#endif // __ANDROID__

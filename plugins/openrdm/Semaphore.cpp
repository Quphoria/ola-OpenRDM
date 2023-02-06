// https://stackoverflow.com/questions/4792449/c0x-has-no-semaphores-how-to-synchronize-threads

#include "plugins/openrdm/Semaphore.h"

namespace ola {
namespace plugin {
namespace openrdm {

using std::mutex;
using std::condition_variable;

template <size_t LeastMax>
counting_semaphore<LeastMax>::counting_semaphore(size_t count)
    : mCount{count}
{}

template <size_t LeastMax>
void counting_semaphore<LeastMax>::notify() {
    std::lock_guard<mutex> lock{mMutex};
    if (mCount < LeastMax) ++mCount;
    mCv.notify_one();
}

template <size_t LeastMax>
void counting_semaphore<LeastMax>::wait() {
    std::unique_lock<mutex> lock{mMutex};
    mCv.wait(lock, [&]{ return mCount > 0; });
    --mCount;
}

template <size_t LeastMax>
bool counting_semaphore<LeastMax>::try_wait() {
    std::lock_guard<mutex> lock{mMutex};

    if (mCount > 0) {
        --mCount;
        return true;
    }

    return false;
}

template <size_t LeastMax>
template<class Rep, class Period>
bool counting_semaphore<LeastMax>::wait_for(const std::chrono::duration<Rep, Period>& d) {
    std::unique_lock<mutex> lock{mMutex};
    auto finished = mCv.wait_for(lock, d, [&]{ return mCount > 0; });

    if (finished)
        --mCount;

    return finished;
}

template <size_t LeastMax>
template<class Clock, class Duration>
bool counting_semaphore<LeastMax>::wait_until(const std::chrono::time_point<Clock, Duration>& t) {
    std::unique_lock<mutex> lock{mMutex};
    auto finished = mCv.wait_until(lock, t, [&]{ return mCount > 0; });

    if (finished)
        --mCount;

    return finished;
}

template <size_t LeastMax>
typename counting_semaphore<LeastMax>::native_handle_type counting_semaphore<LeastMax>::native_handle() {
    return mCv.native_handle();
}

}  // namespace openrdm
}  // namespace plugin
}  // namespace ola

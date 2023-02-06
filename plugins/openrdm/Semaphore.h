// https://stackoverflow.com/questions/4792449/c0x-has-no-semaphores-how-to-synchronize-threads

#ifndef PLUGINS_OPENRDM_SEMAPHONE_H_
#define PLUGINS_OPENRDM_SEMAPHONE_H_

#include <mutex>
#include <condition_variable>

namespace ola {
namespace plugin {
namespace openrdm {

using std::mutex;
using std::condition_variable;

template <size_t LeastMax>
class counting_semaphore {
public:
    using native_handle_type = typename condition_variable::native_handle_type;

    explicit counting_semaphore(size_t count = 0);
    counting_semaphore(const counting_semaphore&) = delete;
    counting_semaphore(counting_semaphore&&) = delete;
    counting_semaphore& operator=(const counting_semaphore&) = delete;
    counting_semaphore& operator=(counting_semaphore&&) = delete;

    void notify();
    void wait();
    bool try_wait();
    template<class Rep, class Period>
    bool wait_for(const std::chrono::duration<Rep, Period>& d);
    template<class Clock, class Duration>
    bool wait_until(const std::chrono::time_point<Clock, Duration>& t);

    native_handle_type native_handle();

private:
    mutex   mMutex;
    condition_variable mCv;
    size_t  mCount;
};

}  // namespace openrdm
}  // namespace plugin
}  // namespace ola

#endif // PLUGINS_OPENRDM_SEMAPHONE_H_
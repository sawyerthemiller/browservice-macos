#pragma once

#include "common.hpp"

namespace browservice {

// Timeout that runs given callback from CEF UI thread event loop after a
// specified (fixed) delay unless canceled
class Timeout : public enable_shared_from_this<Timeout> {
SHARED_ONLY_CLASS(Timeout);
public:
    typedef function<void()> Func;

    Timeout(CKey, int64_t delayMs);

    // Set func to be run in delayMs milliseconds or when cleared with
    // runFunc set to true Calling when timeout is active is error
    void set(Func func);

    // If timeout is active stop it If runFunc is true associated
    // function is called immediately
    void clear(bool runFunc);

    // Returns true if timeout is active ie function has been scheduled
    // to run (and not cleared)
    bool isActive();

private:
    void delayedTask_();

    int64_t delayMs_;

    bool active_;
    Func func_;
    steady_clock::time_point funcTime_;

    bool delayedTaskScheduled_;
    steady_clock::time_point delayedTaskTime_;
};

}

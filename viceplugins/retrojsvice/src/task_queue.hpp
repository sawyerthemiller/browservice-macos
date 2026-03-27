#pragma once

#include "common.hpp"

namespace retrojsvice {

class TaskQueueEventHandler {
public:
    // Exceptionally may be called from any thread at any time to signal that
    // TaskQueue -  - runTasks needs to be called (MCE annotations do not concern
    // this function)
    virtual void onTaskQueueNeedsRunTasks() = 0;

    // Called to signal that shutdown has completed which means that no more
    // tasks may be posted onTaskQueueNeedsRunTasks will not be called anymore
    // and task queue may be destructed
    virtual void onTaskQueueShutdownComplete() = 0;
};

class DelayedTaskTag;

// queue used to defer tasks to be run later in API thread Normally the
// queue is used as follows - 
// Context sets its TaskQueue as active task queue for current thread
// for duration of API function call using ActiveTaskQueueLock This
// means that all tasks posted using postTask and postDelayedTask in that
// thread will be posted to that queue posted tasks will be run in API
// thread when Context -  - pumpEvents invokes runTasks member function
//
// When starting background thread that needs to call postTask or
// postDelayedTask one should call getActiveTaskQueue in API thread copy
// returned shared pointer to started thread and set it as active there
// using ActiveTaskQueueLock
//
// Before destruction task queue must be shut down by calling shutdown and
// waiting for onTaskQueueShutdownComplete event (called by runTasks)
class TaskQueue {
SHARED_ONLY_CLASS(TaskQueue);
public:
    // Creates new TaskQueue that will call newTasksCallback (possibly in a
    // background thread) if runTasks needs to be called
    TaskQueue(CKey, weak_ptr<TaskQueueEventHandler> eventHandler);

    ~TaskQueue();

    void runTasks(MCE);

    // Start shutting down queue shutdown will complete next time
    // queue is completely empty Upon completion the
    // onTaskQueueShutdownComplete event handler will be called All
    // attempts to post new tasks to queue or call runTasks after this will
    // panic
    void shutdown();

    // Returns active task queue for current thread - panics if there is
    // none
    static shared_ptr<TaskQueue> getActiveQueue();

private:
    void afterConstruct_(shared_ptr<TaskQueue> self);

    void needsRunTasks_();

    weak_ptr<TaskQueueEventHandler> eventHandler_;

    mutex mutex_;
    enum {Running, ShutdownPending, ShutdownComplete} state_;
    bool runTasksPending_;
    vector<function<void()>> tasks_;
    multimap<
        steady_clock::time_point,
        pair<weak_ptr<DelayedTaskTag>, function<void()>>
    > delayedTasks_;

    thread delayThread_;
    condition_variable delayThreadCv_;

    bool runningTasks_;

    friend void postTask(function<void()> func);
    friend class DelayedTaskTag;
    friend shared_ptr<DelayedTaskTag> postDelayedTask(
        steady_clock::duration delay,
        function<void()> func
    );
};

// RAII object that sets given task queue as active for current thread - 
// panics if this thread already has active task queue task queue stays
// active until returned object is destructed
class ActiveTaskQueueLock {
public:
    ActiveTaskQueueLock(shared_ptr<TaskQueue> taskQueue);
    ~ActiveTaskQueueLock();

    DISABLE_COPY_MOVE(ActiveTaskQueueLock);
};

void postTask(function<void()> func);

template <typename T, typename... Args>
void postTask(shared_ptr<T> ptr, void (T::*func)(Args...), Args... args) {
    postTask([ptr, func, args...]() {
        (ptr.get()->*func)(args...);
    });
}

template <typename T, typename... Args>
void postTask(weak_ptr<T> weakPtr, void (T::*func)(Args...), Args... args) {
    postTask([weakPtr, func, args...]() {
        if(shared_ptr<T> ptr = weakPtr.lock()) {
            (ptr.get()->*func)(args...);
        }
    });
}

// Object returned by postDelayedTask If object is destructed and delay
// for task has not yet been reached task will be cancelled
class DelayedTaskTag {
SHARED_ONLY_CLASS(DelayedTaskTag);
public:
    // Private constructor
    DelayedTaskTag(CKey, CKey);

    ~DelayedTaskTag();

    // If task is still pending remove it from queue and run it
    // immediately Should only be run from API thread
    void expedite();

private:
    shared_ptr<TaskQueue> taskQueue_;
    bool inQueue_;
    multimap<
        steady_clock::time_point,
        pair<weak_ptr<DelayedTaskTag>, function<void()>>
    >::iterator iter_;

    friend class TaskQueue;
    friend shared_ptr<DelayedTaskTag> postDelayedTask(
        steady_clock::duration delay,
        function<void()> func
    );
};

shared_ptr<DelayedTaskTag> postDelayedTask(
    steady_clock::duration delay,
    function<void()> func
);

template <typename T, typename... Args>
shared_ptr<DelayedTaskTag> postDelayedTask(
    steady_clock::duration delay,
    shared_ptr<T> ptr,
    void (T::*func)(Args...),
    Args... args
) {
    return postDelayedTask(delay, [ptr, func, args...]() {
        (ptr.get()->*func)(args...);
    });
}

template <typename T, typename... Args>
shared_ptr<DelayedTaskTag> postDelayedTask(
    steady_clock::duration delay,
    weak_ptr<T> weakPtr,
    void (T::*func)(Args...),
    Args... args
) {
    return postDelayedTask(delay, [weakPtr, func, args...]() {
        if(shared_ptr<T> ptr = weakPtr.lock()) {
            (ptr.get()->*func)(args...);
        }
    });
}

}

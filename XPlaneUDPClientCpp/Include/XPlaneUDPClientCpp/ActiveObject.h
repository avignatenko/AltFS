#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>

#define PM_MULTITHREAD
#include <promise-cpp/promise.hpp>

class DispatchQueue
{
public:
    using Operation = std::function<void()>;

    void put(Operation op)
    {
        std::lock_guard<std::mutex> guard(m_qlock);
        m_opsQueue.push(op);
        m_empty.notify_one();
    }

    Operation take()
    {
        std::unique_lock<std::mutex> lock(m_qlock);
        m_empty.wait(lock, [&] { return !m_opsQueue.empty(); });

        Operation op = m_opsQueue.front();
        m_opsQueue.pop();
        return op;
    }

    Operation takeNonBlocking()
    {
        std::lock_guard<std::mutex> guard(m_qlock);
        if (m_opsQueue.empty()) return nullptr;

        Operation op = m_opsQueue.front();
        m_opsQueue.pop();
        return op;
    }

private:
    mutable std::mutex m_qlock;
    std::queue<Operation> m_opsQueue;
    std::condition_variable m_empty;
};

class Runner
{
public:
    virtual bool run(std::function<void()> func) = 0;

    thread_local static Runner* threadInstance;
};

class ActiveObject : public Runner
{
public:
    ActiveObject() { m_runnable = std::make_unique<std::thread>(&ActiveObject::messageCycle, this); }

    virtual ~ActiveObject()
    {
        stop();
        m_runnable->join();
    }

    bool run(std::function<void()> func) override
    {
        if (m_done) return false;
        m_dispatchQueue.put(func);
        return true;
    }

    bool isDone() { return m_done; }

private:
    void stop() { run(nullptr); }

    void messageCycle()
    {
        threadInstance = this;

        while (!m_done)
        {
            auto functor = m_dispatchQueue.take();
            if (functor)
                functor();
            else
                m_done = true;
        }

        // dispatch remaing functions
        while (auto functor = m_dispatchQueue.takeNonBlocking()) functor();

        threadInstance = nullptr;
    }

private:
    DispatchQueue m_dispatchQueue;

    std::unique_ptr<std::thread> m_runnable;
    std::function<void(std::exception_ptr)> m_doneCallback;
    std::atomic<bool> m_done = false;
};

/* Create new promise object */
template <typename FUNC>
inline promise::Defer newPromiseAsync(Runner* runner, FUNC func)
{
    promise::Defer promise = promise::newPromise();
    promise->run([=](promise::Defer& d) { runner->run([=, caller = Runner::threadInstance] { func(caller, d); }); },
                 promise);

    return promise;
}

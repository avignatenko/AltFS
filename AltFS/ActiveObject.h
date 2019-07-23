#pragma once

#include <mutex>
#include <queue>
#include <atomic>
#include <memory>

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

private:

    std::mutex m_qlock;
    std::queue<Operation> m_opsQueue;
    std::condition_variable m_empty;

};

class ActiveObject
{
public:
    ActiveObject()
    {
        m_runnable = std::make_unique<std::thread>(&ActiveObject::run, this);
    }

    virtual ~ActiveObject()
    {
        stop();
        m_runnable->join();
    }
   
private:

     void stop()
    {
        m_dispatchQueue.put(nullptr);
    }

    void run()
    {
        bool done = false;
        while (!done)
        {
            auto functor = m_dispatchQueue.take();
            if (functor)
                functor();
            else
                done = true;
        }
    }

protected:

    DispatchQueue m_dispatchQueue;

private:

    std::unique_ptr<std::thread> m_runnable;
};
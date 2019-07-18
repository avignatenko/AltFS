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
        std::lock_guard<std::mutex> guard(qlock);
        ops_queue.push(op);
        empty.notify_one();
    }

    Operation take()
    {
        std::unique_lock<std::mutex> lock(qlock);
        empty.wait(lock, [&] { return !ops_queue.empty(); });

        Operation op = ops_queue.front();
        ops_queue.pop();
        return op;
    }

private:

    std::mutex qlock;
    std::queue<Operation> ops_queue;
    std::condition_variable empty;

};

/// http://netaz.blogspot.com/2015/01/revisiting-active-object-pattern-with.html
class ActiveObject
{
public:
    ActiveObject()
    {
        runnable = std::make_unique<std::thread>(&ActiveObject::run, this);
    }

    virtual ~ActiveObject()
    {
        stop();
        runnable->join();
    }
   
private:

     void stop()
    {
        dispatchQueue.put(nullptr);
    }

    void run()
    {
        bool done = false;
        while (!done)
        {
            auto functor = dispatchQueue.take();
            if (functor)
                functor();
            else
                done = true;
        }
    }

protected:

    DispatchQueue dispatchQueue;

private:

    std::unique_ptr<std::thread> runnable;
};
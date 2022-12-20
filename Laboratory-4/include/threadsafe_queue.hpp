#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

template<typename T>
class threadsafe_queue
{
  private:
      mutable std::mutex _mutex;
      std::condition_variable _cv;
      std::queue<T> _queue;

  public:
    threadsafe_queue() {}

    threadsafe_queue(const threadsafe_queue& other)
    {
        std::lock_guard<std::mutex> lock(other._mutex);

        _queue = std::move(other._queue);
    }

    threadsafe_queue& operator=(const threadsafe_queue&) = delete;

    void push(T new_value)
    {
        {
            std::lock_guard<std::mutex> lock(_mutex);

            _queue.push(new_value);
        }

        _cv.notify_all();
    }

    bool try_pop(T& value)
    {
        std::lock_guard<std::mutex> lock(_mutex);

        bool cond = !_queue.empty();
        if(cond){
            value = _queue.front();
            _queue.pop();
        }
        
        return cond;
    }   

    void wait_and_pop(T& value)
    {
        std::unique_lock<std::mutex> lock(_mutex);

        _cv.wait(lock, [this]{ return !_queue.empty(); });

        value = _queue.front();
        _queue.pop();

        lock.unlock();
    }

    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _cv.wait(lock, [this]{ return !_queue.empty(); });

        std::shared_ptr<T> value(std::make_shared<T>(_queue.front()));
        _queue.pop();

        lock.unlock();

        return value;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(_mutex);

        return _queue.empty();
    }

    void print(bool count){
        if(count)
            std::cout << this->_queue.size() << std::endl;
    }
};

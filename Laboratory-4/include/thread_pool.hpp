#pragma once

#include <atomic>
#include <functional>
#include <vector>

#include<join_threads.hpp>
#include<threadsafe_queue.hpp>

class thread_pool
{
  private:

  using task_type = void();
  
  std::atomic<bool> _done;
  size_t _thread_count;
  std::vector<std::thread> _threads;

  join_threads _joiner;

  void worker_thread()
  {
    while(!_done){
      std::function<task_type> task;

      if(_work_queue.try_pop(task)){
        task();
      }else{
        std::this_thread::yield();
      }

      //_work_queue.wait_and_pop(task);
    }
  }

  public:
  thread_pool(size_t num_threads = std::thread::hardware_concurrency())
    : _done(false), _thread_count(num_threads), _joiner(_threads)
  {
    for(size_t i = 0; i < _thread_count; ++i){
      _threads.push_back(std::thread(&thread_pool::worker_thread, this));
    }

  }

  ~thread_pool()
  {
    wait();
  }

  void wait()
  {   
      // wait for completion
      while(!_done){
        _done = _work_queue.empty();
      }
      // active waiting
  }

  template<typename F>
    void submit(F f)
    {
      _work_queue.push(std::function<task_type>(f));
    }
  threadsafe_queue<std::function<task_type>> _work_queue;
};

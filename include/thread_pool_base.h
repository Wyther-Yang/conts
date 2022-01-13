#ifndef GUID_C542F663_7C6A_4ACA_9C76_0E0424A17CE0_YANG
#define GUID_C542F663_7C6A_4ACA_9C76_0E0424A17CE0_YANG

#include "pre_config.h"
#include "con_utility.h"
#include "sync_queue.h"
#include "thread_handler.h"

namespace concurrent {
namespace pool {

// normal thread pool with single task queue
template<typename _Q, typename _Callable>
class base_thread_pool
{
  using main_queue_t = _Q;
  using threads_t = vector<thread>;
  using task_t = base_work_t;

  main_queue_t _M_q;
  threads_t _M_threads;
  joiner<threads_t> waiting_join;

  void worker_thread()
  {
    try {
      while (!_M_q.closed()) {
        task_t task;
        if (_M_q.pull(task) == queue::closed) {
          _Callable()();
          return;
        }
        task();
      }
    } catch (...) {
      std::terminate();
    }
  }

public:
  base_thread_pool(unsigned __i = std::thread::hardware_concurrency())
    : _M_threads(__i)
    , waiting_join(_M_threads)
  {
    try {
      for (unsigned n{}; n < __i; ++n) {
        _M_threads[n] = thread(&base_thread_pool::worker_thread, this);
      }
    } catch (...) {
      _M_q.close();
      throw;
    }
  }

  ~base_thread_pool() { _M_q.close(); }

  __NO_COPYABLE(base_thread_pool);

  template<typename _F>
  void submit(_F __f)
  {
    _M_q.push(task_t(move(__f)));
  }

  bool try_executing_one()
  {
    task_t task;
    if (_M_q.try_pull(task)) {
      task();
      return true;
    }
    return false;
  }
};

// the thread-pool fully control child thread in every details
template<typename _Q, typename _Callable>
class base_thread_pool<_Q, wrapper_thread<_Callable>>
{
  using __threads_t = vector<wrapper_thread<_Callable>>;
  using __thread_t = wrapper_thread<_Callable>;
  using __task_t = base_work_t;
  using __main_queue_t = _Q;
  using __queues_t = vector<std::unique_ptr<_Q>>;
  using __queue_t = normal_queue<base_work_t>;

  static thread_local int _M_idx;
  static thread_local __main_queue_t* _M_local_q_ptr;
  __threads_t _M_ths;
  __queues_t _M_qs;
  __queue_t _M_q;
  joiner<__threads_t> _M_joiner;

  bool from_local_tasks(__task_t& __task)
  {
    return _M_local_q_ptr && _M_local_q_ptr->try_pull(__task);
  }

  bool from_main_tasks(__task_t& __task) { return _M_q.try_pull(__task); }

  bool steal_from_other_tasks(__task_t& __task)
  {
    for (unsigned __off{}; __off < _M_qs.size(); ++__off) {
      unsigned const __curr_idx = (_M_idx + __off + 1) % _M_qs.size();
      if (_M_qs[__curr_idx]->try_pull(__task))
        return true;
    }
    return false;
  }

  void worker_thread(unsigned __i)
  {
    _M_idx = __i;
    _M_local_q_ptr = _M_qs[__i].get();
    for (;;) {
      __INTERRUPT_POINT;
      try_executing_one();
    }
  }

public:
  __NO_COPYABLE(base_thread_pool);

  __ACCEPTION_RVALUE_DEFAULT(base_thread_pool);

  base_thread_pool(unsigned __count = thread::hardware_concurrency())
    : _M_joiner(_M_ths)
  {
    for (unsigned __i{}; __i < __count; ++__i) {
      _M_qs.push_back(std::unique_ptr<_Q>(new _Q));
    }

    for (unsigned __i{}; __i < __count; ++__i) {
      _M_ths.push_back([this, __i] {
        _M_idx = __i;
        _M_local_q_ptr = _M_qs[__i].get();
        for (;;) {
          __INTERRUPT_POINT;
          try_executing_one();
        }
      });
    }
  }

  ~base_thread_pool()
  {
    for (auto& __th : _M_ths)
      __th.interrupt();
  }

  void try_executing_one()
  {
    __task_t __task;
    if (from_local_tasks(__task) || from_main_tasks(__task) ||
        steal_from_other_tasks(__task)) {
      __task();
    } else {
      __ROTATION;
    }
  }

  template<typename _F>
  void submit(_F&& __f)
  {
    if (_M_local_q_ptr) {
      _M_local_q_ptr->push(__task_t(std::forward<_F>(__f)));
    } else {
      _M_q.try_push(__task_t(std::forward<_F>(__f)));
    }
  }
};

template<typename _Q, typename _Callable>
thread_local int base_thread_pool<_Q, wrapper_thread<_Callable>>::_M_idx{};

template<typename _Q, typename _Callable>
thread_local _Q*
  base_thread_pool<_Q, wrapper_thread<_Callable>>::_M_local_q_ptr{};

} // namespace pool
} // namespace concurrent

#endif /* GUID_C542F663_7C6A_4ACA_9C76_0E0424A17CE0_YANG */

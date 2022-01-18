/*
 * the wrapped template class provides more features with std::thread class.
 */

#ifndef GUID_A458B22B_D88A_4BFE_847C_7690AB6CC215_YANG
#define GUID_A458B22B_D88A_4BFE_847C_7690AB6CC215_YANG

#include "pre_config.h"

namespace concurrent {
namespace threads {

// exception when interruption happening
struct thread_interrupted : std::exception
{};

// default processing func in intr_thread
struct _empty_callable
{
  __ALWAYS_INLINE
  void operator()() noexcept {}
};

// forward dec.
void
interrupt_point();

//
class interrupt_flag
{
  // a thread executes a task at the same time, so false sharing is almost never
  // happened.
  // __ALIGNAS_WITH_GCC
  atomic_bool interrupted;
  // __ALIGNAS_WITH_GCC
  mutex condi_mutex;
  condition_variable* condi;
  std::condition_variable_any* condi_any;

  void set_condition_variable(condition_variable& __condi)
  {
    lock_guard<mutex> lk(condi_mutex);
    condi = &__condi;
  }

  void clear_condition_variable()
  {
    lock_guard<mutex> lk(condi_mutex);
    condi = nullptr;
  }

public:
  __NO_COPYABLE(interrupt_flag);

  __ACCEPTION_RVALUE_DEFAULT(interrupt_flag);

  interrupt_flag()
    : interrupted(false)
    , condi(nullptr)
    , condi_any(nullptr)
  {}

  void set()
  {
    interrupted.store(true, std::memory_order_relaxed);
    lock_guard<mutex> lk(condi_mutex);
    if (condi)
      condi->notify_all();
    if (condi_any)
      condi_any->notify_all();
  }

  struct set_guard
  {
    interrupt_flag* self;
    ~set_guard() { self->clear_condition_variable(); }
  };

  bool is_set() const { return interrupted.load(std::memory_order_relaxed); }

  /// for future
  /* dangerous!!!!!, MUST recover some core source if want to interrupt
   * future-waiting */
  template<typename _T>
  void wait(future<_T>& __f)
  {
    while (!is_set()) {
      if (__f.wait_for(std::chrono::milliseconds(10)) ==
          std::future_status::ready)
        break;
    }
    interrupt_point();
  }

  /// for condition
  template<typename _Predicate>
  void wait(condition_variable& __condi,
            unique_lock<mutex>& lk,
            _Predicate pred)
  {

    interrupt_point();
    set_condition_variable(__condi);
    set_guard guard(this);
    while (!is_set() && !pred())
      __condi.wait_for(lk, std::chrono::milliseconds(10));
    interrupt_point();
  }

  /// for condition_any
  template<typename _Predicate, typename _Lock>
  void wait(std::condition_variable_any& __condi_any,
            _Lock& lk,
            _Predicate pred)
  {
    struct merged_lock
    {
      interrupt_flag* self;
      _Lock& lk;

      merged_lock(interrupt_flag* __self,
                  _Lock& __lk,
                  std::condition_variable_any& __condi)
        : self(__self)
        , lk(__lk)
      {
        self->condi_mutex.lock();
        self->condi_any = &__condi;
      }

      void unlock()
      {
        lk.unlock();
        self->condi_mutex.unlock();
      }

      void lock() { std::lock(self->condi_mutex, lk); }

      ~merged_lock()
      {
        self->condi_any = nullptr;
        self->condi_mutex.unlock();
      }
    };

    merged_lock mlk(this, lk, __condi_any);
    interrupt_point();
    __condi_any.wait(mlk, move(pred));
    interrupt_point();
  }
};

// local thread handler used by main or distributing thread
inline thread_local interrupt_flag current_thread_interrupt_flag;

//
void
interrupt_point()
{
  if (current_thread_interrupt_flag.is_set())
    __THROW_EXCEPTION(thread_interrupted());
}

// with condi, condi_any, and future
template<typename... _Args>
void
interrupt_wait(_Args&&... __args)
{
  current_thread_interrupt_flag.wait(std::forward<_Args>(__args)...);
}

// wrapper class with std::thread for interrupting style
template<typename callable = _empty_callable>
class intr_thread : public std::thread
{
  typedef std::thread base;

  interrupt_flag* flag;

public:
  __DEFAULT_CTOR(intr_thread);

  __NO_COPYABLE(intr_thread);

  __ACCEPTION_RVALUE_DEFAULT(intr_thread);

  intr_thread(std::thread&& _t)
    : base(move(_t))
    , flag(nullptr)
  {}

  template<typename T,
           typename = std::enable_if_t<std::is_invocable_v<T> &&
                                       std::is_invocable_v<callable>>>
  intr_thread(T&& _task)
  {
    promise<interrupt_flag*> _p;
    future<interrupt_flag*> _f(_p.get_future());
    __BASE_REF(this) =
      base([p = move(_p), task = std::forward<T>(_task)]() mutable {
        try {
          p.set_value(&current_thread_interrupt_flag);
          task();
        } catch (thread_interrupted const&) {
          callable()();
        } catch (...) {
          p.set_exception(std::current_exception());
        }
      });
    flag = _f.get();
  }

  inline void interrupt()
  {
    if (flag)
      flag->set();
  }
};

} // namespace threads

template<typename _Callable>
using wrapper_thread = threads::intr_thread<_Callable>;

// typedef wrapper_thread<threads::_empty_callable> default_thread_type;

} // namespace concurrent

#endif /* GUID_A458B22B_D88A_4BFE_847C_7690AB6CC215_YANG */

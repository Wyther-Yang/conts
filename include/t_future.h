/* with boost::future::then, i got some problems that its very weak in practical use. 
 * bcoz, in huge task list, coroutines is better. 
 * in real-time and small task list, its too heavy and slow.
 * so, i want to provide a new way with std::future, not boost::future;
 *
 * ////////////////////////////////
 * eg:
 *      // if you want to put a new task list, you can do like:
 *      std::future f = t_async([]{
 * 
 *      // statement
 * 
 *        // whatever which type
 *        return typeA;
 *      }).then([](typeA ){
 * 
 *      // statment
 *        
 *        // whatever which type
 *        return typeB
 *      }).then([](typeB ){
 *      
 *      });
 * 
 *      // ..................
 * 
 *      //sometime do
 *      f.get();
 * ///////////////////////////////////
 * 
 * interestingly, intermediate types are completely transparent to users, ohhhhh  its perfect! ^.^!.
 *
 */

#ifndef __T_FUTURE_H__YANG
#define __T_FUTURE_H__YANG

#include <chrono>
#include <future>
#include <thread>
#include <utility>
#include <vector>

using std::async;
using std::decay_t;
using std::declval;
using std::future;
using std::move;
using std::promise;
using std::thread;

template<typename T>
class t_future : public future<T>
{

  template<typename F>
  auto spwan_async(F&& func);

  template<typename F>
  auto spwan_async_void(F&& func);

public:
  template<typename F>
  auto then(F&& f)
  {
    if constexpr (std::is_same_v<T, void>)
      return spwan_async_void(std::forward<F>(f));
    else
      return spwan_async(std::forward<F>(f));
  }
};

template<typename T>
template<typename F>
auto t_future<T>::spwan_async(F&& func)
{
  using rType = decltype(declval<decay_t<F>>()(declval<T>()));

  promise<rType> p;
  auto f = p.get_future();

  thread([_p = move(p), _func = move(func), _f = move(*this)]() mutable {
    try {
      while (_f.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
        std::this_thread::yield();

      if constexpr (std::is_same_v<rType, void>) {
        _func(_f.get());
        _p.set_value_at_thread_exit();
      } else
        _p.set_value_at_thread_exit(_func(_f.get()));

    } catch (...) {
      _p.set_exception_at_thread_exit(std::current_exception());
    }
  }).detach();

  return t_future<rType>{ move(f) };
}

template<typename T>
template<typename F>
auto t_future<T>::spwan_async_void(F&& func)
{
  using rType = decltype(declval<decay_t<F>>()());

  promise<rType> p;
  auto f = p.get_future();

  thread([_p = move(p), _func = move(func), _f = move(*this)]() mutable {
    try {
      while (_f.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
        std::this_thread::yield();

      if constexpr (std::is_same_v<rType, void>) {
        _func();
        _p.set_value_at_thread_exit();
      } else
        _p.set_value_at_thread_exit(_func());

    } catch (...) {
      _p.set_exception_at_thread_exit(std::current_exception());
    }
  }).detach();

  return t_future<rType>{ move(f) };
}

template<typename T>
auto t_async(T&& f)
{
  return static_cast<t_future<void>>(async(std::forward<T>(f)));
}

#endif
#ifndef GUID_B8E118BC_E23F_477C_8134_731D66426C4B_YANG
#define GUID_B8E118BC_E23F_477C_8134_731D66426C4B_YANG

#include "pre_config.h"
#include "queue_status.h"

namespace concurrent {
namespace queue {

template<typename T, queue_type types>
class sync_queue_base;

// normal
template<typename T>
class sync_queue_base<T, normal>
{
protected:
  struct node
  {
    shared_ptr<T> data;
    unique_ptr<node> next;
  };

  unique_ptr<node> head;
  node* tail;
  condition_variable condi;
  mutex head_mutex;
  mutex tail_mutex;
  atomic_bool closed_;

public:
  __NO_COPYABLE(sync_queue_base);

  sync_queue_base()
    : head(new node)
    , tail(head.get())
    , closed_(false)
  {}

  inline node* get_tail()
  {
    lock_guard<mutex> lk(tail_mutex);
    return tail;
  }

  inline bool empty() { return head.get() == get_tail(); }

  inline bool empty(unique_lock<mutex>& lk) { return head.get() == get_tail(); }

  inline bool empty(lock_guard<mutex>& lk) { return head.get() == get_tail(); }

  inline void throw_if_closed() { __THROW_EXCEPTION(sync_queue_closed()); }

  inline bool not_empty_or_closed()
  {
    return !empty() || closed_.load(m_relaxed);
  }

  inline bool wait_until_not_empty_or_closed(unique_lock<mutex>& lk)
  {
    condi.wait(lk, [this] { return not_empty_or_closed(); });
    if (closed_.load(m_relaxed))
      return false;
    return true;
  }

  inline void pop(unique_lock<mutex>& lk, T& val)
  {
    auto temp = move(head->next);
    val = move(*(head->data));
    head = move(temp);
  }

  inline void pop(lock_guard<mutex>& lk, T& val)
  {
    auto temp = move(head->next);
    val = move(*(head->data));
    head = move(temp);
  }

  inline auto pop(unique_lock<mutex>& lk)
  {
    auto temp = move(head->next);
    auto data = move(head->data);
    head = move(temp);
    return data;
  }

  inline auto pop(lock_guard<mutex>& lk)
  {
    auto temp = move(head->next);
    auto data = move(head->data);
    head = move(temp);
    return data;
  }

  inline void close() { closed_.store(true, m_relaxed); }

  inline bool closed() { return closed_.load(m_relaxed); }

  inline bool closed(unique_lock<mutex>& lk) { return closed_.load(m_relaxed); }

  inline void notify_elem_added() { condi.notify_one(); }

  inline void notify_all_elems_added() { condi.notify_all(); }
};

// steal
template<typename T>
class sync_queue_base<T, steal>
{
protected:
  mutex _mutex;
  deque<T> _data;

  typedef deque<T> container_type;

public:
  __DEFAULT_CTOR(sync_queue_base);

  __NO_COPYABLE(sync_queue_base);

  inline void push(lock_guard<mutex>& lk, T& val)
  {
    _data.push_front(move(val));
  }

  inline bool empty(lock_guard<mutex>& lk) const { return _data.empty(); }

  inline bool empty(unique_lock<mutex>& lk) const { return _data.empty(); }

  inline bool try_pull(lock_guard<mutex>& lk, T& val)
  {
    if (empty(lk))
      return false;
    val = move(_data.front());
    _data.pop_front();
    return true;
  }

  inline bool try_pull(unique_lock<mutex>& lk, T& val)
  {
    if (empty(lk))
      return false;
    val = move(_data.front());
    _data.pop_front();
    return true;
  }

  inline bool try_steal(lock_guard<mutex>& lk, T& val)
  {
    if (empty(lk))
      return false;
    val = move(_data.back());
    _data.pop_back();
    return true;
  }

  inline bool try_steal(unique_lock<mutex>& lk, T& val)
  {
    if (empty(lk))
      return false;
    val = move(_data.back());
    _data.pop_back();
    return true;
  }
};

} // namepsace queue
} // namespace concurrent

#endif /* GUID_B8E118BC_E23F_477C_8134_731D66426C4B_YANG */

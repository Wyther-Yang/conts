#ifndef GUID_B8E118BC_E23F_477C_8134_731D66426C4B_YANG
#define GUID_B8E118BC_E23F_477C_8134_731D66426C4B_YANG

#include "pre_config.h"
#include "queue_status.h"

namespace concurrent {
namespace queue {

template<typename T, queue_type types>
class sync_queue_base;

// high-load
template<typename _T>
struct _sync_hl_queue_base
{

  static constexpr auto ac = std::memory_order::acquire;
  static constexpr auto re = std::memory_order::release;
  static constexpr auto rx = std::memory_order::relaxed;

  struct node;

  struct counted_ptr
  {
    // node ptr
    node* n_ptr;
    // single count
    int s_count;
  };

  struct counter
  {
    unsigned internal_count : 30 {};
    unsigned external_count : 2 { 2 };
  };

  struct node
  {
    atomic<_T*> data;
    atomic<counter> all_count;
    atomic<counted_ptr> next;
  };

  atomic<counted_ptr> head;
  atomic<counted_ptr> tail;

  _sync_hl_queue_base()
  {
    node* temp = new node{};
    head.store({ temp, 0 });
    tail.store({ temp, 0 });
  }
  _sync_hl_queue_base(_sync_hl_queue_base&) = delete;
  _sync_hl_queue_base(_sync_hl_queue_base&&) = delete;
  _sync_hl_queue_base& operator=(_sync_hl_queue_base&) = delete;
  _sync_hl_queue_base& operator=(_sync_hl_queue_base&&) = delete;
  ~_sync_hl_queue_base() = default;

  inline void increase_count(atomic<counted_ptr>& __A, counted_ptr& __old)
  {
    counted_ptr __new;
    do {
      __new = __old;
      ++__new.s_count;
    } while (!__A.compare_exchange_strong(__old, __new, rx));
    __old = __new;
  }

  void normal_release_ref(node* __n_ptr)
  {
    counter __curr = __n_ptr->all_count.load(rx);
    counter __new;
    do {
      __new = __curr;
      --__new.internal_count;
    } while (!__n_ptr->all_count.compare_exchange_strong(__curr, __new, rx));
    if (!__new.internal_count && !__new.external_count) {
      delete __n_ptr;
    }
  }

  void ends_release_ref(counted_ptr& __old)
  {
    node* const __n_ptr = __old.n_ptr;
    int const num = __old.s_count - 1;
    counter __curr = __n_ptr->all_count.load(rx);
    counter __new;
    do {
      __new = __curr;
      __new.internal_count += num;
      --__new.external_count;
    } while (__n_ptr->all_count.compare_exchange_strong(__curr, __new, rx));
    if (!__new.internal_count && !__new.external_count) {
      delete __n_ptr;
    }
  }

  void set_new_tail(counted_ptr& __old, counted_ptr const& __new)
  {
    node* const __n_ptr = __old.n_ptr;
    while (!tail.compare_exchange_strong(__old, __new, rx) &&
           __n_ptr == __old.n_ptr)
      ;
    if (__n_ptr == __old.n_ptr) {
      ends_release_ref(__old);
    } else {
      normal_release_ref(__n_ptr);
    }
  }
};

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

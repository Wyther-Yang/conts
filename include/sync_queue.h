#ifndef GUID_E13A387C_A3D4_43DD_A19B_DF85CDABC72E_YANG
#define GUID_E13A387C_A3D4_43DD_A19B_DF85CDABC72E_YANG

#include "sync_queue_base.h"

namespace concurrent {
namespace queue {

template<typename T, queue_type types>
class sync_queue;

// a normal queue used at thread pool
template<typename T>
class sync_queue<T, normal> : public sync_queue_base<T, normal>
{
  typedef sync_queue_base<T, normal> base;
  typedef typename base::node node;

public:
  __DEFAULT_CTOR(sync_queue);

  __NO_COPYABLE(sync_queue);

  inline bool empty()
  {
    lock_guard<mutex> lk(base::head_mutex);
    return base::empty(lk);
  }

  inline void close()
  {
    base::close();
    base::notify_all_elems_added();
  }

  inline bool closed() { return base::closed(); }

  queue_op_status push(T val)
  {
    if (base::closed())
      base::throw_if_closed();
    shared_ptr<T> temp(make_shared<T>(move(val)));
    unique_ptr<node> new_next(new node);
    node* new_tail = new_next.get();
    {
      lock_guard<mutex> lk(base::tail_mutex);
      base::tail->data = move(temp);
      base::tail->next = move(new_next);
      base::tail = new_tail;
    }
    base::notify_elem_added();
    return queue::success;
  }

  queue_op_status pull(T& ref_val)
  {
    unique_lock<mutex> lk(base::head_mutex);
    if (!base::wait_until_not_empty_or_closed(lk))
      return queue::closed;
    base::pop(lk, ref_val);
    return queue::success;
  }

  auto pull()
  {
    unique_lock<mutex> lk(base::head_mutex);
    if (!base::wait_until_not_empty_or_closed(lk))
      return std::make_shared<T>();
    return base::pop(lk);
  }

  bool try_pull(T& ref_val)
  {
    lock_guard<mutex> lk(base::head_mutex);
    /*
    if (base::closed())
      return queue::closed;
    */
    if (base::empty(lk))
      return false;
    base::pop(lk, ref_val);
    return true;
  }

  void try_push(T val)
  {
    shared_ptr<T> temp(make_shared<T>(move(val)));
    unique_ptr<node> new_next(new node);
    node* new_tail = new_next.get();
    {
      lock_guard<mutex> lk(base::tail_mutex);
      base::tail->data = move(temp);
      base::tail->next = move(new_next);
      base::tail = new_tail;
    }
    base::notify_elem_added();
  }

  void try_push(shared_ptr<T> sptr)
  {
    unique_ptr<node> new_next(new node);
    node* new_tail = new_next.get();
    {
      lock_guard<mutex> lk(base::tail_mutex);
      base::tail->data = move(sptr);
      base::tail->next = move(new_next);
      base::tail = new_tail;
    }
    base::notify_elem_added();
  }
};

// elements in the queue can be stolen,
// and the queue only belonged to the thread of thread pool one by one
template<typename T>
class sync_queue<T, steal> : public sync_queue_base<T, steal>
{
  typedef sync_queue_base<T, steal> base;

public:
  typedef typename base::container_type container_type;

  __DEFAULT_CTOR(sync_queue);

  __NO_COPYABLE(sync_queue);

  inline bool empty()
  {
    lock_guard<mutex> lk(base::_mutex);
    return base::empty(lk);
  }

  inline void push(T val)
  {
    lock_guard<mutex> lk(base::_mutex);
    base::push(lk, val);
  }

  inline bool try_pull(T& val)
  {
    lock_guard<mutex> lk(base::_mutex);
    return base::try_pull(lk, val);
  }

  inline queue_op_status try_steal(T& val)
  {
    lock_guard<mutex> lk(base::_mutex);
    return base::try_steal(lk, val);
  }
};

} // namespace queue

template<typename T>
using normal_queue = queue::sync_queue<T, queue::normal>;

template<typename T>
using steal_queue = queue::sync_queue<T, queue::steal>;

} // namespace concurrent

#endif /* GUID_E13A387C_A3D4_43DD_A19B_DF85CDABC72E_YANG */

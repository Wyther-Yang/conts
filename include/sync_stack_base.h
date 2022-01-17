#ifndef GUID_FACE677D_151A_4BD8_A5C5_A90C78A67B0D_YANG
#define GUID_FACE677D_151A_4BD8_A5C5_A90C78A67B0D_YANG

#include "pre_config.h"

namespace concurrent {
namespace stack {

enum class stack_type
{
  low_load = 0,

  high_load
};

// for low load
template<typename _T>
struct _sync_ll_stack_base
{
  struct node
  {
    shared_ptr<_T> _M_data;
    node* _M_next;
  };

  __ALIGNAS_WITH_GCC
  std::atomic<node*> _M_head;

  __ALIGNAS_WITH_GCC
  std::atomic<int> _M_ths_count;

  __ALIGNAS_WITH_GCC
  std::atomic<node*> _M_ready_ls;

  inline auto compare_ex_with_acq_rlx(std::atomic<node*>& __A,
                                      node* __source,
                                      node* __candidate)
  {
    return __A.compare_exchange_weak(__source,
                                     __candidate,
                                     std::memory_order_acquire,
                                     std::memory_order_relaxed);
  }

  inline auto compare_ex_with_rel_rlx(std::atomic<node*>& __A,
                                      node* __source,
                                      node* __candidate)
  {
    return __A.compare_exchange_weak(__source,
                                     __candidate,
                                     std::memory_order_release,
                                     std::memory_order_relaxed);
  }

  inline auto load_with_rlx(std::atomic<node*>& __A)
  {
    return __A.load(std::memory_order_relaxed);
  }

  inline void as_fence(std::atomic<node*>& __A)
  {
    __A.load(std::memory_order_acquire);
  }

  inline auto add_with_rlx(int __i)
  {
    return _M_ths_count.fetch_add(__i, std::memory_order_relaxed);
  }

  inline auto sub_with_rlx(int __i)
  {
    return _M_ths_count.fetch_sub(__i, std::memory_order_relaxed);
  }

  void pending_nodes(node* __first, node* __last)
  {
    __last->_M_next = load_with_rlx(_M_ready_ls);
    while (!compare_ex_with_rel_rlx(_M_ready_ls, __last->_M_next, __first))
      ;
  }

  void pending_nodes(node* __n)
  {
    node* last = __n;
    while (node* next = last->_M_next) {
      last = next;
    }
    pending_nodes(__n, last);
  }

  void pending_node(node* __n) { pending_nodes(__n, __n); }

  void delete_nodes(node* __n)
  {
    as_fence(_M_ready_ls);
    while (__n) {
      node* temp = __n->_M_next;
      delete __n;
      __n = temp;
    }
  }

  void try_reclaim(node* __n)
  {
    if (add_with_rlx(0) == 1) {
      delete __n;
      node* temp = _M_ready_ls.exchange(nullptr, std::memory_order_relaxed);
      if (sub_with_rlx(1) == 1) {
        delete_nodes(temp);
      } else {
        pending_nodes(temp);
      }
    } else {
      pending_node(__n);
      sub_with_rlx(1);
    }
  }
};

// for high load
template<typename _T>
struct _sync_hl_stack_base
{

  struct _cptr;

  struct node
  {
    shared_ptr<_T> _M_data;
    _cptr _M_next;
    //__ALIGNAS_WITH_GCC
    std::atomic<int> _M_count;

    __DEFAULT_CTOR(node);

    node(_T __val, _cptr __next)
      : _M_data(make_shared<_T>(move(__val)))
      , _M_next(__next)
      , _M_count(0)
    {}
  };

  struct _cptr
  {
    int _M_count;
    node* _M_ptr;
  };

  std::atomic<_cptr> _M_head;

  inline auto compare_ex_with_acq_rlx(std::atomic<_cptr>& __A,
                                      _cptr& __source,
                                      _cptr& __candidate)
  {
    return __A.compare_exchange_weak(__source,
                                     __candidate,
                                     std::memory_order_acquire,
                                     std::memory_order_relaxed);
  }

  inline auto compare_ex_with_rel_rlx(std::atomic<_cptr>& __A,
                                      _cptr& __source,
                                      _cptr& __candidate)
  {
    return __A.compare_exchange_weak(__source,
                                     __candidate,
                                     std::memory_order_release,
                                     std::memory_order_relaxed);
  }

  inline auto compare_ex_strong_with_rlx(std::atomic<_cptr>& __A,
                                         _cptr& __source,
                                         _cptr& __candidate)
  {
    return __A.compare_exchange_strong(
      __source, __candidate, std::memory_order_relaxed);
  }

  inline auto compare_ex_strong_with_acq_rlx(std::atomic<_cptr>& __A,
                                             _cptr& __source,
                                             _cptr& __candidate)
  {
    return __A.compare_exchange_strong(__source,
                                       __candidate,
                                       std::memory_order_acquire,
                                       std::memory_order_relaxed);
  }

  inline void as_fence(std::atomic<int>& __A)
  {
    __A.load(std::memory_order_acquire);
  }

  inline auto load_with_rlx(std::atomic<_cptr>& __A)
  {
    return __A.load(std::memory_order_relaxed);
  }

  inline auto add_with_rlx(std::atomic<int>& __A, int __n)
  {
    return __A.fetch_add(__n, std::memory_order_relaxed);
  }

  inline auto add_with_rel(std::atomic<int>& __A, int __n)
  {
    return __A.fetch_add(__n, std::memory_order_release);
  }

  inline auto sub_with_rlx(std::atomic<int>& __A, int __n)
  {
    return __A.fetch_sub(__n, std::memory_order_relaxed);
  }

  inline auto sub_with_rel(std::atomic<int>& __A, int __n)
  {
    return __A.fetch_sub(__n, std::memory_order_release);
  }

  inline void update_head_count(_cptr& __old)
  {
    _cptr temp;
    do {
      temp = __old;
      ++temp._M_count;
    } while (!compare_ex_strong_with_acq_rlx(_M_head, __old, temp));
    ++__old._M_count;
  }
};

} // namespace stack
} // namesapce concurrent

#endif /* GUID_FACE677D_151A_4BD8_A5C5_A90C78A67B0D_YANG */

#ifndef GUID_C05CC3AE_2619_4B6F_AA72_D47079DC59F3_YANG
#define GUID_C05CC3AE_2619_4B6F_AA72_D47079DC59F3_YANG

#include "sync_stack_base.h"

namespace concurrent {
namespace stack {

template<typename _T, stack_type _St = stack_type::low_load>
class sync_stack;

// low
template<typename _T>
class sync_stack<_T, stack_type::low_load> : _sync_ll_stack_base<_T>
{
  using _super = _sync_ll_stack_base<_T>;
  using node = _sync_ll_stack_base<_T>::node;

public:
  void push(_T val)
  {
    node* temp = new node({ make_shared<_T>(move(val)), nullptr });
    temp->_M_next = _super::load_with_rlx(_super::_M_head);
    while (
      !_super::compare_ex_with_rel_rlx(_super::_M_head, temp->_M_next, temp))
      ;
  }

  auto pull()
  {
    _super::add_with_rlx(1);
    node* temp = _super::load_with_rlx(_super::_M_head);
    while (temp && !_super::compare_ex_with_acq_rlx(
                     _super::_M_head, temp, temp->_M_next))
      ;
    shared_ptr<_T> res;
    if (temp) {
      res.swap(temp->_M_data);
    }
    _super::try_reclaim(temp);
    return res;
  }

  bool pull(_T& __val)
  {
    _super::add_with_rlx(1);
    node* temp = _super::load_with_rlx(_super::_M_head);
    while (temp && !_super::compare_ex_with_acq_rlx(
                     _super::_M_head, temp, temp->_M_next))
      ;
    if (temp) {
      __val = move(*(temp->_M_data));
    }
    _super::try_reclaim(temp);
    return temp ? true : false;
  }
};

// high
template<typename _T>
class sync_stack<_T, stack_type::high_load> : _sync_hl_stack_base<_T>
{
  using _super = _sync_hl_stack_base<_T>;
  using _cptr = _super::_cptr;
  using node = _super::node;

public:
  void push(_T __val)
  {
    node* __raw =
      new node{ move(__val), _super::load_with_rlx(_super::_M_head) };
    _cptr temp{ 1, __raw };
    while (!_super::compare_ex_with_rel_rlx(_super::_M_head, __raw->_M_next, temp));
  }

  shared_ptr<_T> pull()
  {
    auto __old = _super::load_with_rlx(_super::_M_head);
    for (;;) {
      _super::update_head_count(__old);
      node* const __raw = __old._M_ptr;
      if (!__raw)
        return shared_ptr<_T>();
      if (_super::compare_ex_strong_with_rlx(
            _super::_M_head, __old, __raw->_M_next)) {
        shared_ptr<_T> res;
        res.swap(__raw->_M_data);
        int const __decrease = __old._M_count - 2;
        if (_super::add_with_rel(__raw->_M_count, __decrease) == -__decrease) {
          delete __raw;
        }
        return res;
      } else if (_super::sub_with_rlx(__raw->_M_count, 1) == 1){
        _super::as_fence(__raw->_M_count);
        delete __raw;
      }
    }
  }
};

} // namespace stacks

template<typename _T>
using ll_stack = stack::sync_stack<_T, stack::stack_type::low_load>;

template<typename _T>
using hl_stack = stack::sync_stack<_T, stack::stack_type::high_load>;

} // namespace concurrent

#endif /* GUID_C05CC3AE_2619_4B6F_AA72_D47079DC59F3_YANG */

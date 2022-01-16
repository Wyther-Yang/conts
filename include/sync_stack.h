#ifndef GUID_C05CC3AE_2619_4B6F_AA72_D47079DC59F3_YANG
#define GUID_C05CC3AE_2619_4B6F_AA72_D47079DC59F3_YANG

#include "sync_stack_base.h"

namespace concurrent {
namespace stack {

template<typename _T, stack_type _St = stack_type::low_load>
class sync_stack;

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
    if (temp)
    {
      __val = move(*(temp->_M_data));
    }
    _super::try_reclaim(temp);
    return temp ? true : false;
  }
};

} // namespace stacks
} // namespace concurrent

#endif /* GUID_C05CC3AE_2619_4B6F_AA72_D47079DC59F3_YANG */

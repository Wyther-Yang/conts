#ifndef GUID_C3540073_7322_4CB9_A0EE_00AD354625C7_YANG
#define GUID_C3540073_7322_4CB9_A0EE_00AD354625C7_YANG

#include "dispatcher.h"

namespace concurrent {
namespace msg {
template<typename _Q>
class receiver
{
  _Q _M_q;

public:
  constexpr _dispatcher_base<_Q> wait() { return _dispatcher_base<_Q>(&_M_q); }

  constexpr _Q* get() { return &_M_q; }
};

} // namespace msg

using msg::receiver;

} 

#endif /* GUID_C3540073_7322_4CB9_A0EE_00AD354625C7_YANG */

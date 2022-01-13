#ifndef GUID_E3C46196_73F5_4365_9965_E892FD627CA4_YANG
#define GUID_E3C46196_73F5_4365_9965_E892FD627CA4_YANG

#include "dispatcher.h"

namespace concurrent {
namespace msg {

template<typename _Q>
class sender
{
  _Q* _M_q;

public:
  __DEFAULT_CTOR(sender);

  sender(_Q* __q) : _M_q(__q) {}

  template<typename _M>
  void send(_M&& __m)
  {
    _M_q->try_push(make_shared<_msg_impl<std::decay_t<_M>>>(std::forward<_M>(__m)));
  }
};

} // namespace msg

using msg::sender;

} // namespace concurrent

#endif /* GUID_E3C46196_73F5_4365_9965_E892FD627CA4_YANG */

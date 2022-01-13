#ifndef GUID_E81285F4_85F6_42D9_8475_EF973363C2B6_YANG
#define GUID_E81285F4_85F6_42D9_8475_EF973363C2B6_YANG

#include "message.h"
#include "sync_queue.h"

namespace concurrent {
namespace msg {

template<typename _M, typename _Q, typename _F, typename _Pre>
class _dispatcher_impl
{
  using _msg_t = shared_ptr<_msg_base>;

  template<typename, typename, typename, typename>
  friend class _dispatcher_impl;

  _Q* _M_q;
  _F _M_f;
  _Pre* _M_up;
  bool _M_chained;

  bool patcher(_msg_t const& __msg)
  {
    if (auto temp = std::dynamic_pointer_cast<_msg_impl<_M>>(__msg)) {
      _M_f(temp->_M_data);
      return true;
    }
    return _M_up->patcher(__msg);
  }

  void batch_processing()
  {
    for (;;) {
      auto __m(_M_q->pull());
      if (patcher(__m))
        break;
    }
  }

public:
  __DEFAULT_CTOR(_dispatcher_impl);

  __NO_COPYABLE(_dispatcher_impl);

  _dispatcher_impl(_Q* __q, _F __f, _Pre* __pre)
    : _M_q(__q)
    , _M_f(move(__f))
    , _M_up(__pre)
    , _M_chained(false)
  {
    _M_up->_M_chained = true;
  }

  template<typename _OtherM, typename _OtherF>
  constexpr auto handle(_OtherF&& __f)
  {
    return _dispatcher_impl<_OtherM, _Q, _OtherF, _dispatcher_impl>(
      _M_q, std::forward<_OtherF>(__f), this);
  }

  ~_dispatcher_impl() noexcept(false)
  {
    if (!_M_chained)
      batch_processing();
  }

};

template<typename _Q>
class _dispatcher_base
{
  using _msg_t = shared_ptr<_msg_base>;

  template<typename, typename, typename, typename>
  friend class _dispatcher_impl;

  _Q* _M_q;
  bool _M_chained;

  bool patcher(_msg_t const& __msg)
  {
    if (std::dynamic_pointer_cast<_msg_impl<sync_queue_closed>>(__msg))
      __THROW_EXCEPTION(sync_queue_closed());
    return false;
  }

  void batch_processing()
  {
    for (;;) {
      auto __m(_M_q->pull());
      patcher(__m);
    }
  }

public:
  __DEFAULT_CTOR(_dispatcher_base);

  __NO_COPYABLE(_dispatcher_base);

  _dispatcher_base(_Q* __q)
    : _M_q(__q)
    , _M_chained(false)
  {}

  template<typename _M, typename _F>
  constexpr auto handle(_F&& __f)
  {
    return _dispatcher_impl<_M, _Q, _F, _dispatcher_base>(
      _M_q, std::forward<_F>(__f), this);
  }

  ~_dispatcher_base() noexcept(false)
  {
    if (!_M_chained)
      batch_processing();
  }
};

} // namespace msg

using msg_queue = queue::sync_queue<msg::_msg_base, queue::normal>;

} // namespace concurrent

#endif /* GUID_E81285F4_85F6_42D9_8475_EF973363C2B6_YANG */

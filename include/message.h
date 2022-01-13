#ifndef GUID_F7FBFF51_189F_4154_B561_4B017356A333_YANG
#define GUID_F7FBFF51_189F_4154_B561_4B017356A333_YANG

#include "pre_config.h"
namespace concurrent {
namespace msg {

struct _msg_base
{
  virtual ~_msg_base() {}
};

template<typename _T>
struct _msg_impl : _msg_base
{
  _T _M_data;

  __DEFAULT_CTOR(_msg_impl);

  __DELETE_COPY_CTOR(_msg_impl);

  explicit _msg_impl(_T __data)
    : _M_data(move(__data))
  {}

  virtual ~_msg_impl() {}
};

} // namespace msg

using msg::_msg_base;
using msg::_msg_impl;

} // namespace concurrent

#endif /* GUID_F7FBFF51_189F_4154_B561_4B017356A333_YANG */

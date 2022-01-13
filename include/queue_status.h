#ifndef GUID_E9FCD7BE_C9B9_4F8E_8119_E493E89CF4AC_YANG
#define GUID_E9FCD7BE_C9B9_4F8E_8119_E493E89CF4AC_YANG

#include <exception>

namespace concurrent {
namespace queue {

enum class queue_op_status
{
  empty = 0,
  success,
  busy,
  closed,
  timeout,
  get_fail
};

enum class queue_type
{
  normal = 0,

  /* lock-free with lower load */
  lf_with_low_load,

  /* lock-free with higher load */
  lf_with_high_load,

  steal
};

struct sync_queue_closed : std::exception
{};

namespace alias {
  
constexpr queue_type normal = queue_type::normal;
constexpr queue_type lf_with_low_load = queue_type::lf_with_low_load;
constexpr queue_type lf_with_high_load = queue_type::lf_with_high_load;
constexpr queue_type steal = queue_type::steal;

constexpr queue_op_status success = queue_op_status::success;
constexpr queue_op_status empty = queue_op_status::empty;
constexpr queue_op_status closed = queue_op_status::closed;
constexpr queue_op_status timeout = queue_op_status::timeout;
constexpr queue_op_status get_fail = queue_op_status::get_fail;
constexpr queue_op_status busy = queue_op_status::busy;

constexpr auto m_relaxed = std::memory_order_relaxed;

} // namespace alias

using namespace alias;

} // namespace queue

using queue::queue_op_status;
using queue::queue_type;
using queue::sync_queue_closed;

} // namespace concurrent

#endif /* GUID_E9FCD7BE_C9B9_4F8E_8119_E493E89CF4AC_YANG */

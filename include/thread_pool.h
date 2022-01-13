#ifndef GUID_F70A8FBD_D4E8_4507_992D_0B07DFE9D8A0_YANG
#define GUID_F70A8FBD_D4E8_4507_992D_0B07DFE9D8A0_YANG

#include "thread_pool_base.h"

namespace concurrent {

template<typename _Callable = threads::_empty_callable>
using default_pool =
  pool::base_thread_pool<normal_queue<base_work_t>, _Callable>;

template<typename _Callable = threads::_empty_callable>
using ctr_pool =
  pool::base_thread_pool<steal_queue<base_work_t>, wrapper_thread<_Callable>>;

}

#endif /* GUID_F70A8FBD_D4E8_4507_992D_0B07DFE9D8A0_YANG */

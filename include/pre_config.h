#ifndef GUID_C6E17319_8C0C_4D42_A7A1_FDC9614B5152_YANG
#define GUID_C6E17319_8C0C_4D42_A7A1_FDC9614B5152_YANG

#include <assert.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <ext/aligned_buffer.h>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#ifdef __NEED_OUTSTREAM
#include <ostream>
#endif
#ifdef __NEED_INSTREAM
#include <istream>
#endif

#if __cplusplus > 201703L
#define __ALIGNAS_WITH_GCC alignas(__GCC_CONSTRUCTIVE_SIZE)
#else
#define __MAYBE_ALIGNAS_NUM alignas(64)
#endif

#define __ALWAYS_INLINE inline __attribute__((__always_inline__))

#define __BASE_REF(VALUE) *dynamic_cast<base*>(VALUE)
#define __ROTATION std::this_thread::yield();

#define __INTERRUPT_POINT concurrent::threads::interrupt_point();
#define __INTERRUPT_CONDI_WAIT(CONDI, LOCK, PRED)                              \
  concurrent::threads::interrupt_wait(CONDI, LOCK, PRED);
#define __INTERRUPT_FUTURE_WAIT(FUTURE)                                        \
  concurrent::threads::interrupt_wait(FUTURE);

#define __DELETE_COPY_CTOR(TYPE) TYPE(TYPE const&) = delete;
#define __DELETE_COPY_ASSIGN(TYPE) TYPE& operator=(TYPE const&) = delete;

#define __ACCEPT_RVALUE_ASSIGN(TYPE) TYPE& operator=(TYPE&&) = default;
#define __ACCEPT_RVALUE_CTOR(TYPE) TYPE(TYPE&&) = default;

#define __DEFAULT_CTOR(TYPE) TYPE() = default;

#define __NO_COPYABLE(TYPE)                                                    \
  __DELETE_COPY_CTOR(TYPE)                                                     \
  __DELETE_COPY_ASSIGN(TYPE)

#define __ACCEPTION_RVALUE_DEFAULT(TYPE)                                       \
  __ACCEPT_RVALUE_ASSIGN(TYPE)                                                 \
  __ACCEPT_RVALUE_CTOR(TYPE)

#if defined __USE_BOOST_THREAD
#define BOOST_THREAD_VERSION 5
#endif

#define __THROW_EXCEPTION(x) throw(x)

using std::atomic_bool;
using std::condition_variable;
using std::deque;
using std::future;
using std::lock_guard;
using std::make_shared;
using std::move;
using std::mutex;
using std::packaged_task;
using std::promise;
using std::shared_ptr;
using std::thread;
using std::unique_lock;
using std::unique_ptr;
using std::vector;

#endif /* GUID_C6E17319_8C0C_4D42_A7A1_FDC9614B5152_YANG */

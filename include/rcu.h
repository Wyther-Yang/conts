#ifndef GUID_C8FFFB80_FA1C_4BDA_B89C_363FFEACF808_YANG
#define GUID_C8FFFB80_FA1C_4BDA_B89C_363FFEACF808_YANG
/**
 * eg:
 *      rcu_t<T> r;
 * read-thread A:
 *      auto src = r.read();
 *      if (src) {
 *
 *        // do what you want to with src.data();
 *      }
 * update-thread B:
 *      r.update(val);
 *
 */
#include <atomic>
#include <mutex>
#include <utility>

using std::atomic;
using std::forward;
using std::lock_guard;
using std::mutex;

constexpr uint32_t locked = 0x0100'0000U;
constexpr uint32_t unlocked = 0x0011'1111U;

namespace conts {

template<typename _T>
class r_transfer
{
  struct r_payoff
  {
    _T data{};
    atomic<uint32_t> cnt{};
  };

  template<typename>
  friend class rcu_t;

  r_payoff* raw_p{};

public:
  r_transfer() = default;
  r_transfer(r_transfer const&) = default;
  r_transfer& operator=(r_transfer&) = default;
  r_transfer& operator=(r_transfer&&) = default;

  r_transfer(_T&& src)
    : raw_p(new r_payoff{ forward<_T>(src), locked + 1 })
  {}

  ~r_transfer()
  {
    if (raw_p && (raw_p->cnt).fetch_sub(1, std::memory_order_relaxed) == 1)
      delete raw_p;
  }

  inline _T& data() { return raw_p->data; }

  inline operator bool() { return raw_p; }
};

// read-copy update class
template<typename _T>
class rcu_t
{
  mutex m{};
  r_transfer<_T> src{};

public:
  rcu_t() = default;
  rcu_t(rcu_t const&) = delete;
  rcu_t& operator=(rcu_t&) = delete;
  rcu_t& operator=(rcu_t&&) = delete;
  ~rcu_t() = default;

  // forbid cpy the reture value to other thread, only moveable. bcause not
  // increase the counter if cpy.
#if __cplusplus > 201703L
  [[nodiscard]]
#endif
  r_transfer<_T>
  read()
  {
    lock_guard<mutex> locked(m);
    if (!src)
      return src;
    (src.raw_p)->cnt.fetch_add(1, std::memory_order_relaxed);
    return src;
  }

  void update(_T&& val)
  {
    r_transfer<_T> tsf{ forward<_T>(val) };

    // side effect is RAII
    r_transfer<_T> clue = src;

    {
      lock_guard<mutex> locked(m);
      src = tsf;
    }

    if (!clue)
      return;

    uint32_t tmp = (clue.raw_p)->cnt.load(std::memory_order_relaxed);
    while (!(clue.raw_p)
              ->cnt.compare_exchange_weak(
                tmp, tmp & unlocked, std::memory_order_relaxed))
      ;

  } //
};

} // namespace conts

#endif /* GUID_C8FFFB80_FA1C_4BDA_B89C_363FFEACF808_YANG */

#ifndef GUID_B6787F65_F2EE_4434_B5EE_66C924552467_YANG
#define GUID_B6787F65_F2EE_4434_B5EE_66C924552467_YANG

#include <atomic>

using std::atomic;

namespace conts {

class seq_sp_lock 
{
  atomic<bool> sp_lock{};

  atomic<uint64_t> seq_cnt{};

public:
  seq_sp_lock() = default;
  seq_sp_lock(seq_sp_lock&) = delete;
  seq_sp_lock(seq_sp_lock&&) = delete;
  seq_sp_lock& operator=(seq_sp_lock&) = delete;
  seq_sp_lock& operator=(seq_sp_lock&&) = delete;
  ~seq_sp_lock() = default;


  inline void w_seqlock();

  inline void w_sequnlock();

  /**
   * note: no pointers , and side effects in critical region
   * eg:
   *  uint64_t seq;
   *  do {
   *    seq = r_seqbegin();
   * 
   *    // critical region 
   * 
   *  } while (r_seqretry(seq))
  */
  inline uint64_t r_seqbegin();

  inline bool r_seqretry(uint64_t);
};

inline void seq_sp_lock::w_seqlock() {
  bool nobody{false};
  for (;;) {
    if (sp_lock.compare_exchange_weak(nobody, 1, std::memory_order_relaxed)) {
      seq_cnt.fetch_add(1, std::memory_order_acquire);
      return;
    }
    nobody = false;
 //   cpu_relax();
  }
}

inline void seq_sp_lock::w_sequnlock()
{
  seq_cnt.fetch_sub(1, std::memory_order_release);
  sp_lock.store(0, std::memory_order_relaxed);
}

inline uint64_t seq_sp_lock::r_seqbegin()
{
  for (;;)
  {
    uint64_t tmp = seq_cnt.load(std::memory_order_acquire);
    if (!(tmp & 1UL))
      return tmp;
  }
  return 0;
}

inline bool seq_sp_lock::r_seqretry(uint64_t seq)
{
  uint64_t tmp = seq_cnt.load(std::memory_order_relaxed);
  return (tmp & 1UL) || (seq != tmp);
}

} // namespace conts

#endif /* GUID_B6787F65_F2EE_4434_B5EE_66C924552467_YANG */

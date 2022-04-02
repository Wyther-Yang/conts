#ifndef GUID_DB5D7CCB_4286_494C_B4A3_7E01F86CF874_YANG
#define GUID_DB5D7CCB_4286_494C_B4A3_7E01F86CF874_YANG

#include <atomic>

using std::atomic;

namespace conts {

// with 80x86 arch, opt in spin loop
inline void cpu_relax() { __asm__ __volatile__("rep;nop" : : : "memory"); }

#define R_FREE  0x01000000U
#define W_FREE  0x00000000U

#define NUM 0x00111111U

#define onps 0x10000000U

// a read and write lock
class rw_sp_lock 
{
  atomic<uint32_t> cnt{R_FREE};

 public:
  rw_sp_lock() = default;
  rw_sp_lock(rw_sp_lock &) = delete;
  rw_sp_lock(rw_sp_lock &&) = delete;
  rw_sp_lock &operator=(rw_sp_lock &) = delete;
  rw_sp_lock &operator=(rw_sp_lock &&) = delete;
  ~rw_sp_lock() = default;

  inline int r_lock();

  inline void w_lock();

  inline void r_unlock();

  inline void w_unlock();
};

inline int rw_sp_lock::r_lock() 
{
  for (;;) {
    uint32_t old = cnt.load(std::memory_order_relaxed);
    if (old & R_FREE) {
      if (cnt.compare_exchange_strong(old, old + 1, std::memory_order_acquire,
                                      std::memory_order_relaxed))
        return (old + 1) & NUM;
    }
    cpu_relax();
  }
  return onps;
}

inline void rw_sp_lock::w_lock() 
{
  uint32_t tmp = R_FREE;
  while (!cnt.compare_exchange_strong(tmp, W_FREE, std::memory_order_acquire,
                                     std::memory_order_relaxed))
  {
    cpu_relax();
    tmp = R_FREE;
  }
}

inline void rw_sp_lock::r_unlock() 
{
  cnt.fetch_sub(1, std::memory_order_relaxed);
}

inline void rw_sp_lock::w_unlock() 
{
  cnt.store(R_FREE, std::memory_order_release);
}

} // namespace conts

#endif /* GUID_DB5D7CCB_4286_494C_B4A3_7E01F86CF874_YANG */

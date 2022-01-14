#ifndef GUID_B7B4DD3C_23E3_45A0_8C8D_07C1785847B8_YANG
#define GUID_B7B4DD3C_23E3_45A0_8C8D_07C1785847B8_YANG

#include "conts.h"

#include <iostream>

inline auto
now() noexcept
{
  return std::chrono::high_resolution_clock::now();
}

template<typename _F>
void
bm_test(_F __f, unsigned __m)
{
  unsigned const __max_runs{ __m };

  std::cout << "starts: \n";
  unsigned __average{};
  for (unsigned i{}; i < __max_runs; ++i) {
    auto const __start{ now() };
    __f();
    std::chrono::duration<double, std::milli> elapsed{ now() - __start };
    std::cout << "the task spent " << elapsed.count() << " ms\n";
    __average += elapsed.count();
  }
  std::cout << "Average time: " << __average / __max_runs << " ms\n";
}

#endif /* GUID_B7B4DD3C_23E3_45A0_8C8D_07C1785847B8_YANG */

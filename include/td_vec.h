#ifndef GUID_D47DDEC4_2A76_442E_BDDD_0CBEF0DBB12E_YANG
#define GUID_D47DDEC4_2A76_442E_BDDD_0CBEF0DBB12E_YANG

/**
 * wrapper vector for two dimensional array to reduce heap memory
 * eg: vector<wrapper_vec<type, default_column>> vc;
 *     vc.push_back({val1});
 *     vc.push_back({val1, val2});
 *     vc.push_back({val1m val3, ...});
 */

#include <vector>
#include <utility>

using std::forward;
using std::vector;

namespace conts {

template<typename _T, size_t N = 10>
struct wrapper_vec : std::vector<_T>
{

  using base = std::vector<_T>;

  wrapper_vec()
    : vector<_T>(N)
  {}

  template<typename... _Ts>
  wrapper_vec(_Ts&&... args)
    : base(N)
  {
    push_back(forward<_Ts>(args)...);
  }

  template<typename... _Ts>
  void push_back(_Ts&&... values)
  {
    push_helper(0, *this, forward<_Ts>(values)...);
  }

private:
  void push_helper(int, vector<_T>&) { return; }

  template<typename _head, typename... _tail>
  void push_helper(int n, vector<_T>& base, _head&& h, _tail&&... t)
  {
    base[n] = forward<_head>(h);
    push_helper(n + 1, base, forward<_tail>(t)...);
  }
};

} // namespace conts

#endif /* GUID_D47DDEC4_2A76_442E_BDDD_0CBEF0DBB12E_YANG */

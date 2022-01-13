#ifndef GUID_D9052FE2_5CF0_45F9_8571_D868574B35C0_YANG
#define GUID_D9052FE2_5CF0_45F9_8571_D868574B35C0_YANG

#include "pre_config.h"

namespace concurrent {
namespace traits {

template<typename... _Elements>
struct typelist
{};

template<typename _T>
struct wrapper_t
{
  using type = _T;
};

#ifdef __USE_STL_TUPLE
#include <tuple>
using std::tuple;
#else
// forward dec for spec tuple
template<typename...>
class tuple;
#endif

// front
template<typename>
struct __front_impl;

template<typename _H, typename... _Tail>
struct __front_impl<typelist<_H, _Tail...>>
{
  using type = _H;
};

template<typename _H, typename... _Tail>
struct __front_impl<tuple<_H, _Tail...>>
{
  using type = _H;
};

// alias
template<typename _List>
using front_t = typename __front_impl<_List>::type;

// popfront
template<typename>
struct __popfront_impl;

template<typename _H, typename... _Tail>
struct __popfront_impl<typelist<_H, _Tail...>>
{
  using type = typelist<_Tail...>;
};

template<typename _H, typename... _Tail>
struct __popfront_impl<tuple<_H, _Tail...>>
{
  using type = tuple<_Tail...>;
};

// alias
template<typename _L>
using popfront_t = typename __popfront_impl<_L>::type;

// pushfront
template<typename, typename>
struct __pushfront_impl;

template<typename... _E, typename _New>
struct __pushfront_impl<typelist<_E...>, _New>
{
  using type = typelist<_New, _E...>;
};

template<typename... _E, typename _New>
struct __pushfront_impl<tuple<_E...>, _New>
{
  using type = tuple<_New, _E...>;
};

// alias
template<typename _L, typename _N>
using pushfront_t = typename __pushfront_impl<_L, _N>::type;

// pushback
template<typename, typename>
struct __pushback_impl;

template<typename... _E, typename _New>
struct __pushback_impl<typelist<_E...>, _New>
{
  using type = typelist<_E..., _New>;
};

template<typename... _E, typename _New>
struct __pushback_impl<tuple<_E...>, _New>
{
  using type = tuple<_E..., _New>;
};

// alias
template<typename _L, typename _N>
using pushback_t = typename __pushback_impl<_L, _N>::type;

// Nth element
template<typename _L, size_t __N>
struct __Nth_element_impl : __Nth_element_impl<popfront_t<_L>, __N - 1>
{};

template<typename _L>
struct __Nth_element_impl<_L, 0> : __front_impl<_L>
{};

// alias
template<typename _L, unsigned __N>
using Nth_element_t = typename __Nth_element_impl<_L, __N>::type;

// empty
template<typename>
struct __is_empty_impl
{
  static constexpr bool value = false;
};

template<>
struct __is_empty_impl<typelist<>>
{
  static constexpr bool value = true;
};

template<>
struct __is_empty_impl<tuple<>>
{
  static constexpr bool value = true;
};

// alias
template<typename _L>
bool constexpr is_empty_v = __is_empty_impl<_L>::value;

// using condition_trait
using std::conditional_t;

// largest type
template<typename _L, bool = is_empty_v<_L>>
class __largest_type_impl;

template<typename _L>
class __largest_type_impl<_L, false>
{
  using __curr_t = front_t<_L>;
  using __tail_ts = typename __largest_type_impl<popfront_t<_L>>::type;

public:
  using type =
    conditional_t<(sizeof(__curr_t) >= sizeof(__tail_ts)), __curr_t, __tail_ts>;
};

template<typename _L>
class __largest_type_impl<_L, true>
{
public:
  using type = unsigned char;
};

// alias
template<typename _L>
using largest_t = typename __largest_type_impl<_L>::type;

// reverse
template<typename _L, bool = is_empty_v<_L>>
struct __reverse_impl;

template<typename _L>
struct __reverse_impl<_L, false>
  : pushback_t<typename __reverse_impl<popfront_t<_L>>::type, front_t<_L>>
{};

template<typename _L>
struct __reverse_impl<_L, true>
{
  using type = _L;
};

// alias
template<typename _L>
using reverse_t = typename __reverse_impl<_L>::type;

// popback
template<typename _L>
struct __popback_impl
{
  using type = reverse_t<popfront_t<reverse_t<_L>>>;
};

// alias
template<typename _L>
using popback_t = typename __popback_impl<_L>::type;

// transform
template<typename _L, template<typename> class _Metafunc, bool = is_empty_v<_L>>
struct __transform_impl;

template<typename _L, template<typename> class _Metafunc>
struct __transform_impl<_L, _Metafunc, false>
  : __pushfront_impl<typename __transform_impl<popfront_t<_L>, _Metafunc>::type,
                     typename _Metafunc<front_t<_L>>::type>
{};

template<typename _L, template<typename> class _Metafunc>
struct __transform_impl<_L, _Metafunc, true>
{
  using type = _L;
};

// alias
template<typename _L, template<typename> class _Metafunc>
using transform_t = typename __transform_impl<_L, _Metafunc>::type;

// sort_base
template<typename _L,
         typename _Element,
         template<typename, typename>
         class _Compare,
         bool = is_empty_v<_L>>
class __sort_base;

template<typename _L,
         typename _Element,
         template<typename, typename>
         class _Compare>
class __sort_base<_L, _Element, _Compare, false>
{
  using head = conditional_t<_Compare<_Element, front_t<_L>>::value,
                             _Element,
                             front_t<_L>>;
  using tail =
    conditional_t<_Compare<_Element, front_t<_L>>::value,
                  wrapper_t<_L>,
                  __sort_base<popfront_t<_L>, _Element, _Compare>>::type;

public:
  using type = pushfront_t<tail, head>;
};

template<typename _L,
         typename _Element,
         template<typename, typename>
         class _Compare>
class __sort_base<_L, _Element, _Compare, true>
{
public:
  using type = pushfront_t<_L, _Element>;
};

template<typename _L,
         template<typename, typename>
         class _Compare,
         bool = is_empty_v<_L>>
struct __sort_impl;

template<typename _L, template<typename, typename> class _Compare>
struct __sort_impl<_L, _Compare, false>
  : __sort_base<typename __sort_impl<popfront_t<_L>, _Compare>::type,
                front_t<_L>,
                _Compare>
{};

template<typename _L, template<typename, typename> class _Compare>
struct __sort_impl<_L, _Compare, true>
{
  using type = _L;
};

// alias
template<typename _L, template<typename, typename> class _Compare>
using sort_t = typename __sort_impl<_L, _Compare>::type;

template<size_t _V>
struct Cv
{
  static constexpr size_t value = _V;
};

template<size_t... _Vs>
using vlist = typelist<Cv<_Vs>...>;

#ifdef __NEED_OUTSTREAM
template<typename _CharT, typename... _Cvs>
std::basic_ostream<_CharT>&
operator<<(std::basic_ostream<_CharT>& os, typelist<_Cvs...>)
{
  (os << ... << _Cvs::value);
  return os;
}
#endif

using std::__and_;
using std::__nonesuch;
using std::__or_;
using std::decay_t;
using std::enable_if_t;
using std::false_type;
using std::forward;
using std::integral_constant;
using std::is_assignable;
using std::is_final;
using std::is_nothrow_constructible;
using std::true_type;

template<typename _T>
struct __not_final
{
  static constexpr bool value = !is_final<_T>::value;
};

} // namespace traits

} // namespace concurrent

#endif /* GUID_D9052FE2_5CF0_45F9_8571_D868574B35C0_YANG */

/*
 * the con-utility is not the same as <utility> of STL.
 *
 * @ordinary tool, includes RAII and some wrapper class.
 *
 * @con-function, a callable must be copyable if wrapped from function of STL,
 * but con-function not, it only needs and requires moveable.
 *
 * @con-tuple, VS.tuple of STL it has clear direction, so no such many
 * restrictions and easy to use, meanwhile impl of apply from STL is not good
 * and in line with my expectation.
 *
 * @con-variant, variant of STL is tedious and not practical because of the two
 * aspect that too much space is used and too many restrictions.
 *
 * @con-any, remove some error prone-interfaces, only one construction
 * applied(eg. any a(std::string("hello"))), suggest that using variant rather
 * than any.
 */

#ifndef GUID_F9DE9EF9_7A27_4845_B247_99F40AC0DCCF_YANG
#define GUID_F9DE9EF9_7A27_4845_B247_99F40AC0DCCF_YANG

#include "con_traits.h"
#include "pre_config.h"

namespace concurrent {
namespace utility {

using namespace traits;

////////// ordinary tool

//
template<typename _Ths>
class joiner
{
  _Ths& _M_v;

public:
  joiner(_Ths& __v)
    : _M_v(__v)
  {}

  ~joiner()
  {
    for (auto& _v : _M_v) {
      if (_v.joinable())
        _v.join();
    }
  }
};

//
template<typename _R, typename _F>
auto
get_task(future<_R>& __f, _F __fs)
{
  promise<_R> __p;
  __f = __p.get_future();
  return [p = move(__p), f = move(__fs)]() mutable {
    try {
      p.set_value(f());
    } catch (...) {
      p.set_exception(std::current_exception());
    }
  };
}

///// ordiary tool finished

/////////////// work type

template<typename _R, typename... _ParaType>
class function;

template<typename _R, typename... _ParaType>
struct _impl_base
{
  virtual _R call(_ParaType...) const = 0;
  virtual ~_impl_base() {}
};

template<typename _F, typename _R, typename... _ParaType>
struct _impl_type : _impl_base<_R, _ParaType...>
{
  mutable _F _M_f;

  _impl_type(_F __f)
    : _M_f(std::move(__f))
  {}

  _R call(_ParaType... __paras) const
  {
    return _M_f(std::forward<_ParaType>(__paras)...);
  }
};

template<typename _R, typename... _ParaType>
class function<_R(_ParaType...)>
{
  std::shared_ptr<_impl_base<_R, _ParaType...>> _M_impl;

public:
  __DEFAULT_CTOR(function);

  __NO_COPYABLE(function);

  __ACCEPTION_RVALUE_DEFAULT(function);

  template<typename _F>
  function(_F&& __f)
    : _M_impl(new _impl_type<_F, _R, _ParaType...>(std::forward<_F>(__f)))
  {}

  _R operator()(_ParaType... __paras) const
  {
    return _M_impl->call(std::forward<_ParaType>(__paras)...);
  }
};

///// work type finished

///////////////// con-tuple

template<typename... _Elements>
class tuple;

template<typename... _Types>
struct _tupleConstraints
{
  static constexpr bool __nothrow_ctor()
  {
    return is_nothrow_constructible<_Types...>::value;
  }
};

// element_base

template<size_t _Idx,
         typename _Head,
         bool = std::is_class_v<_Head> && !std::is_final_v<_Head>>
struct _Head_base;

template<size_t _Idx, typename _Head>
struct _Head_base<_Idx, _Head, true> : _Head
{
  constexpr __DEFAULT_CTOR(_Head_base);

  __DELETE_COPY_CTOR(_Head_base);

  constexpr __ACCEPTION_RVALUE_DEFAULT(_Head_base);

  template<typename _Other>
  constexpr _Head_base(_Other&& __head)
    : _Head(forward<_Other>(__head))
  {}

  /// helper func

  static constexpr _Head& _M_head(_Head_base& __h) noexcept { return __h; }

  static constexpr _Head const& _M_head(_Head_base const& __ch) noexcept
  {
    return __ch;
  }
};

template<size_t _Idx, typename _Head>
struct _Head_base<_Idx, _Head, false>
{

  _Head _M_head_val;

  constexpr __DEFAULT_CTOR(_Head_base);

  __DELETE_COPY_CTOR(_Head_base);

  constexpr __ACCEPTION_RVALUE_DEFAULT(_Head_base);

  template<typename _Other>
  constexpr _Head_base(_Other&& __head)
    : _M_head_val(forward<_Other>(__head))
  {}

  /// helper func

  static constexpr _Head& _M_head(_Head_base& __h) noexcept
  {
    return __h._M_head_val;
  }

  static constexpr _Head const& _M_head(_Head_base const& __ch) noexcept
  {
    return __ch._M_head_val;
  }
};

// tuple_impl

template<size_t _Idx, typename... _Elements>
struct _Tuple_impl;

template<size_t _Idx, typename _Head>
struct _Tuple_impl<_Idx, _Head> : private _Head_base<_Idx, _Head>
{
  using _Base = _Head_base<_Idx, _Head>;

  template<size_t, typename...>
  friend struct _Tuple_impl;

  constexpr __DEFAULT_CTOR(_Tuple_impl);

  __DELETE_COPY_CTOR(_Tuple_impl);

  constexpr __ACCEPTION_RVALUE_DEFAULT(_Tuple_impl);

  template<typename _Other>
  constexpr _Tuple_impl(_Other&& __other)
    : _Base(forward<_Other>(__other))
  {}

  static constexpr _Head& _M_head(_Tuple_impl& __head) noexcept
  {
    return _Base::_M_head(__head);
  }

  static constexpr _Head const& _M_head(_Tuple_impl const& __head)
  {
    return _Base::_M_head(__head);
  }

  template<typename _Other>
  constexpr void _M_assign(_Tuple_impl<_Idx, _Other>&& __other)
  {
    _M_head(*this) = std::move(__other);
  }

  constexpr void _M_swap(_Tuple_impl& __same)
  {
    std::swap(_M_head(*this, _M_head(__same)));
  }
};

template<size_t _Idx, typename _Head, typename... _Tail>
struct _Tuple_impl<_Idx, _Head, _Tail...>
  : _Tuple_impl<_Idx + 1, _Tail...>
  , private _Head_base<_Idx, _Head>
{
  using _Super = _Tuple_impl<_Idx + 1, _Tail...>;
  using _Base = _Head_base<_Idx, _Head>;

  template<size_t, typename...>
  friend struct _Tupel_impl;

  constexpr __DEFAULT_CTOR(_Tuple_impl);

  constexpr __ACCEPTION_RVALUE_DEFAULT(_Tuple_impl);

  __DELETE_COPY_CTOR(_Tuple_impl);

  template<typename _Ohead,
           typename... _Otail,
           typename = enable_if_t<sizeof...(_Tail) == sizeof...(_Otail)>>
  _Tuple_impl(_Ohead&& __head, _Otail&&... __tail)
    : _Super(forward<_Otail>(__tail)...)
    , _Base(forward<_Ohead>(__head))
  {}

  static constexpr _Head& _M_head(_Tuple_impl& __head) noexcept
  {
    return _Base::_M_head(__head);
  }

  static constexpr _Head const& _M_head(_Tuple_impl const& __head) noexcept
  {
    return _Base::_M_head(__head);
  }

  static constexpr _Super& _M_tail(_Tuple_impl& __tail) noexcept
  {
    return __tail;
  }

  static _Super const& _M_tail(_Tuple_impl const& __tail) noexcept
  {
    return __tail;
  }

  template<typename... _Other>
  constexpr void _M_assign(_Tuple_impl<_Idx, _Other...>&& __other)
  {
    using _Other_super = _Tuple_impl<_Idx, _Other...>;
    _M_head(*this) = std::move(_Other_super::_M_head(__other));
    _M_tail(*this)._M_assign(std::move(_Other_super::_M_tail(__other)));
  }

  constexpr void _M_swap(_Tuple_impl& __in)
  {
    std::swap(_M_head(*this), _M_head(__in));
    _Super::_M_swap(_M_tail(__in));
  }
};

// tuple

template<typename... _Elements>
class tuple : public _Tuple_impl<0, _Elements...>
{
  typedef _tupleConstraints<_Elements...> _TCC;
  using _Super = _Tuple_impl<0, _Elements...>;

public:
  constexpr tuple() noexcept(_TCC::__nothrow_ctor())
    : _Super()
  {}

  constexpr __ACCEPTION_RVALUE_DEFAULT(tuple);

  template<typename... _Otypes>
  constexpr tuple(_Otypes&&... _others) noexcept(_TCC::__nothrow_ctor())
    : _Super(forward<_Otypes>(_others)...)
  {}

  template<typename... _Otypes>
  static constexpr std::
    __enable_if_t<sizeof...(_Elements) == sizeof...(_Otypes), bool>
    __assignable()
  {
    return __and_<is_assignable<_Elements&, _Otypes>...>::value;
  }

  template<typename... _Otypes>
  constexpr std::__enable_if_t<__assignable<_Otypes...>(), tuple&> operator=(
    tuple<_Otypes...>&& __others)
  {
    this->_M_assign(std::move(__others));
    return *this;
  }
};

template<>
class tuple<>
{
public:
  constexpr __DEFAULT_CTOR(tuple);

  __DELETE_COPY_CTOR(tuple);

  constexpr __ACCEPTION_RVALUE_DEFAULT(tuple);
};

// tuple_size
template<typename...>
struct tuple_size;

template<typename... _Elements>
struct tuple_size<tuple<_Elements...>>
  : integral_constant<size_t, sizeof...(_Elements)>
{};

template<typename _Tp>
inline constexpr size_t tuple_size_v = tuple_size<_Tp>::value;

// return type

template<size_t, typename...>
struct return_type;

template<size_t _Idx, typename _Head, typename... _Tail>
struct return_type<_Idx, tuple<_Head, _Tail...>>
  : return_type<_Idx - 1, tuple<_Tail...>>
{};

template<typename _Head, typename... _Tail>
struct return_type<0, tuple<_Head, _Tail...>>
{
  using type = _Head;
};

template<size_t _Idx, typename _Tp>
using return_type_t = typename return_type<_Idx, _Tp>::type;

// get

template<size_t _Idx, typename _Head, typename... _Tail>
constexpr _Head&
__get_impl(_Tuple_impl<_Idx, _Head, _Tail...>& __t)
{
  return _Tuple_impl<_Idx, _Head, _Tail...>::_M_head(__t);
}

template<size_t _Idx, typename _Head, typename... _Tail>
constexpr _Head const&
__get_impl(_Tuple_impl<_Idx, _Head, _Tail...> const& __t)
{
  return _Tuple_impl<_Idx, _Head, _Tail...>::_M_head(__t);
}

template<size_t _Idx, typename... _Elements>
constexpr return_type_t<_Idx, tuple<_Elements...>> const&
get(tuple<_Elements...> const& __t)
{
  return __get_impl<_Idx>(__t);
}

template<size_t _Idx, typename... _Elements>
constexpr return_type_t<_Idx, tuple<_Elements...>>&
get(tuple<_Elements...>& __t)
{
  return __get_impl<_Idx>(__t);
}

template<size_t _Idx, typename... _Elements>
constexpr std::__enable_if_t<_Idx >= sizeof...(_Elements)>
get(tuple<_Elements...> const& __t) = delete;

template<typename... _Elements>
constexpr tuple<decay_t<_Elements>...>
make_tuple(_Elements&&... __els)
{
  return tuple<decay_t<_Elements>...>(forward<_Elements>(__els)...);
}

// apply

template<typename _F, typename _Tp>
constexpr void
apply(_F&& __f, _Tp&& __t, std::index_sequence<>)
{}

template<typename _F, typename _Tp, size_t _Curr, size_t... _Idx>
constexpr void
apply(_F&& __f, _Tp&& __t, std::index_sequence<_Curr, _Idx...>)
{
  std::__invoke(forward<_F>(__f), get<_Curr>(forward<_Tp>(__t)));
  apply(forward<_F>(__f), forward<_Tp>(__t), std::index_sequence<_Idx...>{});
}

template<typename _F, typename _Tp>
constexpr void
apply(_F&& __f, _Tp&& __t)
{
  using __v_t = std::make_index_sequence<tuple_size_v<decay_t<_Tp>>>;
  apply(forward<_F>(__f), forward<_Tp>(__t), __v_t{});
}

//////////////////// con-tuple finished.

/////// con-union

inline constexpr size_t union_npos = -1;

template<typename... _Types>
class variant;

struct empty_variant_exception : std::exception
{};

// make index_v from type
template<typename _T, typename... _Types>
struct _indx_of : integral_constant<size_t, union_npos>
{};

template<typename _T, typename _H, typename... _Tail>
struct _indx_of<_T, _H, _Tail...>
  : integral_constant<
      size_t,
      std::is_same_v<_T, _H> ? 0 : _indx_of<_T, _Tail...>::value + 1>
{};

//
template<typename... _Types>
class _union_storage
{
  using _largest_type = largest_t<typelist<_Types...>>;

  __gnu_cxx::__aligned_membuf<_largest_type> _M_storage;
  size_t _M_anchor;

public:
  _union_storage()
    : _M_anchor(union_npos)
  {}

  constexpr size_t _M_get_anchor() const { return _M_anchor; }
  constexpr void _M_set_anchor(size_t __ac) { _M_anchor = __ac; }
  constexpr void* _M_get_ptr() { return _M_storage._M_addr(); }
  constexpr void const* _M_get_ptr() const { return _M_storage._M_addr(); }
  template<typename _T>
  constexpr _T* _M_get_ptr_as()
  {
    return std::launder(reinterpret_cast<_T*>(_M_get_ptr()));
  }

  template<typename _T>
  constexpr _T const* _M_get_ptr_as() const
  {
    return std::launder(reinterpret_cast<_T*>(_M_get_ptr()));
  }
};

//
template<typename _T, typename... _Types>
class _union_base
{
  using _Super = variant<_Types...>;
  constexpr _Super& _M_get_super() { return *static_cast<_Super*>(this); }
  constexpr _Super const& _M_get_super() const
  {
    return *static_cast<_Super*>(this);
  }

protected:
  static constexpr size_t __pos = _indx_of<_T, _Types...>::value;

public:
  constexpr __DEFAULT_CTOR(_union_base);

  _union_base(_T const& __v)
  {
    ::new (_M_get_super()._M_get_ptr()) _T(__v);
    _M_get_super()._M_set_anchor(__pos);
  }

  _union_base(_T&& __v)
  {
    ::new (_M_get_super()._M_get_ptr()) _T(move(__v));
    _M_get_super()._M_set_anchor(__pos);
  }

  void _M_destory()
  {
    if (_M_get_super()._M_get_anchor() == __pos)
      _M_get_super().template _M_get_ptr_as<_T>()->~_T();
  }

  _Super& operator=(_T const& __v)
  {
    if (_M_get_super()._M_get_anchor() == __pos)
      *_M_get_super().template _M_get_ptr_as<_T>() = __v;
    else {
      _M_get_super()._M_destory();
      new (_M_get_super()._M_get_ptr()) _T(__v);
      _M_get_super()._M_set_anchor(__pos);
    }
    return _M_get_super();
  }

  _Super& operator=(_T&& __v)
  {
    if (_M_get_super()._M_get_anchor() == __pos)
      *_M_get_super().template _M_get_ptr_as<_T>() = move(__v);
    else {
      _M_get_super()._M_destory();
      new (_M_get_super()._M_get_ptr()) _T(move(__v));
      _M_get_super()._M_set_anchor(__pos);
    }
    return _M_get_super();
  }
};

//
template<typename... _Types>
class variant
  : private _union_storage<_Types...>
  , private _union_base<_Types, _Types...>...
{
private:
  template<typename _Callable, typename _H, typename... _Tail>
  void _M_apply_impl(_Callable&& __f, typelist<_H, _Tail...>)
  {
    if (this->is<_H>()) {
      forward<_Callable>(__f)(this->get<_H>());
    } else if constexpr (sizeof...(_Tail) > 0) {
      _M_apply_impl(forward<_Callable>(__f), typelist<_Tail...>{});
    } else {
      throw empty_variant_exception();
    }
  }

public:
  template<typename, typename...>
  friend class _union_base;

  using _union_base<_Types, _Types...>::_union_base...;
  using _union_base<_Types, _Types...>::operator=...;

  __DEFAULT_CTOR(variant);

  __DELETE_COPY_CTOR(variant);

  template<typename... _Otypes>
  variant(variant<_Otypes...>& __other)
  {
    __other.apply([this](auto const& __val) { *this = __val; });
  }

  template<typename... _Otypes>
  variant(variant<_Otypes...>&& __other)
  {
    __other.apply([this](auto& __val) { *this = move(__val); });
  }

  bool empty() { return this->_M_get_anchor() == union_npos; }

  template<typename _T>
  bool is() const
  {
    return this->_M_get_anchor() == _union_base<_T, _Types...>::__pos;
  }

  template<typename _T>
  _T& get() &
  {
    assert(is<_T>());
    return *this->template _M_get_ptr_as<_T>();
  }

  template<typename _T>
  _T const& get() const&
  {
    assert(is<_T>());
    return *this->template _M_get_ptr_as<_T>();
  }

  template<typename _T>
  _T&& get() &&
  {
    assert(is<_T>());
    return move(*this->template _M_get_ptr_as<_T>());
  }

  void _M_destory()
  {
    (_union_base<_Types, _Types...>::_M_destory(), ...);
    this->_M_set_anchor(union_npos);
  }

  ~variant() { _M_destory(); }

  template<typename _Callable>
  void apply(_Callable&& __f)
  {
    return this->_M_apply_impl(forward<_Callable>(__f), typelist<_Types...>());
  }
};

//////////////////// con-union finished.

/////// con-any

class any
{
  template<typename _Tp>
  struct _any_manager_internal;

  template<typename _Tp>
  struct _any_manager_external;

  enum class _any_op
  {
    _access = 0,
    _get_type_info,
    _destory,
    //  _exchange
  };

  union _storage
  {
    void* _M_ptr;
    __gnu_cxx::__aligned_membuf<void*> _M_buffer;
  };

  union _args
  {
    void* obj;
    std::type_info const* type_info;
    any* a;
  };

  template<typename _Tp>
  using _choice =
    std::integral_constant<bool, (sizeof(_Tp) <= sizeof(_storage))>;

  template<typename _Tp>
  using _manager = std::conditional_t<_choice<_Tp>::value,
                                      _any_manager_internal<_Tp>,
                                      _any_manager_external<_Tp>>;
  //
  _storage _M_s;
  void (*_M_m)(_any_op, any const*, _args*){};

  template<typename _Tp>
  struct _any_manager_internal
  {
    static void _S_manage(_any_op __which, any const* __any, _args* __arg)
    {
      _Tp const* ptr = static_cast<_Tp const*>(__any->_M_s._M_buffer._M_addr());
      //_Tp const* ptr = reinterpret_cast<_Tp
      // const*>(__any->_M_s._M_buffer._M_storage);
      switch (__which) {
        case _any_op::_access:
          __arg->obj = const_cast<_Tp*>(ptr);
          break;
        case _any_op::_get_type_info:
          __arg->type_info = &typeid(_Tp);
          break;
        case _any_op::_destory:
          ptr->~_Tp();
          break;
      }
    }

    static void _S_create(_storage& __s, _Tp&& value)
    {
      ::new (__s._M_buffer._M_addr()) _Tp(std::forward<_Tp>(value));
    }

    static constexpr _Tp* _S_access(_storage& __s)
    {
      return static_cast<_Tp*>(__s._M_buffer._M_addr());
    }
  };

  template<typename _Tp>
  struct _any_manager_external
  {

    static void _S_manage(_any_op __which, any const* __any, _args* __arg)
    {
      _Tp const* ptr = static_cast<_Tp const*>(__any->_M_s._M_ptr);
      switch (__which) {
        case _any_op::_access:
          __arg->obj = const_cast<_Tp*>(ptr);
          break;
        case _any_op::_get_type_info:
          __arg->type_info = &typeid(_Tp);
          break;
        case _any_op::_destory:
          delete ptr;
          break;
      }
    }

    static void _S_create(_storage& __s, _Tp&& value)
    {
      __s._M_ptr = new _Tp(std::forward<_Tp>(value));
    }

    static constexpr _Tp* _S_access(_storage& __s)
    {
      return static_cast<_Tp*>(__s._M_ptr);
    }
  };

  //
  template<typename _Tp>
  friend _Tp* get(any*);

public:
  any() = default;
  any(any&) = delete;
  any(any&&) = delete;
  any& operator=(any&) = delete;
  any& operator=(any&&) = delete;
  ~any() { reset(); }

  template<typename _Tp, typename _M = _manager<std::decay_t<_Tp>>>
  explicit any(_Tp&& __val) noexcept
    : _M_m(_M::_S_manage)
  {
    _M::_S_create(_M_s, std::forward<_Tp>(__val));
  }

  inline bool has_value() const noexcept { return _M_m; }

  std::type_info const& type() const noexcept
  {
    if (!has_value())
      return typeid(void);
    _args __arg;
    _M_m(_any_op::_get_type_info, this, &__arg);
    return *__arg.type_info;
  }

  void reset()
  {
    if (has_value()) {
      _M_m(_any_op::_destory, this, nullptr);
      _M_m = nullptr;
    }
  }
};

//
template<typename _Tp>
_Tp*
get(any* __any)
{
  using _Up = std::remove_cv_t<_Tp>;
  if (__any->type() == typeid(_Up))
    return any::_manager<_Up>::_S_access(__any->_M_s);
  return nullptr; // may should throw a exception for mismatch
}

} // namespace utility

using utility::any;
using utility::tuple;
using utility::variant;

using utility::function;
using utility::get_task;
using utility::joiner;

//
typedef function<void()> base_work_t;

} // namespace concurrent

#endif /* GUID_F9DE9EF9_7A27_4845_B247_99F40AC0DCCF_YANG */

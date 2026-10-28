/** @file libcx/traits/types.hh **/

#ifndef CX_TRAITS_TYPES_HH
#define CX_TRAITS_TYPES_HH

#include "libcx/traits/qualifier.hh"

namespace cx {

// Integral

template<typename T> proposition cx__is_sintegral              = false;
template<> proposition           cx__is_sintegral<char>        = true;
template<> proposition           cx__is_sintegral<signed char> = true;
template<> proposition           cx__is_sintegral<wchar_t>     = true;
template<> proposition           cx__is_sintegral<short>       = true;
template<> proposition           cx__is_sintegral<int>         = true;
template<> proposition           cx__is_sintegral<long>        = true;
template<> proposition           cx__is_sintegral<long long>   = true;
#if CX_HAS_INT128
  template<> proposition         cx__is_sintegral<__int128>    = true;
#endif
template<typename T> proposition is_sintegral = cx__is_sintegral<rm_cv<T>>;

template<typename T> proposition cx__is_uintegral                     = false;
template<> proposition           cx__is_uintegral<bool>               = true;
template<> proposition           cx__is_uintegral<unsigned char>      = true;
#if CX_HAS_CHAR8
  template<> proposition         cx__is_uintegral<char8_t>            = true;
#endif
#if CX_HAS_CHAR16
  template<> proposition         cx__is_uintegral<char16_t>           = true;
#endif
#if CX_HAS_CHAR32
  template<> proposition         cx__is_uintegral<char32_t>           = true;
#endif
template<> proposition           cx__is_uintegral<unsigned short>     = true;
template<> proposition           cx__is_uintegral<unsigned int>       = true;
template<> proposition           cx__is_uintegral<unsigned long>      = true;
template<> proposition           cx__is_uintegral<unsigned long long> = true;
#if CX_HAS_INT128
  template<> proposition         cx__is_uintegral<unsigned __int128>  = true;
#endif
template<typename T> proposition is_uintegral = cx__is_uintegral<rm_cv<T>>;

template<typename T> proposition is_integral = is_sintegral<T> || is_uintegral<T>;

// Float

template<typename T> proposition cx__is_floating              = false;
template<> proposition           cx__is_floating<float>       = true;
template<> proposition           cx__is_floating<double>      = true;
template<> proposition           cx__is_floating<long double> = true;
#if CX_HAS_FLOAT128
  template<> proposition         cx__is_floating<__float128>  = true;
#endif
template<typename T> proposition is_floating = cx__is_floating<rm_cv<T>>;

// Raw pointer
// TODO: is `is_base_ptr` a better name ?

template<typename T> proposition cx__is_raw_ptr     = false;
template<typename T> proposition cx__is_raw_ptr<T*> = true;
template<typename T> proposition is_raw_ptr = cx__is_raw_ptr<rm_cv<T>>;

template<typename T>
proposition is_base_ptr = is_raw_ptr<T> && !is_raw_ptr<rm_cv<rm_ptr<rm_cv<T>>>>;

// Arithmetic

template<typename T>
proposition is_arithmetic = is_integral<T> || is_floating<T> || is_raw_ptr<T>;

// Raw array

template<typename T> proposition          cx__is_raw_array       = false;
template<typename T> proposition          cx__is_raw_array<T[]>  = true;
template<typename T, usize N> proposition cx__is_raw_array<T[N]> = true;
template<typename T> proposition is_raw_array = cx__is_raw_array<rm_cv<T>>;

// Callables

template<typename T>
proposition is_func = !is_ref<T> && !is_const<T const>;

template<typename T>
proposition is_ptr_to_func = is_raw_ptr<rm_cvref<T>> && is_func<rm_ptr<rm_cvref<T>>>;

template<typename T>
proposition is_ref_to_func = is_ref<T> && is_func<rm_cvref<T>>;

template<typename T>
proposition is_func_any = is_func<T> || is_ptr_to_func<T> || is_ref_to_func<T>;

template<typename T>
proposition is_fntor_type = requires { &rm_cvref<T>::operator(); };  // XXX:

template<typename T>
proposition is_callable = is_func_any<T> || is_fntor_type<T>;

// Void

template<typename T> proposition is_void       = false;
template<>           proposition is_void<void> = true;

// TODO:
// template<typename T> proposition is_smart_ptr = false;
// template<typename U> proposition is_smart_ptr<std::shared_ptr<U>> = true;
// template<typename U, typename D> proposition is_smart_ptr<std::unique_ptr<U, D>> = true;

}       // namespace cx
#endif  // CX_TRAITS_TYPES_HH

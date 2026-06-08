/** @file libcx/traits/types.hh **/

#ifndef CX_TRAITS_TYPES_HH
#define CX_TRAITS_TYPES_HH

#include "libcx/traits/qualifier.hh"

namespace cx {

// Integral

template<typename T> predicate ___is_sintegral = false;
template<> predicate           ___is_sintegral<char> = true;
template<> predicate           ___is_sintegral<signed char> = true;
template<> predicate           ___is_sintegral<wchar_t> = true;
template<> predicate           ___is_sintegral<short> = true;
template<> predicate           ___is_sintegral<int> = true;
template<> predicate           ___is_sintegral<long> = true;
template<> predicate           ___is_sintegral<long long> = true;
#if CX_HAS_INT128
  template<> predicate         ___is_sintegral<__int128> = true;
#endif
template<typename T> predicate is_sintegral = ___is_sintegral<rm_cv<T>>;

template<typename T> predicate ___is_uintegral = false;
template<> predicate           ___is_uintegral<bool> = true;
template<> predicate           ___is_uintegral<unsigned char> = true;
#if CX_HAS_CHAR8
  template<> predicate         ___is_uintegral<char8_t> = true;
#endif
#if CX_HAS_CHAR16
  template<> predicate         ___is_uintegral<char16_t> = true;
#endif
#if CX_HAS_CHAR32
  template<> predicate         ___is_uintegral<char32_t> = true;
#endif
template<> predicate           ___is_uintegral<unsigned short> = true;
template<> predicate           ___is_uintegral<unsigned int> = true;
template<> predicate           ___is_uintegral<unsigned long> = true;
template<> predicate           ___is_uintegral<unsigned long long> = true;
#if CX_HAS_INT128
  template<> predicate         ___is_uintegral<unsigned __int128> = true;
#endif
template<typename T> predicate is_uintegral = ___is_uintegral<rm_cv<T>>;

template<typename T> predicate is_integral = is_sintegral<T> || is_uintegral<T>;

// Float

template<typename T> predicate ___is_floating = false;
template<> predicate ___is_floating<float> = true;
template<> predicate ___is_floating<double> = true;
template<> predicate ___is_floating<long double> = true;
#if CX_HAS_FLOAT128
  template<> predicate ___is_floating<__float128> = true;
#endif
template<typename T> predicate is_floating = ___is_floating<rm_cv<T>>;

// Raw pointer
// TODO: is `is_base_ptr` a better name ?

template<typename T> predicate ___is_raw_ptr = false;
template<typename U> predicate ___is_raw_ptr<U*> = true;
template<typename T> predicate is_raw_ptr = ___is_raw_ptr<rm_cv<T>>;

template<typename T>
predicate is_base_ptr = is_raw_ptr<T> && !is_raw_ptr<rm_cv<rm_ptr<rm_cv<T>>>>;

// Arithmetic

template<typename T>
predicate is_arithmetic = is_integral<T> || is_floating<T> || is_raw_ptr<T>;

// Raw array

template<typename T> predicate ___is_raw_array = false;
template<typename T> predicate ___is_raw_array<T[]> = true;
template<typename T, usize N> predicate ___is_raw_array<T[N]> = true;
template<typename T> predicate is_raw_array = ___is_raw_array<rm_cv<T>>;

// Callables

template<typename T> predicate is_func = !is_ref<T> && !is_const<T const>;

template<typename T>
predicate is_ptr_to_func = is_raw_ptr<rm_cvref<T>> && is_func<rm_ptr<rm_cvref<T>>>;

template<typename T>
predicate is_ref_to_func = is_ref<T> && is_func<rm_cvref<T>>;

template<typename T>
predicate is_func_any = is_func<T> || is_ptr_to_func<T> || is_ref_to_func<T>;

template<typename T>
predicate is_fntor_type = requires { &rm_cvref<T>::operator(); };  // XXX:

template<typename T>
predicate is_callable = is_func_any<T> || is_fntor_type<T>;

// Void

template<typename T> predicate is_void = false;
template<> predicate is_void<void> = true;

// TODO:
// template<typename T> predicate is_smart_ptr = false;
// template<typename U> predicate is_smart_ptr<std::shared_ptr<U>> = true;
// template<typename U, typename D> predicate is_smart_ptr<std::unique_ptr<U, D>> = true;

}       // namespace cx
#endif  // CX_TRAITS_TYPES_HH

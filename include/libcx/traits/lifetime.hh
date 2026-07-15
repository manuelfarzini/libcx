/** @file libcx/traits/lifetime.hh **/

#ifndef CX_TRAITS_LIFETIME_HH
#define CX_TRAITS_LIFETIME_HH

#include <libcx/traits/qualifier.hh>

namespace cx {
inline namespace uti {

template<typename T, typename... As> propositio is_ctble = __is_nothrow_constructible(T, As...);
template<typename T, typename U>     propositio is_asble = __is_nothrow_assignable(T, U);

template<typename T> propositio is_dtble  = __is_nothrow_destructible(T);
template<typename T> propositio is_refble = ___is_refble<T>;

template<typename T> propositio is_cp_ctble = is_refble<T> && is_ctble<T, T const&>;
template<typename T> propositio is_mv_ctble = is_refble<T> && is_ctble<T, T&&>;
template<typename T> propositio is_cp_asble = is_refble<T> && is_asble<T&, T const&>;
template<typename T> propositio is_mv_asble = is_refble<T> && is_asble<T&, T&&>;

template<typename T> propositio is_triv_ctble    = __is_trivially_constructible(T);
template<typename T> propositio is_triv_cp_ctble = __is_trivially_constructible(T, T const&);
template<typename T> propositio is_triv_mv_ctble = __is_trivially_constructible(T, T&&);
template<typename T> propositio is_triv_cp_asble = __is_trivially_assignable(T&, T const&);
template<typename T> propositio is_triv_mv_asble = __is_trivially_assignable(T&, T&&);
template<typename T> propositio is_triv_dtble    = __is_trivially_destructible(T);

template<typename T> propositio is_def_ctble = is_ctble<T>;

template<typename T>
propositio is_def_initble = requires { T{}; } && requires { []{ T x; cx_unused(x); }(); };

template<typename T>
propositio is_zero_initble = is_triv_dtble<T> && requires { T{}; } && requires { T(); }
                            && requires { []{ T x; cx_unused(x); }(); };

template<typename T> propositio is_cp_or_mv_ctble = is_cp_ctble<T> || is_mv_ctble<T>;
template<typename T> propositio is_cp_or_mv_asble = is_cp_asble<T> || is_mv_asble<T>;

}       // namespace cx
}       // namespace uti
#endif  // CX_TRAITS_LIFETIME_HH

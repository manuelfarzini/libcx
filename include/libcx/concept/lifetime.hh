/** @file libcx/concepts/lifetime.hh **/

#ifndef CX_CONCEPTS_LIFETIME_HH
#define CX_CONCEPTS_LIFETIME_HH

#include "libcx/traits/lifetime.hh"

namespace cx {
inline namespace uti {

template<typename... Ts> concept DefCtble    = (is_def_ctble<Ts> && ...);
template<typename... Ts> concept CpCtble     = (is_cp_ctble<Ts> && ...);
template<typename... Ts> concept MvCtble     = (is_mv_ctble<Ts> && ...);
template<typename... Ts> concept CpAsble     = (is_cp_asble<Ts> && ...);
template<typename... Ts> concept MvAsble     = (is_mv_asble<Ts> && ...);
template<typename... Ts> concept Dtble       = (is_dtble<Ts> && ...);
template<typename... Ts> concept CpOrMvCtble = (is_cp_or_mv_ctble<Ts> && ...);
template<typename... Ts> concept CpOrMvAsble = (is_cp_or_mv_asble<Ts> && ...);
template<typename... Ts> concept DefInitble  = (is_def_initble<Ts> && ...);
template<typename... Ts> concept ZeroInitble = (is_zero_initble<Ts> && ...);

template<typename... Ts> predicate TrivDtble = (is_triv_dtble<Ts> && ...);
// FIX:(manu): should impose sizeof...(Ts) >= 1 ?
// static_assert(DefCtble<>); // e.g. this one should work? Now does.

}       // namespace cx
}       // namespace uti
#endif  // CX_CONCEPTS_LIFETIME_HH

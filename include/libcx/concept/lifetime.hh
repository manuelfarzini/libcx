/** @file libcx/concept/lifetime.hh **/

#ifndef CX_CONCEPT_LIFETIME_HH
#define CX_CONCEPT_LIFETIME_HH

#include "libcx/trait/lifetime.hh"

namespace cx {
inline namespace uti {

template<typename... Ts> concept DefCtble = (is_def_ctble<Ts> && ...);
template<typename... Ts> concept CpCtble = (is_cp_ctble<Ts> && ...);
template<typename... Ts> concept MvCtble = (is_mv_ctble<Ts> && ...);
template<typename... Ts> concept CpAsble = (is_cp_asble<Ts> && ...);
template<typename... Ts> concept MvAsble = (is_mv_asble<Ts> && ...);
template<typename... Ts> concept Dtble = (is_dtble<Ts> && ...);
template<typename... Ts> concept CpOrMvCtble = (is_cp_or_mv_ctble<Ts> && ...);
template<typename... Ts> concept CpOrMvAsble = (is_cp_or_mv_asble<Ts> && ...);
template<typename... Ts> concept DefInitble = (is_def_initble<Ts> && ...);
template<typename... Ts> concept ValueInitble = (is_value_initble<Ts> && ...);
template<typename... Ts> concept PassiveValue = (is_passive_value<Ts> && ...);
template<typename... Ts> concept TrivCtble = (is_triv_ctble<Ts> && ...);
template<typename... Ts> concept TrivCpCtble = (is_triv_cp_ctble<Ts> && ...);
template<typename... Ts> concept TrivMvCtble = (is_triv_mv_ctble<Ts> && ...);
template<typename... Ts> concept TrivCpAsble = (is_triv_cp_asble<Ts> && ...);
template<typename... Ts> concept TrivMvAsble = (is_triv_mv_asble<Ts> && ...);
template<typename... Ts> concept TrivCopyable = (is_triv_copyable<Ts> && ...);
template<typename... Ts> concept TrivDtble = (is_triv_dtble<Ts> && ...);
// FIX:(manu): should impose sizeof...(Ts) >= 1 ?
// static_assert(DefCtble<>); // e.g. this one should work? Now does.

}       // namespace uti
}       // namespace cx
#endif  // CX_CONCEPT_LIFETIME_HH

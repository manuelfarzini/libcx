/** @file libcx/uti/common_ref.hh **/

#ifndef CX_UTI_COMMON_REF_HH
#define CX_UTI_COMMON_REF_HH

#include "libcx/conf/macro.hh"
#include "libcx/traits/qualifier.hh"
#include "libcx/traits/relation.hh"
#include "libcx/uti/sfinae.hh"

namespace cx  {
inline namespace uti {

template<typename R> void accept__common_ref(R);
#if defined(cx__accept)
    #undef cx__accept
#endif
#define cx__accept(R, S) accept__common_ref<R>(declv<S>())

template<typename T, typename U>
using ___common_ref_help =

    // lvalue/lvalue: prefer & then const&
    condition<is_lref<T> && is_lref<U>,
        condition<requires { cx__accept(T, U); },                   rm_ref<T>&,
        condition<requires { cx__accept(U, T); },                   rm_ref<U>&,
        condition<requires { cx__accept(rm_ref<T> const&, U); },    rm_ref<T> const&,
        condition<requires { cx__accept(rm_ref<U> const&, T); },    rm_ref<U> const&,
                                                                    void >>>>,
    // rvalue/rvalue: prefer && then const&
    condition<is_rref<T> && is_rref<U>,
        condition<requires { cx__accept(rm_ref<T>&&, U); },         rm_ref<T>&&,
        condition<requires { cx__accept(rm_ref<U>&&, T); },         rm_ref<U>&&,
        condition<requires { cx__accept(rm_ref<T> const&, U); },    rm_ref<T> const&,
        condition<requires { cx__accept(rm_ref<U> const&, T); },    rm_ref<U> const&,
                                                                    void >>>>,
    // mixed: const& if possible
    condition<requires { cx__accept(rm_ref<T> const&, U); },        rm_ref<T> const&,
    condition<requires { cx__accept(rm_ref<U> const&, T); },        rm_ref<U> const&,
                                                                    void >>>>;

// if the ref-route give void, try a value type via ternary_res
template<typename T, typename U>
using ___common_ref_or_val =
    condition<!same_as<___common_ref_help<T, U>, void>,  ___common_ref_help<T, U>,
    condition<requires { typename ternary_res<T, U>; },  rm_cvref<ternary_res<T, U>>, void >>;

template<typename... Args>
struct ___common_ref;

template<typename T>
struct ___common_ref<T> { using Type = T; };

template<typename T, typename U>
struct ___common_ref<T, U> { using Type = ___common_ref_or_val<T, U>; };

template<typename T, typename U, typename... Rest>
struct ___common_ref<T, U, Rest...> {
    using Type = typename ___common_ref<___common_ref_or_val<T, U>, Rest...>::Type;
};

template<typename... Args>
using common_ref = typename ___common_ref<Args...>::Type;

template<typename T, typename U>
predicate common_ref_w = same_as<common_ref<T, U>, common_ref<U, T>>
                         && is_convertible<T, common_ref<T, U>>
                         && is_convertible<U, common_ref<T, U>>;

#undef  cx__accept
}       // namespace cx
}       // namespace cx
#endif  // CX_UTI_COMMON_REF_HH

/** @file libcx/concept/type.hh */

#ifndef CX_CONCEPTS_TYPES_HH
#define CX_CONCEPTS_TYPES_HH

#include "libcx/traits/lifetime.hh"
#include "libcx/traits/relation.hh"
#include "libcx/traits/types.hh"

namespace cx {
inline namespace uti {
  
template<typename T> concept SomeSintegral = is_sintegral<T>;
template<typename T> concept SomeUintegral = is_uintegral<T>;
template<typename T> concept SomeIntegral = is_sintegral<T> || is_uintegral<T>;

template<typename T> concept SomeFunction = is_func_any<T>;
template<typename T> concept SomeFunctor = is_fntor_type<T>;
template<typename T> concept SomeCallable = is_callable<T>;

template<typename T> concept SomeRawPointer = is_raw_ptr<T>;
template<typename U> concept SomePlainArithmetic = is_arithmetic<U> && !is_cv_qual<U>;

template<typename T>
concept SomePlainRawPointer = SomeRawPointer<T> && !is_cv_qual<T> && !is_cv_qual<rm_ptr<T>>;

template<typename T> concept PlainZeroInitble = is_zero_initble<T> && !is_cv_qual<T>;

template<typename T, typename U> concept SameAs = same_as<T, U>;

}       // namespace cx
}       // namespace uti
#endif  // CX_CONCEPTS_TYPES_HH

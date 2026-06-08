/** @file libcx/uti/multi.hh **/

#ifndef CX_TRAITS_MULTI_HH
#define CX_TRAITS_MULTI_HH

#include "libcx/conf/prelude.hh"
#include "libcx/traits/relation.hh"
#include "libcx/uti/typeseq.hh"

namespace cx {
inline namespace uti {

// Same type or reference of the same type
// Multi typewise version
template<SomeTypeSeq L1, SomeTypeSeq L2, isize I = 0>
    where (L1::size == L2::size)
struct _multi_same_or_ref {
  using T = TypeAt<I, L1>;
  using U = TypeAt<I, L2>;
  glob predicate value = same_or_ref<T, U> &&
                           _multi_same_or_ref<L1, L2, I + 1>::value;
};

template<SomeTypeSeq L1, SomeTypeSeq L2> 
struct _multi_same_or_ref<L1, L2> { glob predicate value = true; };

template<SomeTypeSeq Ts, SomeTypeSeq Us>
predicate multi_same_or_ref = _multi_same_or_ref<Ts, Us>::value;

}       // namespace uti
}       // namespace cx
#endif  // CX_TRAITS_MULTI_HH

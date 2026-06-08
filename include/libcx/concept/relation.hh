/** @file libcx/concept/relation.hh **/

#ifndef CX_CONCEPTS_RELATION_HH
#define CX_CONCEPTS_RELATION_HH

#include "libcx/traits/relation.hh"

namespace cx {
inline namespace uti {

template<typename T, typename U> concept SameAs = same_as<T, U>;

}       // namespace uti
}       // namespace cx
#endif  // CX_CONCEPTS_RELATION_HH

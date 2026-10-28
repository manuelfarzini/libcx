/** @file  libcx/uti/declval.hh **/

#ifndef CX_UTI_DECLVAL_HH
#define CX_UTI_DECLVAL_HH

#include "libcx/traits/qualifier.hh"

namespace cx {

template<typename T> cons fn declv() noexce -> add_rref<T>;

}       // namespace cx
#endif  // CX_UTI_DECLVAL_HH

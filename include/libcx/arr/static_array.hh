/** @file libcx/arr/static_array.hh **/

#ifndef CX_ARR_STATIC_ARRAY_HH
#define CX_ARR_STATIC_ARRAY_HH

#include "libcx/concept/lifetime.hh"
#include "libcx/traits/logic.hh"
#include "libcx/uti/iterator.hh"
#include "libcx/uti/sfinae.hh"

namespace cx {
inline namespace arr {

template<ZeroInitble E, isize Cap, typename S = isize> where (Cap > 0)
struct StaticArray
{
    CX_DEFINE_MEMBER_TYPES(E, S);
    using Self = StaticArray<Elem, Cap, Size>;

    onedef glob cons Size cap{Cap};
    Elem buf[Cap]{};

    inln cons fn operator[](usize const i) noexce -> Elem& { return buf[i]; }
    inln cons fn operator[](usize const i) const noexce -> Elem const& { return buf[i]; }

    inln cons fn begin() noexce -> Iter { return buf; }
    inln cons fn end() noexce -> Iter { return buf + cap; }
    inln cons fn begin() const noexce -> Kter { return buf; }
    inln cons fn end() const noexce -> Kter { return buf + cap; }
};

CX_CONCEPT_GEN_TEMPL(StaticArray, is_static_array, SomeStaticArray,
                     VA_(typename Tp, usize Cap, typename Sz), VA_(Tp, Cap, Sz));

template<ZeroInitble E> StaticArray(E) -> StaticArray<E, 1>;

// TODO: convertible_to to bind to the first passed element type e.g. `StaticArray arr = {1u, 2, 3}`
template<typename E, typename... Es>
StaticArray(E, Es...) -> StaticArray<enable_if<bvariand<same_as<E, Es>...>, E>, 1 + sizeof...(Es)>;


template<typename Arr, typename E> predicate is_static_array_of = false;
template<typename T, isize Cap, typename S> predicate is_static_array_of<StaticArray<T, Cap, S>, T> = true;
template<typename Arr, typename E> concept StaticArrayOf = is_static_array_of<Arr, E>;

}       // namespace arr
}       // namespace cx
#endif  // CX_ARR_STATIC_ARRAY_HH

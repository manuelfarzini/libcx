/** @file libcx/uti/typeseq.hh **/

#ifndef CX_UTI_TYPESEQ_HH
#define CX_UTI_TYPESEQ_HH

#include "libcx/conf/macro.hh"
#include "libcx/concept/type.hh"
#include "libcx/uti/members.hh"

namespace cx {
inline namespace uti {

////////////////////////////////////////////
// Type sequence

namespace ___type_seq_detail {
struct ___NullType {};
}

template<typename... Ts> struct TypeSeq;

template<>
struct TypeSeq<> {
    using Head = ___type_seq_detail::___NullType;
    using Rest = TypeSeq<>;

    onedef glob cons bool empty = true;
    onedef glob cons isize size = 0;
};

template<typename H, typename... Rs>
struct TypeSeq<H, Rs...> {
    using Head = H;
    using Rest = TypeSeq<Rs...>;

    onedef glob cons bool empty = false;
    onedef glob cons isize size = 1 + sizeof...(Rs);
};

CX_CONCEPT_GEN_TEMPL(TypeSeq, is_type_seq, SomeTypeSeq, typename... Ts, Ts...);

template<typename... Ts>
onedef cons isize size_of_tseq = TypeSeq<Ts...>::size;

template<typename>
proposition cx__always_false = false; // TODO: to be moved

////////////////////////////////////////////
// Homogeneous type sequence

template<SomeTypeSeq Seq>
cons fn cx__tseq_is_homo() -> bool
{
    if constexpr (Seq::empty || Seq::Rest::empty) {
        return true;
    } else { // XXX: or cvref ???
        return same_as<typename Seq::Head, typename Seq::Rest::Head>
               && cx__tseq_is_homo<typename Seq::Rest>();
    }
}

template<SomeTypeSeq Seq>
proposition tseq_is_homo = cx__tseq_is_homo<Seq>();

template<typename... Ts>
proposition va_is_homo = tseq_is_homo<TypeSeq<Ts...>>;

////////////////////////////////////////////
// Type at a given index in a type sequence

template<isize Idx, SomeTypeSeq Seq, bool Empty = Seq::empty>
struct cxTypeAt;

template<isize Idx, SomeTypeSeq Seq>
struct cxTypeAt<Idx, Seq, false>
    : cxTypeAt<Idx - 1, typename Seq::Rest> {};

template<SomeTypeSeq Seq>
struct cxTypeAt<0, Seq, false> {
    using Type = typename Seq::Head;
};

template<isize Idx, SomeTypeSeq Seq>
struct cxTypeAt<Idx, Seq, true> {
    static_assert(cx__always_false<Seq>, "TypeAt index out of range");
};

template<isize Idx, SomeTypeSeq Seq>
using TypeAt = typename cxTypeAt<Idx, Seq>::Type;

template<isize Idx, typename... Ts>
using TypeAtVa = TypeAt<Idx, TypeSeq<Ts...>>;

////////////////////////////////////////////
// Index of a given type in a type sequence

template<typename T, SomeTypeSeq Seq>
comp fn cx__type_idx_of() -> isize
{
    if constexpr (Seq::empty) {
        static_assert(cx__always_false<T>, "type_idx type not found in TypeSeq");
        return isize{0};
    } else if constexpr (same_as<T, typename Seq::Head>) {
        return isize{0};
    } else {
        return isize{1} + cx__type_idx_of<T, typename Seq::Rest>();
    }
}

template<typename T, SomeTypeSeq Seq>
onedef cons isize type_idx = cx__type_idx_of<T, Seq>();

////////////////////////////////////////////
// Integer sequence

template<SomeIntegral Int, Int... Is>
struct IntegerSeq {
    using Elem = Int;
    onedef glob cons isize size = sizeof...(Is);
};

CX_CONCEPT_GEN_TEMPL(IntegerSeq, is_integer_seq, SomeIntegerSeq,
                     VA_(SomeIntegral Int, Int... Is), VA_(Int, Is...));

template<SomeIntegral Int, isize N, isize... Is>
struct cx__integer_seq : cx__integer_seq<Int, N - 1, N - 1, Is...> {};

template<SomeIntegral Int, isize... Is>
struct cx__integer_seq<Int, isize{0}, Is...> {
    using Type = IntegerSeq<Int, Int{Is}...>;
};

template<SomeIntegral Int, isize N>
using integer_seq = typename cx__integer_seq<Int, N>::Type;

template<SomeIntegral Int, typename... Ts>
using integer_seq_va = integer_seq<Int, isize{sizeof...(Ts)}>;

template<SomeIntegral Int, typename T>
using integer_seq_for = integer_seq<Int, isize{T::size}>;

template<isize... Is>
using IndexSeq = IntegerSeq<isize, Is...>;

template<isize N>
using index_seq = integer_seq<isize, N>;

template<typename... Ts>
using index_seq_va = index_seq<isize{sizeof...(Ts)}>;

template<typename T>
using index_seq_for = index_seq<isize{T::size}>;

}       // namespace uti
}       // namespace cx

#endif  // CX_UTI_TYPESEQ_HH

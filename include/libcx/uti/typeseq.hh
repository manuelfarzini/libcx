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

template<typename... Ts>
onedef cons isize va_size_of = TypeSeq<Ts...>::size;

CX_CONCEPT_GEN_TEMPL(TypeSeq, is_type_seq, SomeTypeSeq, typename... Ts, Ts...);

template<typename>
predicate ___always_false = false;

////////////////////////////////////////////
// Homogeneous type sequence

template<SomeTypeSeq Seq>
cons fn ___typeseq_is_homogeneous() -> bool
{
    if constexpr (Seq::empty || Seq::Rest::empty) {
        return true;
    } else { // XXX: or cvref ???
        return same_as<typename Seq::Head, typename Seq::Rest::Head>
               && ___typeseq_is_homogeneous<typename Seq::Rest>();
    }
}

template<SomeTypeSeq Seq>
predicate is_homogeneous = ___typeseq_is_homogeneous<Seq>();

template<typename... Ts>
predicate va_is_homo = is_homogeneous<TypeSeq<Ts...>>;

////////////////////////////////////////////
// Type at a given index in a type sequence

template<isize Idx, SomeTypeSeq Seq, bool Empty = Seq::empty>
struct ___TypeAt;

template<isize Idx, SomeTypeSeq Seq>
struct ___TypeAt<Idx, Seq, false>
    : ___TypeAt<Idx - 1, typename Seq::Rest> {};

template<SomeTypeSeq Seq>
struct ___TypeAt<0, Seq, false> {
    using Type = typename Seq::Head;
};

template<isize Idx, SomeTypeSeq Seq>
struct ___TypeAt<Idx, Seq, true> {
    static_assert(___always_false<Seq>, "TypeAt index out of range");
};

template<isize Idx, SomeTypeSeq Seq>
using TypeAt = typename ___TypeAt<Idx, Seq>::Type;

template<isize Idx, typename... Ts>
using TypeAtVA = TypeAt<Idx, TypeSeq<Ts...>>;

////////////////////////////////////////////
// Index of a given type in a type sequence

template<typename T, SomeTypeSeq Seq>
comp fn ___type_idx_of() -> isize
{
    if constexpr (Seq::empty) {
        static_assert(___always_false<T>, "type_idx type not found in TypeSeq");
        return isize{0};
    } else if constexpr (same_as<T, typename Seq::Head>) {
        return isize{0};
    } else {
        return isize{1} + ___type_idx_of<T, typename Seq::Rest>();
    }
}

template<typename T, SomeTypeSeq Seq>
onedef cons isize type_idx = ___type_idx_of<T, Seq>();

////////////////////////////////////////////
// Integer sequence

template<SomeIntegral Int, Int... Is>
struct IntegerSeq {
    using Elem = Int;
    glob onedef cons isize size = sizeof...(Is);
};

CX_CONCEPT_GEN_TEMPL(IntegerSeq, is_integer_seq, CIntegerSeq,
                     VA_(SomeIntegral Int, Int... Is), VA_(Int, Is...));

template<SomeIntegral Int, isize N, isize... Is>
struct ___integer_seq : ___integer_seq<Int, N - 1, N - 1, Is...> {};

template<SomeIntegral Int, isize... Is>
struct ___integer_seq<Int, isize{0}, Is...> {
    using Type = IntegerSeq<Int, Int{Is}...>;
};

template<SomeIntegral Int, isize N>
using integer_seq = typename ___integer_seq<Int, N>::Type;

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

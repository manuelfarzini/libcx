/** @file libcx/uti/tuple.hh **/

#ifndef CX_UTI_TUPLE_HH
#define CX_UTI_TUPLE_HH

#include "libcx/uti/ownership.hh"
#include "libcx/uti/sfinae.hh"
#include "libcx/uti/typeseq.hh"

#define CX_TUPLE_TYPE_MEMBERS(First, ...)                    \
    using Elem = First;                                      \
    using Types = TypeSeq<First __VA_OPT__(, ) __VA_ARGS__>; \
    using Self = Tuple<First __VA_OPT__(, ) __VA_ARGS__>

namespace cx {
inline namespace uti {

/**
    A tuple that can hold up to 8 elements.

    @param 
    - `Ts...`: the types of the elements
**/
template<typename... Types> struct Tuple;

template<typename T0>
struct Tuple<T0> {
    CX_TUPLE_TYPE_MEMBERS(T0);
    onedef glob cons isize size = 1;
    T0 e0{};
    cons Tuple() = default;
    cons Tuple(auto&& x0)
        : e0(forward<declt(x0)>(x0)) {}
};

template<typename T0, typename T1>
struct Tuple<T0, T1> {
    CX_TUPLE_TYPE_MEMBERS(T0, T1);
    onedef glob cons isize size = 2;
    T0 e0{}; T1 e1{};
    cons Tuple() = default;
    cons Tuple(auto&& x0, auto&& x1)
        : e0(forward<declt(x0)>(x0)) , e1(forward<declt(x1)>(x1)) {}
};

template<typename T0, typename T1, typename T2>
struct Tuple<T0, T1, T2> {
    CX_TUPLE_TYPE_MEMBERS(T0, T1, T2);
    onedef glob cons isize size = 3;
    T0 e0{}; T1 e1{}; T2 e2{};
    cons Tuple() = default;
    cons Tuple(auto&& x0, auto&& x1, auto&& x2)
        : e0(forward<declt(x0)>(x0)) , e1(forward<declt(x1)>(x1)) , e2(forward<declt(x2)>(x2)) {}
};

template<typename T0, typename T1, typename T2, typename T3>
struct Tuple<T0, T1, T2, T3> {
    CX_TUPLE_TYPE_MEMBERS(T0, T1, T2, T3);
    onedef glob cons isize size = 4;
    T0 e0{}; T1 e1{}; T2 e2{}; T3 e3{};
    cons Tuple() = default;
    cons Tuple(auto&& x0, auto&& x1, auto&& x2, auto&& x3)
        : e0(forward<declt(x0)>(x0)), e1(forward<declt(x1)>(x1)), e2(forward<declt(x2)>(x2))
        , e3(forward<declt(x3)>(x3)) {}
};

template<typename T0, typename T1, typename T2, typename T3, typename T4>
struct Tuple<T0, T1, T2, T3, T4> {
    CX_TUPLE_TYPE_MEMBERS(T0, T1, T2, T3, T4);
    onedef glob cons isize size = 5;
    T0 e0{}; T1 e1{}; T2 e2{}; T3 e3{}; T4 e4{};
    cons Tuple() = default;
    cons Tuple(auto&& x0, auto&& x1, auto&& x2, auto&& x3, auto&& x4)
        : e0(forward<declt(x0)>(x0)) , e1(forward<declt(x1)>(x1)) , e2(forward<declt(x2)>(x2))
        , e3(forward<declt(x3)>(x3)) , e4(forward<declt(x4)>(x4)) {}
};

template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
struct Tuple<T0, T1, T2, T3, T4, T5> {
    CX_TUPLE_TYPE_MEMBERS(T0, T1, T2, T3, T4, T5);
    onedef glob cons isize size = 6;
    T0 e0{}; T1 e1{}; T2 e2{}; T3 e3{}; T4 e4{}; T5 e5{};
    cons Tuple() = default;
    cons Tuple(auto&& x0, auto&& x1, auto&& x2, auto&& x3, auto&& x4, auto&& x5)
        : e0(forward<declt(x0)>(x0)) , e1(forward<declt(x1)>(x1)) , e2(forward<declt(x2)>(x2))
        , e3(forward<declt(x3)>(x3)) , e4(forward<declt(x4)>(x4)) , e5(forward<declt(x5)>(x5)) {}
};

template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct Tuple<T0, T1, T2, T3, T4, T5, T6> {
    CX_TUPLE_TYPE_MEMBERS(T0, T1, T2, T3, T4, T5, T6);
    onedef glob cons isize size = 7;
    T0 e0{}; T1 e1{}; T2 e2{}; T3 e3{}; T4 e4{}; T5 e5{}; T6 e6{};
    cons Tuple() = default;
    cons Tuple(auto&& x0, auto&& x1, auto&& x2, auto&& x3, auto&& x4, auto&& x5, auto&& x6)
        : e0(forward<declt(x0)>(x0)) , e1(forward<declt(x1)>(x1)) , e2(forward<declt(x2)>(x2))
        , e3(forward<declt(x3)>(x3)) , e4(forward<declt(x4)>(x4)) , e5(forward<declt(x5)>(x5))
        , e6(forward<declt(x6)>(x6)) {}
};

template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
struct Tuple<T0, T1, T2, T3, T4, T5, T6, T7> {
    CX_TUPLE_TYPE_MEMBERS(T0, T1, T2, T3, T4, T5, T6, T7);
    onedef glob cons isize size = 8;
    T0 e0{}; T1 e1{}; T2 e2{}; T3 e3{}; T4 e4{}; T5 e5{}; T6 e6{};
    T7 e7{};
    cons Tuple() = default;
    cons Tuple(auto&& x0, auto&& x1, auto&& x2, auto&& x3, auto&& x4, auto&& x5, auto&& x6, auto&& x7)
        : e0(forward<declt(x0)>(x0)) , e1(forward<declt(x1)>(x1)) , e2(forward<declt(x2)>(x2))
        , e3(forward<declt(x3)>(x3)) , e4(forward<declt(x4)>(x4)) , e5(forward<declt(x5)>(x5))
        , e6(forward<declt(x6)>(x6)) , e7(forward<declt(x7)>(x7)) {}
};

template<typename... Us> Tuple(Us...) -> Tuple<Us...>;
template<typename U, typename... Us> Tuple(U, Us...) -> Tuple<U, Us...>;
template<typename U> Tuple(U) -> Tuple<U>;

CX_CONCEPT_GEN_TEMPL(Tuple, is_tuple, SomeTuple, typename... Us, Us...);

// Operations definition.

template<isize idx, SomeTuple Tup req (idx < Tup::size)>
inln cons fn get(Tup& tup) -> auto&
{
    if constexpr (idx == 0) {
        return tup.e0;
    } else if constexpr (idx == 1) {
        return tup.e1;
    } else if constexpr (idx == 2) {
        return tup.e2;
    } else if constexpr (idx == 3) {
        return tup.e3;
    } else if constexpr (idx == 4) {
        return tup.e4;
    } else if constexpr (idx == 5) {
        return tup.e5;
    } else if constexpr (idx == 6) {
        return tup.e6;
    } else if constexpr (idx == 7) {
        return tup.e7;
    }
}

template<isize idx, SomeTuple Tup req (idx < Tup::size)>
inln cons fn get(Tup const& tup) -> auto const&
{
    if constexpr (idx == 0) {
        return tup.e0;
    } else if constexpr (idx == 1) {
        return tup.e1;
    } else if constexpr (idx == 2) {
        return tup.e2;
    } else if constexpr (idx == 3) {
        return tup.e3;
    } else if constexpr (idx == 4) {
        return tup.e4;
    } else if constexpr (idx == 5) {
        return tup.e5;
    } else if constexpr (idx == 6) {
        return tup.e6;
    } else if constexpr (idx == 7) {
        return tup.e7;
    }
}

template<typename U, SomeTuple Tup, typename Us = Tup::Types>
inln cons fn get(Tup& tup) -> auto&
{
    cons isize idx = type_idx<U, Us>;
    return get<idx>(tup);
}

template<typename U, SomeTuple Tup, typename Us = Tup::Types>
inln cons fn get(Tup const& tup) -> auto const&
{
    cons isize idx = type_idx<U, Us>;
    return get<idx>(tup);
}

}       // namespace cx
}       // namespace cx
#endif  // CX_UTI_TUPLE_HH

#if CX_TEST
#ifndef CX_TEST_TUPLE
#define CX_TEST_TUPLE
fn test_tuple_basic() -> void
{
    using namespace cx;
    Tuple tup = {i64(42), 'c', 3.14};
    usize size = tup.size;
    i64 x = get<0>(tup);
    auto c = get<char>(tup);
    cx_unused(size);
    cx_unused(x);
    cx_unused(c);
}
#endif
#endif

// template<typename... Us> where (sizeof...(Us) > 0)
// struct Tuple;
//
// template<typename U, typename... Us> where (sizeof...(Us) > 0)
// struct Tuple<U, Us...> {
//     using Elem = U;
//     using Types = TypeSeq<U, Us...>;
//     using Self = Tuple<U, Us...>;
//     using Rest = Tuple<Us...>;
//
//     onedef glob cons isize size = 1 + sizeof...(Us);
//
//     cons Tuple() = default;
//
//     cons Tuple(auto&& e, auto&&... es)
//         : elem(forward<declt(e)>(e))
//         , rest(forward<declt(es)>(es)...) {}
//
//     Elem elem{};
//     Rest rest{};
// };
//
// template<typename U>
// struct Tuple<U> {
//     using Elem = U;
//     using Types = TypeSeq<U>;
//     using Self = Tuple<U>;
//     using Rest = void;
//
//     onedef glob cons isize size = 1;
//
//     cons Tuple() = default;
//     cons Tuple(auto&& elm) : elem(forward<declt(elm)>(elm)) {}
//
//     Elem elem{};
// };

//
// template<usize idx, SomeTuple Tup req (idx < Tup::size)>
// inln cons fn get(Tup& tup) -> auto&
// {
//     if constexpr (idx == 0) {
//         return tup.elem;
//     } else {
//         return get<idx - 1>(tup.rest);
//     }
// }
//
// template<usize idx, SomeTuple Tup req (idx < Tup::size)>
// inln cons fn get(Tup const& tup) -> auto const&
// {
//     if cons (idx == 0) {
//         return tup.elem;
//     } else {
//         return get<idx - 1>(tup.rest);
//     }
// }
//
// template<typename U, SomeTuple Tup, typename L = Tup::Types>
// inln cons fn get(Tup& tup) -> auto&
// {
//     cons usize idx = type_idx<U, L>;
//     if cons (idx == 0) {
//         return tup.elem;
//     } else {
//         return get<idx - 1>(tup.rest);
//     }
// }
//
// template<typename U, SomeTuple Tup, typename L = Tup::Types>
// inln cons fn get(Tup const& tup) -> auto const&
// {
//     cons usize idx = type_idx<U, L>;
//     if cons (idx == 0) {
//         return tup.elem;
//     } else {
//         return get<idx - 1, typename Tup::Rest>(tup.rest);
//     }
// }

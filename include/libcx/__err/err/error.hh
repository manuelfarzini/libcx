/** @file libcx/err/error.hh **/

// NOTE: seems to work
// TODO: heavy testing with sanitizer.

#ifndef CX_ERR_ERROR_HH
#define CX_ERR_ERROR_HH

#include <libcx/conf.hh>
#include <libcx/mem/common.hh>
#include <libcx/uti/utilities.hh>
#include <libcx/err/macro.hh>
#include <libcx/err/result_pair.hh>

namespace cx::err {

/// Returns the message of the given `err`or.
inln cons fn get_message(IErrorMessage* err) -> char const*
{
    return (char const*) (IErrorMessage*) priv__ptr_untag(err);
}

/// Returns the error type tag of the given `err`or.
inln cons fn get_tag(IErrorMessage* err) -> ErrorTag
{
    return (ErrorTag) (uptr(err) & ErrorTagMask);
}

/// Creates an error message from the given source string literals `srcs`.
template<usize... Ns>
inln comp fn err_create(char const (&... srcs)[Ns]) noexce
    -> ErrorMessage<(Ns + ... + 1) - sizeof...(Ns)>
{
    return {.buf = str::zs_create(srcs...)};
}

/// Creates an error message from the given source string literals `Zs`
/// provided as `cx::err::ZeroString`.
template<str::CZeroString auto... Zs>
inln comp fn err_create() noexce -> ErrorMessage<(1 + ... + Zs.len) - sizeof...(Zs)>
{
    return {.buf = str::zs_create<Zs...>()};
}

/// Allocates memory for the given `err`or, copies that error in the acquired
/// location and tags the pointer to that buffer with the given `Tag`.
/// Returns the tagged pointer.
template<ErrorTag Tag, CErrorMessage auto err>
inln cons fn err_make() noexce -> ErrorPointer
{
    using ErrMsg = rm_cvref<decltype(err)>;
    auto* res = (IErrorMessage*) malloc(sizeof(ErrMsg));
    mem::mem_move(res, &err, sizeof(ErrMsg));
    res = (ErrMsg*) ((uptr) res | (uptr) Tag);
    return ErrorPointer{res};
}

/// Creates a pair with the given `err`or and the given zii value.
template<typename V>
nodisc inln cons fn res_create(V&& val, ErrorPointer err) noexce -> ResultPair<V>
{
    return {cx_forward(val), uti::take(err)};
}

/// Creates a pair with the given `val`ue and no error.
template<typename V> requires (!same_as<rm_cvref<V>, ErrorPointer>)
nodisc inln cons fn res_create(V&& val) noexce -> ResultPair<V>
{
    return {cx_forward(val), ErrorPointer{}};
}

}  // namespace cx::err
#endif  // CX_ERR_ERROR_HH

template<typename V = cx::Empty, typename E = cx::err::ErrorPointer>
using Result = cx::err::ResultPair<V, E>;

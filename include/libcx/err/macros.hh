/** @file libcx/err/error_macro.hh **/

#ifndef CX_ERR_MACRO
#define CX_ERR_MACRO

#include <libcx/conf.hh>
#include <libcx/str/macro.hh>

#define CX__ERR_CONTEXT_RHS(...) ":", STR_(CX_LINE), " ! ", CX_FUNCTION, " ! ", __VA_ARGS__
#define CX__ERR_ZS_CONTEXT_RHS(...) CX_ZS(CX__ERR_CONTEXT_RHS(__VA_ARGS__))
#define CX__ERR_ZS_CONTEXT_FULL(...) CX_ZS_SHORT_FILE, CX__ERR_ZS_CONTEXT_RHS(__VA_ARGS__)

#define cx_res cx::err::ResultPair
#define cx_res_t(T) cx::err::ResultPair<T>

#define CX__ERR_MAKE(tag, ...) cx::err::err_make<tag, cx::err::err_create<__VA_ARGS__>()>()
#define CX__ERR_FWD(...) CX__ERR_ZS_CONTEXT_FULL(__VA_ARGS__)

#define CX__ARG_ERR(...) CX__ERR_MAKE(cx::err::ArgumentError, __VA_ARGS__)
#define cx_arg_err(msg) CX__ARG_ERR(CX__ERR_FWD(msg, " [argument_error]"))
// #define cx_arg_err_t(T, msg) cx_res_t(T){CX__ARG_ERR(CX__ERR_FWD(msg, " [argument_error]"))}

#define CX__OPE_ERR(...) CX__ERR_MAKE(cx::err::OperationError, __VA_ARGS__)
#define cx_ope_err(msg) CX__ARG_ERR(CX__ERR_FWD(msg, " [operation_error]"))

#define CX__FOPEN_ERR(...) CX__ERR_MAKE(cx::err::FileOpenError, __VA_ARGS__)
#define cx_fopen_err(msg) CX__FOPEN_ERR(CX__ERR_FWD(msg, " [file_open_error]"))

#define CX__ALLOC_ERR(...) CX__ERR_MAKE(cx::err::AllocationError, __VA_ARGS__)
#define cx_alloc_err(msg) CX__ALLOC_ERR(CX__ERR_FWD(msg, " [allocation_error]"))

#define CX__NULLPTR_ERR(...) CX__ERR_MAKE(cx::err::NullPtrError, __VA_ARGS__)
#define cx_null_err(msg) CX__NULLPTR_ERR(CX__ERR_FWD(msg, " [nullptr_error]"))

#define cx_between_err(val) cx_between_enum(ErrorTag, val)

#define cx_take_err cx::uti::take(err)

#ifndef try_rethrow_as
  #define CX__TRY_RETHROW_AS(val, _BLK_, R, uniq)               \
    auto [val, CX_JOIN2(err, uniq)] = (_BLK_);                  \
    if (CX_JOIN2(err, uniq)) {                                  \
      return cx_res{R{}, cx::uti::take(CX_JOIN2(err, uniq))}; \
    }
  #define try_rethrow_as(val, _BLK_, R) CX__TRY_RETHROW_AS(val, _BLK_, R, __COUNTER__)
#endif

#ifndef try_rethrow
  #define CX__TRY_RETHROW(val, _BLK_, uniq)                                       \
    auto [val, CX_JOIN2(err, uniq)] = (_BLK_);                                    \
    if (CX_JOIN2(err, uniq)) {                                                    \
      if cons (cx::uti::mv_ctble<plaint(val)>) {                            \
        return cx_res{cx::uti::take(val), cx::uti::take(CX_JOIN2(err, uniq))}; \
      } else {                                                                     \
        return cx_res{val, cx::uti::take(CX_JOIN2(err, uniq))};                 \
      }                                                                            \
    }
  #define try_rethrow(val, _BLK_) CX__TRY_RETHROW(val, _BLK_, __COUNTER__)
#endif

#ifndef try_rethrow_empty
  #define CX__TRY_VALUE(val, _BLK_, uniq)                               \
    auto [val, CX_JOIN2(err, uniq)] = (_BLK_);                          \
    if (CX_JOIN2(err, uniq)) {                                          \
      return cx_res{cx_empty{}, cx::uti::take(CX_JOIN2(err, uniq))}; \
    }
  #define try_rethrow_empty(val, _BLK_) CX__TRY_VALUE(val, _BLK_, __COUNTER__)
#endif

// #include "libcx/err/result_pair.hh"
// fn test_help() -> cx::err::ResultPair<int>
// {
//   return cx::err::ResultPair(42);
// }
// fn test() -> cx::err::ResultPair<>
// {
//   try_rethrow_empty(n, test_help());
//   return {cx_empty{}, null};
// }

// /// If `_BLK_` returns an error, it is be propagated unchanged.
// /// Requires: `_BLK_` is a `ResultPair`.
// /// Note: defines `val` and generates a unique name for `err` 
// ///       while defining it.

/// If `_BLK_` returns an error, it is be propagated unchanged.
/// Requires: `_BLK_` is a `ResultPair`.
/// Note: defines both `val` and `err`.
// #define cx_try_ve(val, err, _BLK_)                             \
//   auto [val, err] = (_BLK_);                                    \
//   if (err) {                                                    \
//     if cons (cx::uti::mv_ctble<plaint(val)>) {           \
//       return cx_res{cx::uti::take(val), cx::uti::take(err)}; \
//     } else {                                                    \
//       return cx_res{val, cx::uti::take(err)};                 \
//     }                                                           \
//   }

/// If `_BLK_` returns an error, it is be propagated unchanged.
/// Requires: `_BLK_` is a `ResultPair`, `val` and `err` are
///           already defined.
// FIX:
// Need a custom `ResultTie` implementation or another mechanism 
// to avoid std::tie.
// The problem is std::tie generates references to pointers.
// 1. `ResultTie` type with specialization for pointers
// 2. `ResultView` view type with custom behaviour.
// #define cx_try(val, err, _BLK_)                                \
//   std::tie(val, err) = (_BLK_);                                 \
//   if (err) {                                                    \
//     if cons (cx::uti::mv_ctble<plaint(val)>) {           \
//       return cx_res{cx::uti::take(val), cx::uti::take(err)}; \
//     } else {                                                    \
//       return cx_res{val, cx::uti::take(err)};                 \
//     }                                                           \
//   }

/// If `_BLK_` returns an error, it is be propagated unchanged.
/// Requires: `_BLK_` is a `ResultPair`, `val` is already defined.
/// Note: defines `err`.
// #define cx_try_e(val, err, _BLK_) \
//   cx::err::ErrorPointer err;      \
//   cx_try(val, err, _BLK_)

/// If `_BLK_` returns an error, it is be propagated unchanged.
/// Requires: `_BLK_` is a `ResultPair`, `err` is already defined.
/// Note: defines `val`.
// #define cx_try_v(V, err, _BLK_) \
//   V val;                         \
//   cx_try(val, err, _BLK_)


#endif  // CX_ERR_MACRO

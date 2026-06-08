/** @file libcx/__raits/qualifier.hh **/

#ifndef CX_TRAITS_QUALIFIER_HH
#define CX_TRAITS_QUALIFIER_HH

#include "libcx/conf/macro.hh"

namespace cx {

// Constant - Volatile

template<typename T> struct ___rm_cv                   { using Type = T; };
template<typename T> struct ___rm_cv<T const>          { using Type = T; };
template<typename T> struct ___rm_cv<T volatile>       { using Type = T; };
template<typename T> struct ___rm_cv<T const volatile> { using Type = T; };
template<typename T> using rm_cv = typename ___rm_cv<T>::Type;

template<typename T, typename U>
struct ___copy_cv { using Type = U; };

template<typename T, typename U>
struct ___copy_cv<const T, U> { using Type = U const; };

template<typename T, typename U>
struct ___copy_cv<volatile T, U> { using Type = U volatile; };

template<typename T, typename U>
struct ___copy_cv<const volatile T, U> { using Type = U const volatile; };

template<typename T, typename U>
using copy_cv = typename ___copy_cv<T, U>::Type;

template<typename T> predicate is_const = false;
template<typename T> predicate is_const<T const> = true;

template<typename T> predicate is_vola = false;
template<typename T> predicate is_vola<T volatile> = true;

template<typename T> predicate is_cv_qual = is_const<T> || is_vola<T>;

// Reference

template<typename T> struct ___rm_ref      { using Type = T; };
template<typename T> struct ___rm_ref<T&>  { using Type = T; };
template<typename T> struct ___rm_ref<T&&> { using Type = T; };
template<typename T> using rm_ref = typename ___rm_ref<T>::Type;

template<typename T> using rm_cvref = rm_cv<rm_ref<T>>;

template<typename T> using lref = T&;
template<typename T> using rref = T&&;

template<typename T> 
predicate ___is_refble = requires { typename lref<T>; };

template<typename T, bool = ___is_refble<T>> 
struct ___add_rref { using Type = T; };

template<typename T> 
struct ___add_rref<T, true> { using Type = rref<T>; };

template<typename T>
using add_rref = typename ___add_rref<T>::Type;

template<typename T, bool = ___is_refble<T>> 
struct ___add_lref { using Type = T; };

template<typename T>
struct ___add_lref<T, true> { using Type = lref<T>; };

template<typename T>
using add_lref = typename ___add_lref<T>::Type;

template<typename T> predicate is_lref = false;
template<typename T> predicate is_lref<T&> = true;
template<typename T> predicate is_rref = false;
template<typename T> predicate is_rref<T&&> = true;
template<typename T> predicate is_ref = is_lref<T> || is_rref<T>;

template<typename T> struct ___mk_lref { using Type = rm_ref<T>&; };
template<typename T> using mk_lref = typename ___mk_lref<T>::Type;

template<typename T> struct ___mk_const_lref { using Type = rm_ref<T> const&; };
template<typename T> using mk_const_lref = typename ___mk_const_lref<T>::Type;

// Pointer

template<typename T> struct ___rm_ptr                    { using Type = T; };
template<typename T> struct ___rm_ptr<T*>                { using Type = T; };
template<typename T> struct ___rm_ptr<T* const>          { using Type = T; };
template<typename T> struct ___rm_ptr<T* volatile>       { using Type = T; };
template<typename T> struct ___rm_ptr<T* const volatile> { using Type = T; };
template<typename T> using rm_ptr = typename ___rm_ptr<T>::Type;

template<typename T> using rm_cvptr = rm_cv<rm_ptr<T>>;

template<typename T> struct ___ad_ptr { using Type = rm_ref<T>*; };
template<typename T> using add_ptr = typename ___ad_ptr<T>::Type;

// Extent

template<typename T>
struct ___rm_extent { using Type = T; };
 
template<typename T>
struct ___rm_extent<T[]> { using Type = T; };
 
template<typename T, usize N>
struct ___rm_extent<T[N]> { using Type = T; };

template<typename T>
using rm_extent = copy_cv<T, typename ___rm_extent<rm_cv<T>>::Type>;

template<typename T>
struct ___rm_all_extent { using Type = T; };

template<typename T>
struct ___rm_all_extent<T[]> { using Type = ___rm_all_extent<T>::Type; };

template<typename T, usize N>
struct ___rm_all_extent<T[N]> { using Type = ___rm_all_extent<T>::Type; };

template<typename T>
using rm_all_extent = copy_cv<T, typename ___rm_all_extent<rm_cv<T>>::Type>;

}       // namespace cx
#endif  // CX_TRAITS_QUALIFIER_HH

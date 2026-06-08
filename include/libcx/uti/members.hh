/** @file libcx/uti/members.hh **/

#ifndef CX_UTI_MEMBERS_HH
#define CX_UTI_MEMBERS_HH

#include "libcx/traits/lifetime.hh"

namespace cx {
inline namespace uti {

template<typename T> struct ___ElemIn { using Type = typename rm_cvref<T>::Elem; };
template<typename T> using ElemIn = typename ___ElemIn<T>::Type;
template<typename T> struct ___elem_in { using Type = typename rm_cvref<T>::Elem; };
#ifndef elem_in
  #define elem_in(v) typename cx::uti::___elem_in<declt(v)>::Type
#endif

template<typename T> struct ___SizeIn { using Type = typename rm_cvref<T>::Size; };
template<typename T> using SizeIn = typename ___SizeIn<T>::Type;
template<typename T> struct ___size_in { using Type = typename rm_cvref<T>::Size; };
#ifndef size_in
  #define size_in(v) typename cx::uti::___size_in<declt(v)>::Type
#endif

template<typename T> struct ___IterIn { using Type = typename rm_cvref<T>::Iter; };
template<typename T> using IterIn = typename ___IterIn<T>::Type;
template<typename T> struct ___iter_in { using Type = typename rm_cvref<T>::Iter; };
#ifndef iter_in
  #define iter_in(v) typename cx::uti::___iter_in<declt(v)>::Type
#endif

template<typename T> struct ___CterIn { using Type = typename rm_cvref<T>::Kter; };
template<typename T> using CterIn = typename ___CterIn<T>::Type;
template<typename T> struct ___cter_in { using Type = typename rm_cvref<T>::Kter; };
#ifndef cter_in
  #define cter_in(v) typename cx::uti::___cter_in<declt(v)>::Type
#endif

template<typename T> struct ___RterIn { using Type = typename rm_cvref<T>::Rter; };
template<typename T> using RterIn = typename ___RterIn<T>::Type;
template<typename T> struct ___rter_in { using Type = typename rm_cvref<T>::Rter; };
#ifndef rter_in
  #define rter_in(v) typename cx::uti::___rter_in<declt(v)>::Type
#endif

template<typename T> struct ___CRterIn { using Type = typename rm_cvref<T>::KRter; };
template<typename T> using CRterIn = typename ___CRterIn<T>::Type;
template<typename T> struct ___crter_in { using Type = typename rm_cvref<T>::KRter; };
#ifndef crter_in
  #define crter_in(v) typename cx::uti::___crter_in<declt(v)>::Type
#endif

template<typename T> struct ___PlainT { using Type = rm_cvref<T>; };
template<typename T> using PlainT = typename ___PlainT<T>::Type;
template<typename T> struct ___plain_t { using Type = rm_cvref<T>; };
#ifndef plaint
  #define plaint(v) typename cx::uti::___plain_t<declt(v)>::Type
#endif

#ifndef selft 
  #define selft(cont) typename cx::uti::rm_cvref<declt(cont)>::Self
#endif

}       // namespace cx
}       // namespace cx
#endif  // CX_UTI_MEMBERS_HH

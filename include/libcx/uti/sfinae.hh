/** @file libcx/uti/sfinae.hh **/

#ifndef CX_UTI_SFINAE_HH
#define CX_UTI_SFINAE_HH

namespace cx {
inline namespace uti {

//−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−
// Enable if
  
template<bool, typename T = void> struct ___enable_if{};
template<typename T> struct ___enable_if<true, T> { using Type = T; };
template<bool B, typename T = void> using enable_if = typename ___enable_if<B, T>::Type;

//−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−
// Conditional

template<bool B, typename True, typename False>
struct ___condition { using Type = False; };

template<typename True, typename False>
struct ___condition<true, True, False> { using Type = True; };

template<bool B, typename True, typename False>
using condition = typename ___condition<B, True, False>::Type;

}       // namespace uti
}       // namespace cx
#endif  // CX_UTI_TRAITS_SFINAE_HH

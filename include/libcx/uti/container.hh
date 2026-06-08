/** @file libcx/__utils/container.hh **/

#ifndef CX_UTILS_CONTAINER_HH
#define CX_UTILS_CONTAINER_HH

#include "libcx/conf.hh"
#include "libcx/traits/types.hh"
#include "libcx/uti/sfinae.hh"
#include "libcx/uti/members.hh"

namespace cx {
inline namespace uti {

////////////////////////////////////////////
// Type erasure

/** Predicate `true` if `C` is a container with raw erased byte storage. **/
template<typename C>
predicate is_erased_container = requires(C c) {
    where (
        (     is_raw_array<rm_cvref<declt(c.storage)>>
              && same_as<rm_extent<rm_cvref<declt(c.storage)>>, u8>  )
        || (  is_raw_ptr<rm_cvref<declt(c.storage)>>
              && same_as<rm_ptr<rm_cvref<declt(c.storage)>>, u8>     )
        || (  is_raw_array<rm_cvref<declt(c.data)>>
              && same_as<rm_extent<rm_cvref<declt(c.data)>>, u8>     )
        || (  is_raw_ptr<rm_cvref<declt(c.data)>>
              && same_as<rm_ptr<rm_cvref<declt(c.data)>>, u8>        )
        || (  is_raw_array<rm_cvref<declt(c.buf)>>
              && same_as<rm_extent<rm_cvref<declt(c.buf)>>, u8>      )
        || (  is_raw_ptr<rm_cvref<declt(c.ptr)>>
              && same_as<rm_ptr<rm_cvref<declt(c.ptr)>>, u8>         )
    );
};

/** Predicate `true` if `T` is a container with a raw `storage`.   **/
template<typename C>
concept SomeErasedContainer = is_erased_container<C>;

/** 
    Returns a pointer to the object at `p` with its lifetime refreshed.
    @ret 
    - A pointer to the object at `p`, with its lifetime refreshed.
    @nota
    - To be used after constructing or re-constructing an object in existing
      storage, so the returned pointer refers to the current live object.
**/
template<typename T>
nodisc cons fn launder(T* p) noexce -> T* where (!is_func<T> && !is_void<T>)
{
    return __builtin_launder(p);
}

/** 
    Returns the raw memory of an `ErasedContainer` `c`.
    @ret
    - A `T` pointer to the raw memory of an `ErasedContainer` `c`
    @nota
     - To be used to construct elements in `c`.
**/
template<SomeErasedContainer C>
inln cons fn get_place(C& c) noexce -> ElemIn<C>*
{
  return c.storage;
}

/** 
    Returns a pointer to a live object stored in an `ErasedContainer` `c`.
    @ret
     - A `T` pointer to a live object stored in an `ErasedContainer` `c`.
    @nota
     - To be used to access a fully constructed object in `c`.
**/
template<SomeErasedContainer C>
inln cons fn get_object(C& c) noexce -> ElemIn<C>*
{
    return launder(get_place<C>(c));
}

////////////////////////////////////////////
// Key type

template<typename T>
concept HasKeyIn = requires { typename T::KeyType; };

template<typename T, typename = void>
struct ___KeyOrSelf { using Type = T; };

template<typename T>
struct ___KeyOrSelf<T, Void<typename T::KeyType>> { using Type = typename T::KeyType; };
/** 
    Extract the key type from a container. If the `ElemType` in the container
    does not have a `KeyType`, it is set as the `ElemType` itself.
    It relies on SFINAE: the `void` template parameter fails substituition if
    the `ElemType` does not have a `KeyType`.
    It is meant to be used as follows.
    ```
        template<typename T> struct Container {
            using ElemType = T;
            using KeyType = cx::KeyOrSelf<T>;
            // ...
        };
    ```
**/
template<typename T>
using KeyOrSelf = typename ___KeyOrSelf<T>::Type;

}       // namespace uti
}       // namespace cx
#endif  // CX_UTILS_CONTAINER_HH

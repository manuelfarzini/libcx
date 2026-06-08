/// \file libcx/err/error_message.hh

#ifndef CX_ERR_ERROR_MESSAGE
#define CX_ERR_ERROR_MESSAGE

#include <libcx/config.hh>
#include <libcx/mem/pointer.hh>
#include <libcx/uti/utilities.hh>
#include <libcx/str/string.hh>

namespace cx::err {

//−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−
// Definitions

enum ErrorTag : uptr;
struct IErrorMessage;
template<usize L> struct ErrorMessage;
struct ErrorPointer;
CX_CONCEPT_GEN(IErrorMessage, is_ierror_message, CIErrorMessage);
CX_CONCEPT_GEN_TEMPL(ErrorMessage, is_error_message, CErrorMessage, usize L, L);
CX_CONCEPT_GEN(ErrorPointer, is_error_pointer, CErrorPointer);
#define IError_Message cx::err::CIErrorMessage auto
#define Error_Message cx::err::CErrorMessage auto
#define Error_Pointer cx::err::CErrorPointer auto
inln onedef cons fn priv__ptr_tag(IErrorMessage* ptr, ErrorTag tag) noexce -> IErrorMessage*;
inln onedef cons fn priv__ptr_untag(IErrorMessage* ptr) noexce -> IErrorMessage*;


//−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−−
// Declarations

/// A tag that specifies the error type.
enum ErrorTag : uptr {
  NoneError = 0,
  ErrorTagBeg = (1UL << 59) - 1,
  ArgumentError = 0b1UL << 59,
  FileOpenError = 0b10UL << 59,
  OperationError = 0b11UL << 59,
  CreationError = 0b100UL << 59,
  AllocationError = 0b101UL << 59,
  NullPtrError = 0b110UL << 59,
  ErrorTagEnd = 0b1111UL << 59,
  ErrorTagMask = ErrorTagEnd,
};
static_assert(ErrorTagMask < 1UL << 63);
static_assert(ErrorTagMask == ErrorTagEnd);

/// An error message interface.
struct IErrorMessage {};

/// An error message with a `buf`fer given by a `ZeroString`.
template<usize L> struct ErrorMessage : IErrorMessage {
  str::ZeroString<L> buf{};
};

/// A pointer to an `ErrorMessage`.
struct ErrorPointer {

  IErrorMessage* ptr{null};

  cons ErrorPointer() = default;

  cons ErrorPointer(IErrorMessage* p) noexce : ptr(p) {}

  cons ErrorPointer(ZeroType) noexce : ptr(null) {}

  /// Move constructor, leaves the given `ErrorPointer` uninitialized.
  cons ErrorPointer(ErrorPointer&& that) noexce : ptr(that.ptr)
  {
    that.ptr = null;
  }

  /// Move assignment operator, leaves the given `ErrorPointer` uninitialized.
  /// If `this` pointer is not `null` the owned memory is released.
  cons ErrorPointer& operator=(this auto& self, ErrorPointer&& that) noexce
  {
    if (uptr(self.ptr) << 5 == uptr(that.ptr) << 5) {
      return self;
    }
    self.~ErrorPointer();
    self.ptr = that.ptr;
    that.ptr = null;
    return self;
  }

  /// Destructor, if `ptr` is not `null` the owned memory is released.
  cons ~ErrorPointer() noexce
  {
    if (ptr) {
      free(err::priv__ptr_untag(ptr));
    }
  }

  /// Boolean conversion, returns `true` if the pointer is not `null`.
  inln cons operator bool() const noexce { return ptr != null; }

  /// Implicit conversion to `IErrorMessage*`.
  inln cons operator IErrorMessage*() const noexce { return ptr; }

  ErrorPointer(ErrorPointer const&) = delete;
  ErrorPointer& operator=(ErrorPointer const&) = delete;
};

/// Tags the given pointer to error message with the given `tag`.
inln onedef cons fn priv__ptr_tag(IErrorMessage* ptr, ErrorTag tag) noexce
  -> IErrorMessage*
{
  return mem::ptr_tag<IErrorMessage*>(ptr, tag);
}

/// Removes the tag from the given pointer to error message.
inln onedef cons fn priv__ptr_untag(IErrorMessage* ptr) noexce
  -> IErrorMessage*
{
  return mem::ptr_untag<IErrorMessage*>(ptr, ErrorTagMask);
}

CX_BASIC_TYPE_ASSERT(err::ErrorMessage<32>);
CX_TRIVIAL_TYPE_ASSERT(err::ErrorMessage<32>);
}       // namespace cx::err
#endif  // CX_ERR_ERROR_MESSAGE

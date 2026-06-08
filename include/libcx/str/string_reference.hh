/** @file libcx/str/string_reference **/
#pragma once
#include <assert.h>
#include <libcx/uti/utilities.hh>
#include <libcx/uti/iterator.hh>

namespace cx::str {

struct StringReference;
CX_CONCEPT_GEN(StringReference, is_string_reference, CStringReference);
#define String_Reference cx::str::CStringReference auto

/// Non-owning reference to non-null terminated char buffer.
/// Maintains a pointer to the underlying char buffer `data` and his `size`.
struct StringReference {
  CX_DEFINE_MEMBER_TYPES(char, usize);
  using Self = StringReference;

  glob cons usize npos = ~usize(0);
  Kter ptr{nullptr};             
  Size len{0};                   

  /// Default constructor.
  cons StringReference() = default;
  /// Disable conversion from nullptr.
  cons StringReference(nullptr_t) = delete;
  /// Construct a string from a cstring.
  cons StringReference(char const* s)
      : ptr(s), len(s ? __builtin_strlen(s) : 0) {}
  /// Construct a string from pointer and size
  cons StringReference(char const* data, usize size) : ptr(data), len(size) {}
  /// Construct a string from a std::string
  cons StringReference(std::string const& s) : ptr(s.data()), len(s.size()) {}
  /// Construct a string refernece from a std::string_view.
  cons StringReference(std::string_view s) : ptr(s.data()), len(s.size()) {}

  /// Get the first character of the string.
  inln cons fn get_front() const noexce -> char
  {
    assert(!is_empty());
    return ptr[0];
  }
  /// Get the last character of the string.
  inln cons fn get_back() const noexce -> char
  {
    assert(!is_empty());
    return ptr[len - 1];
  }
  /// Check if the string is empty.
  inln cons fn is_empty() const noexce -> bool { return len == 0; }

  // Iterators
  inln cons fn begin() const noexce -> Kter { return ptr; }
  inln cons fn end() const noexce -> Kter { return ptr + len; }
  inln cons fn rbegin() const noexce -> KRter { return uti::rev_iterator(end()); }
  inln cons fn rend() const noexce -> KRter { return uti::rev_iterator(begin()); }

  /// Member access operator.
  /// Returns the `char` at the given index in the underlying buffer.
  /// @param index the index.
  inln cons fn operator[](usize index) const noexce -> char
  {
    assert(index < len && "Invalid index");
    return ptr[index];
  }

  /// Convert a `StringReference` to a `std::string_view`
  implicit cons operator std::string_view() const noexce
  {
    return std::string_view(ptr, len);
  }
};

/// Comptime wrapper around a string literal.
/// Used to construct tables of `StringReferences`es with comptime length.
/// Should *only* be used in cons contexts.
struct StringReferenceLiteral : public StringReference {

  template<usize N>
  consteval StringReferenceLiteral(char const (&s)[N]) : StringReference(s, N - 1) {}
};

/// Provides a formatter for String to enable integration with the fmt API.
inln cons fmt::string_view format_as(StringReference const& lit) noexce {
  return {lit.ptr, lit.len};
}

}  // namespace cx::str


CX_BASIC_TYPE_ASSERT(cx::str::StringReference);

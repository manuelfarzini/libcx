/** @file str/string_pack.hh **/

#ifndef CX_STR_STRING_PACK_HH
#define CX_STR_STRING_PACK_HH

#include "libcx/err/error.hh"

namespace cx::str {

template<usize... Lens> struct StringPack;
CX_CONCEPT_GEN_TEMPL(StringPack, is_string_pack, CStringPack, usize... Ls, Ls...);
#define String_Pack cx::str::CStringPack auto

template<usize N> inln priv comp fn
help__pack_copy_advance(char* dst, char const (&src)[N], usize i) -> void;

/// Buffer of packed strings.
/// Owns a character buffer `buf` …
template<usize... Lens> 
struct StringPack {
  // deduced sizes count the null terminators, so we subtract them 
  static_assert(sizeof...(Lens) > 1,
                "A `StringPack` must contain more than one string");
  static_assert(((Lens > 1) && ...), 
                "All strings in the pack must be non-empty");


  /// The total lenght of the character buffer.
  static cons usize len = (Lens + ... + 0) - sizeof...(Lens);

  /// The underlying character buffer.
  char buf[len] = {};

  /// The beginnings of the different strings.
  usize begs[sizeof...(Lens) + 1] = {};


  /// Creates an empty `StringPack`.
  inln comp StringPack() = default;

  ///
  inln comp StringPack(char const (&...srcs)[Lens]) noexce {
    usize i = 0, b = 0;
    ((begs[b++] = i, help__pack_copy_advance(buf, srcs, i)), ...);
    begs[sizeof...(Lens)] = i;
  }

  nodisc inln cons Result_Pair at(u8 const i) noexce { 
    if (i >= sizeof...(Lens)) {
      return Result{std::string_view{""}, cx_ope_err("index out of bounds")};
    }
    return Result{std::string_view{buf + begs[i], begs[i + 1] - begs[i]}, nullptr}; 
  }
};


template<usize N> inln comp auto
help__pack_copy_advance(char* dst, char const (&src)[N], usize i) -> void {
  for (usize j = 0; j < N; j++) {
    dst[i++] = src[j];
  }
}

void test_string_pack() {
  auto sp = StringPack{"Walter", "White"};
  auto [s, err] = sp.at(1);
  printf("%s\n", s.data());
}


/// Provides a formatter for String to enable integration with the fmt API.
inln cons fmt::string_view format_as(String_Pack const& sp) noexce {
  return {sp.buf, sp.len};
}

}       // namespace cx::str
#endif  // CX_STR_STRING_PACK_HH


CX_BASIC_TYPE_ASSERT(decltype(cx::str::StringPack{"Walter", "White"}));

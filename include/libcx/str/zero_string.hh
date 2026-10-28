/** @File libcx/str/zero_string.hh **/
#ifndef CX_STR_ZERO_STRING_HH
#define CX_STR_ZERO_STRING_HH
#include <libcx/uti/utilities.hh>
#include <libcx/uti/iterator.hh>

namespace cx::str {

template<usize N> struct ZeroString;
CX_CONCEPT_GEN_TEMPL(ZeroString, is_zero_string, CZeroString, usize N, N);
#define Zero_String cx::str::CZeroString auto

/** A zero-terminated character `buf`fer with `len`gth equals to `N`. **/
template<usize N> struct ZeroString {
    CX_DEFINE_MEMBER_TYPES(char, usize);
    using Self = ZeroString<N>;

    char buf[N] = {};
    onedef glob cons usize len = N;

    inln cons fn operator[](usize const idx) noexce -> char& { return buf[idx]; }
    inln cons fn operator[](usize const idx) const noexce -> char { return buf[idx]; }

    // inln cons operator std::string_view() const noexce { return {buf, len}; }

    inln cons fn begin() noexce -> Iter { return buf; }
    inln cons fn begin() const noexce -> Kter { return buf; }
    inln cons fn rbegin() noexce -> Rter { return uti::rev_iterator(buf); }
    inln cons fn rbegin() const noexce -> KRter { return uti::rev_iterator(buf); }
    inln cons fn end() noexce -> Iter { return buf + len; }
    inln cons fn end() const noexce -> Kter { return buf; }
};
template<usize N> ZeroString(char const (&)[N]) -> ZeroString<N>;

/// Copy `src` literal of size `N` into `dst` buffer and advance `i`ndex.
template<usize N>
inln comp fn priv__zs_copy_advance(
    CZeroString auto&    dst,
    const char           (&src)[N],
    usize&               i
) noexce -> void {
    for (usize j = 0; j < N - 1; j++) {
        dst[i++] = src[j];
    }
}

/// Creates a `ZeroString` from a list of literals of size `Ns...`.
/// The resulting lenght counts only a single `null` terminator.
template<usize... Ns>
comp fn zs_create(const char (&... srcs)[Ns]) noexce -> CZeroString auto
{
    ZeroString<(1 + ... + Ns) - sizeof...(Ns)> res{};
    usize i = 0;
    ((priv__zs_copy_advance<Ns>(res, srcs, i)), ...);
    res[i] = '\0';
    return res;
}

/// Copy `src` ZeroString into `dst` buffer and advance `i`ndex.
inln comp fn priv__zs_copy_advance(
    CZeroString auto&          dst,
    CZeroString auto const&    src,
    usize&                     i
) noexce -> void {
    for (usize j = 0; j < src.len - 1; j++) {
        dst[i++] = src[j];
    }
}

/// Creates a `ZeroString` from a list of `ZeroStrings`.
template<ZeroString... Zs> comp fn zs_create() noexce -> Zero_String
{
    ZeroString<(1 + ... + Zs.len) - sizeof...(Zs)> res{};
    usize i = 0;
    ((priv__zs_copy_advance(res, Zs, i)), ...);
    res[i] = '\0';
    return res;
}

///
template<ZeroString Z, usize M> requires (M < Z.len)
comp fn zs_create() noexce -> Zero_String
{
    ZeroString<M> res{};
    for (usize i = 0; i < M - 1; i++) {
        res[i] = Z[i];
    }
    res[M - 1] = '\0';
    return res;
}

/// Formatter to integrate ZeroString into the `fmt` API.
// inln cons fn format_as(Zero_String const& z) noexce -> fmt::string_view
// {
//     return {z.buf, z.len};
// }

CX_BASIC_TYPE_ASSERT(ZeroString<8>);
CX_TRIVIAL_TYPE_ASSERT(ZeroString<16>);
static_assert(sizeof(ZeroString<32>) == 32);
static_assert(sizeof(ZeroString{"Heisenberg"}) == 11);
static_assert(sizeof(zs_create("Heisenberg")) == 11);
static_assert(sizeof(zs_create<ZeroString{"Heisenberg"}>()) == 11);
// static_assert(cx::CStringLike<cx::str::ZeroString<16>>);

// #define CX_TEST 1
#ifdef CX_TEST
fn test_zero_string -> void()
{
    using CX;
    Zero_String z = zs_create("Heisenberg");
    auto* it = uti::beg(z);
}
#endif  // CX_TEST
}       // namespace cx::str
#endif  // CX_STR_ZERO_STRING_HH

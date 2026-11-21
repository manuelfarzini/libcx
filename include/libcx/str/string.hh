/** @file libcx/str/string.hh **/
#ifndef CX_STR_STRING_HH
#define CX_STR_STRING_HH
#include <libcx/str/zero_string.hh>

namespace cx {
inline namespace str {

///
template<ZeroString Z>
comp fn priv__zs_short_fpath_len() noexce -> usize
{
    cons auto& Src = Z.buf;
    cons usize N = Z.len;
    usize count = 0;
    for (usize i = N - 1; i > 0; i--) {
        if (Src[i] == '/') {
            count++;
            if (count == 4) {
                return N - i;
            }
        }
    }
    return N;
}

///
template<ZeroString Z, usize M = priv__zs_short_fpath_len<Z>()>
comp fn zs_short_fpath() noexce -> ZeroString<M> with (M <= Z.len)
{
    ZeroString<M> res{};
    usize i = 0;
    for (usize j = Z.len - M; j < Z.len - 1; j++) {
        res[i++] = Z[j];
    }
    res[i] = '\0';
    return res;
}

//  comp fn priv__zs_short_fpath(
//      $Z: ZeroString,
//      $M := priv__zs_short_fpath_len(Z),
//  where:
//      M: Leq(Z.len)
//  ) -> (res: ZeroString(M)) {
//      i := 0
//      for j := Z.len - M; j < Z.len - 1; j++ {
//          res[i++] = Z[j]
//      }
//      res[i] = null
//  }

}       // namespace str
}       // namespace cx
#endif  // CX_STR_STRING_HH

/** @file libcx/io.hh **/

#ifndef CX_IO_IO_HH
#define CX_IO_IO_HH

#include <libcx/conf.hh>
#include <libcx/uti/utilities.hh>
#include <libcx/err/error.hh>

namespace cx::io {

template<typename... Ts>
inln onedef cons fn printf(fmt::format_string<Ts...> fmts, Ts&&... args) noexce -> void
{
    fast_io::io::print(fmt::format(fmts, std::forward<Ts>(args)...));
}

template<typename... Ts>
inln onedef cons fn printfn(fmt::format_string<Ts...> fmts, Ts&&... args) noexce
    -> void
{
    fast_io::io::println(fmt::format(fmts, std::forward<Ts>(args)...));
}

template<typename... Ts> inln onedef cons fn print(Ts&&... args) noexce -> void
{
    fast_io::io::print(args...);
}

template<typename... Ts> inln onedef cons fn printn(Ts&&... args) noexce -> void
{
    fast_io::io::println(args...);
}

template<typename... Ts>
inln onedef cons fn fprintf(
    fast_io::obuf_file obuf, fmt::format_string<Ts...> fmts, Ts&&... args
) noexce -> void {
    fast_io::io::print(obuf, fmt::format(fmts, std::forward<Ts>(args)...));
}

template<typename... Ts>
inln onedef cons fn
fprintfn(FILE* ofile, fmt::format_string<Ts...> fmts, Ts&&... args) noexce -> void
{
    fast_io::io::println(ofile, fmt::format(fmts, std::forward<Ts>(args)...));
}

inln onedef cons fn chview(usize num) noexce -> auto
{
    return fast_io::mnp::chvw(num);
}

template<
    std::ranges::contiguous_range Rng,
    Requires(std::integral<std::ranges::range_value_t<Rng>>)>
inln onedef cons fn strview(Rng&& r) noexce -> auto
{
    return fast_io::mnp::strvw(r);
}

nodisc onedef cons fn fopen(char const* fpath, char const* mode) noexce
    -> Result<fast_io::c_file>
{
    FILE* f = std::fopen(fpath, mode);
    if (f == null) {
        return {fast_io::c_file{}, cx_fopen_err("Failed to open file")};
    }
    defer(std::fclose(f));
    return {fast_io::c_file{f}, null};
}

}  // namespace cx::io
#endif  // CX_IO_IO_HH

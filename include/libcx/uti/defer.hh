/** @file libcx/uti/defer.hh **/

#ifndef CX_UTI_DEFER_HH
#define CX_UTI_DEFER_HH

#include "libcx/config.hh"
#include "libcx/uti/ownership.hh"

namespace cx {
inline namespace uti {

template<typename F>
struct DeferWrapper
{
    F f;
    inln cons DeferWrapper(F&& f) : f(forward<F>(f)) {}
    inln cons ~DeferWrapper() { f(); };
};

template<typename F>
inln cons fn defer_wrapper(F&& f) noexce -> DeferWrapper<F>
{
  return DeferWrapper<F>(forward<F>(f));
}

#ifndef defer
  #define defer(_BODY_) \
    fn CX_UNIQ(_defer_) = cx::uti::defer_wrapper([&]() inln_clos { _BODY_; })
#endif


#ifndef cx_defer_loop
#define cx_defer_loop(_cx_dl_init_, _cx_dl_clean_)           \
  for (u8 _cx_dl_stop_ = 0, _cx_dl_once_ = 1;                \
       _cx_dl_once_;                                         \
       CX_EMPTY)                                             \
    for (_cx_dl_init_;                                       \
         !_cx_dl_stop_;                                      \
         _cx_dl_stop_ ? (_cx_dl_clean_, _cx_dl_once_ = 0, 1) \
                        : (_cx_dl_once_ = 0, 0))
#endif

#ifndef cx_defer_loop_stop
#define cx_defer_loop_stop() _cx_dl_stop_ = 1
#endif

}       // namespace cx
}       // namespace uti
#endif  // CX_UTI_DEFER_HH

/*

Example
 
fn test() -> void
{
    cx_defer_loop (auto x = 0, printf("cleanup\n")) {
        x += 1;
        printf("inside");
        if (x == 3) {
            cx_defer_loop_stop();
        }
    }
}

fn test() -> void
{
    cx_defer_loop (FILE* f = fopen("test.txt", "r"), fclose(f)) {
        auto c = fgetc(f);
        printf("inside");
        if (c == '#') {
            cx_defer_loop_stop();
        }
    }
}

*/

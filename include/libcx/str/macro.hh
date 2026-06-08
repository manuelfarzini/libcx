/** @file libcx/str/macro.hh **/

#ifndef CX_STR_MACRO_HH
#define CX_STR_MACRO_HH

#include <libcx/conf/macro.hh>

#ifndef CX_ZS_FILE
    #define CX_ZS(...) cx::str::zs_create(__VA_ARGS__)
#endif
#ifndef CX_ZS_FILE
    #define CX_ZS_FILE CX_ZS(CX_FILE)
#endif
#ifndef CX_ZS_SHORT_FILE
    #define CX_ZS_SHORT_FILE cx::str::zs_short_fpath<CX_ZS_FILE>()
#endif
#ifndef CX_ZS_LINE
    #define CX_ZS_LINE CX_ZS(STR_(CX_LINE))
#endif
#ifndef CX_ZS_FUNCTION
    #define CX_ZS_FUNCTION CX_ZS(CX_FUNCTION)
#endif

#endif  // CX_STR_MACRO_HH

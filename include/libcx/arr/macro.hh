/** @file libcx/arr/macro.hh **/

#ifndef CX_ARR_MACRO_HH
#define CX_ARR_MACRO_HH

#include "libcx/conf/macro.hh"

// Macros for multi arrays

#ifndef CX__MULTI_ARRAY_MACROS
    #define CX__MULTI_ARRAY_MACROS 1
    
    // Multi array definition and declaration

    #define CX__MULTI_ARR_APPLY(M, args) M args

    #define CX__MULTI_ROW_TYPE_NAME(Name, pair) \
        CX__MULTI_ARR_APPLY(CX__MULTI_ROW_TYPE_NAME_, (Name, VA_ pair))
    
    #define CX__MULTI_ROW_TYPE_NAME_(Name, T, name) CX_JOIN4(Name, _, name, _t)
    
    #define CX__MULTI_DECLARE_ROW_TYPE(Name, pair) \
        CX__MULTI_ARR_APPLY(CX__MULTI_DECLARE_ROW_TYPE_, (Name, VA_ pair))
    
    #define CX__MULTI_DECLARE_ROW_TYPE_(Name, T, name) \
        using CX__MULTI_ROW_TYPE_NAME_(Name, T, name) = T;
    
    #define CX__MULTI_TEMPLATE_ROW_TYPE(Name, pair) , CX__MULTI_ROW_TYPE_NAME(Name, pair)
    
    #define CX__MULTI_ROW_ENUM(Name, pair) \
        CX__MULTI_ARR_APPLY(CX__MULTI_ROW_ENUM_, (Name, VA_ pair))
    
    #define CX__MULTI_ROW_ENUM_(Name, T, name) CX_JOIN3(Name, _, name),
    
    #define CX_DEFINE_MULTI_ARRAY(Name, Alc, ...)                                     \
        CX_FOR_EACH_WITH_ARG( CX__MULTI_DECLARE_ROW_TYPE, Name, __VA_ARGS__)          \
        using CX_JOIN2(Name, s_t) = MultiArray<                                       \
            Alc CX_FOR_EACH_WITH_ARG(CX__MULTI_TEMPLATE_ROW_TYPE, Name, __VA_ARGS__ ) \
        >;                                                                            \
        enum { CX_FOR_EACH_WITH_ARG(CX__MULTI_ROW_ENUM, Name, __VA_ARGS__) };

    #define CX_DECLARE_MULTI_ARRAY(Name, Alc, ...)    \
        CX_DEFINE_MULTI_ARRAY(Name, Alc, __VA_ARGS__) \
        CX_JOIN2(Name, s_t) CX_JOIN2(Name, s){};

    // Example
    // CX_DECLARE_MULTI_ARRAY(position, A, f64, x, f64, y, f64, z);
    //  =>
    //      using position_x_t = f64;
    //      using position_y_t = f64;
    //      using position_z_t = f64;
    //      using positions_t = MultiArray<A, position_x_t, position_y_t, position_z_t>;
    //      enum {position_x, position_y, position_z}
    //      positions_t positions{};

    // Multi array operations

    #define types_in(arr) cx::rm_cvref<declt(arr)>::Types

    #define base_ptr(arr) mutaptr(get<0>((arr).ptrs))

#endif

#endif  // CX_ARR_MACRO_HH

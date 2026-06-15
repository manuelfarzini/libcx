/** @type conf/macro.hh **/

#ifndef CX_CONF_MACRO
#define CX_CONF_MACRO

#include "libcx/conf/hal.hh"

////////////////////////////////////////////
// Syntax

#ifndef or_return
    #define or_return ; if (err) return
#endif
#ifndef or_else
    #define or_else ; if (err)
#endif
#ifndef or_continue
    #define or_continue ; if (err) continue
#endif
#ifndef or_break
    #define or_break ; if (err) break
#endif
#ifndef or_goto
    #define or_goto ; if (err) goto
#endif

#ifndef nodisc
    #define nodisc [[nodiscard]]
#endif
#ifndef noret
    #define noret [[noreturn]]
#endif
#ifndef like
    #define like [[likely]]
#endif
#ifndef unlike
    #define unlike [[unlikely]]
#endif
#ifndef cx_assume
    #define cx_assume(...) [[assume(__VA_ARGS__)]]
#endif
#ifndef cx_no_uniq_addr
    #if CX_COMPILER_CLANG || CX_COMPILER_GCC
        #define cx_no_uniq_addr [[no_unique_address]]
    #else
        #define cx_no_uniq_addr [[msvc::no_unique_address]]
    #endif
#endif

#ifndef priv  // internal linkage procedures/variables
    #define priv static
#endif
#ifndef glob
    #define glob static
#endif
#ifndef persist  // persistent local variables
    #define persist static
#endif
#ifndef onedef  // single procedure/variable across translation units
    #define onedef inline
#endif

#ifndef fn
    #define fn inline auto
#endif
#ifndef noexce
    #define noexce noexcept
#endif
#ifndef where
    #define where requires
#endif
#ifndef proc
    #define proc(_name_, ...) inline auto _name_(__VA_ARGS__) noexcept ->
#endif
#ifndef func
    #define func auto
#endif
#ifndef clos
    #define clos auto
#endif
#ifndef comp
    #define comp consteval
#endif
#ifndef cons
    #define cons constexpr
#endif
#ifndef predicate
    #define predicate inline constexpr bool
#endif

#ifndef cast
    #define cast(Type, v) ((Type) (v))
#endif
#ifndef nil
    #define nil (void*) nullptr
#endif

#ifndef cx_unused
    #define cx_unused(x) (void) x
#endif

#ifndef declt
    #define declt(x) decltype(x)
#endif

#ifndef Requires
    #define Requires(...)                                                                \
        typename = cx::enable_if<true> > requires (__VA_ARGS__) && cx::kpred < true
#endif
#ifndef req
    #define req(...) , typename = cx::uti::enable_if<(__VA_ARGS__)>
#endif

////////////////////////////////////////////
// Attributes and decorators

#ifndef CX_HIDE_FROM_ABI
    #if CX_COMPILER_MSVC
        #define CX_HIDE_FROM_ABI
    #else
        #define CX_HIDE_FROM_ABI __attribute__((__visibility__("hidden")))
    #endif
#endif

#ifndef inln
    #if CX_COMPILER_MSVC
        #if _MSC_VER < 1300
            #define inln
        #else
            #define inln __forceinline
        #endif
    #elif CX_COMPILER_CLANG
        #define inln [[clang::always_inline]] __attribute__((always_inline))
    #elif CX_COMPILER_GCC
        #define inln [[gnu::always_inline]] __attribute__((always_inline))
    #else
        #define inln
    #endif
#endif
#ifndef noinln
    #if CX_COMPILER_MSVC
        #define noinln __declspec(noinline)
    #elif CX_COMPILER_CLANG || CX_COMPILER_GCC
        #define noinln __attribute__((noinline))
    #else
        #define noinln
    #endif
#endif
#ifndef inln_clos
    #if CX_COMPILER_GCC || CX_COMPILER_CLANG
        #define inln_clos __attribute__((always_inline, hot))
    #else
        #define inln_clos
    #endif
#endif
#ifndef noinln_clos
    #if CX_COMPILER_GCC || CX_COMPILER_CLANG
        #define noinln_clos __attribute__((noinline, cold))
    #else
        #define noinln_clos
    #endif
#endif

#ifndef cx_restrict
    #if CX_COMPILER_GCC or CX_COMPILER_CLANG
        #define cx_restrict __restrict__
    #else
        #define cx_restrict __restrict
    #endif
#endif

#if CX_COMPILER_CLANG
    #ifndef cx_non_null
        #define cx_non_null _Nonnull
    #endif
    #ifndef cx_nullable
        #define cx_nullable _Nullable
    #endif
    #ifndef cx_null_unspec
        #define cx_null_unspec _Null_unspecified
    #endif
#else
    #ifndef cx_non_null
        #define cx_non_null
    #endif
    #ifndef cx_nullable
        #define cx_nullable
    #endif
    #ifndef cx_null_unspec
        #define cx_null_unspec
    #endif
#endif


#ifndef CX_PRINTF_ARGS
    #if CX_COMPILER_GCC or CX_COMPILER_CLANG
        #define CX_PRINTF_ARGS(fmt) __attribute__((format(printf, fmt, (fmt + 1))))
    #else
        #define CX_PRINTF_ARGS(fmt)
    #endif
#endif

////////////////////////////////////////////
// Text manipulation

#ifndef LIBCX
    #define LIBCX namespace cx
#endif
#ifndef NS_
    #define NS_(name) name::
#endif
#ifndef STR_
    #define CX__STR_(x) #x
    #define STR_(x) CX__STR_(x)
#endif
#ifndef VA_
    #define VA_(...) __VA_ARGS__
#endif

#ifndef CX_LINE
    #define CX_LINE __LINE__
#endif
#ifndef CX_FILE
    #define CX_FILE __FILE__
#endif
#ifndef CX_FUNCTION
    #if CX_COMPILER_CLANG || CX_COMPILER_GCC
        #define CX_FUNCTION __PRETTY_FUNCTION__
    #elif CX_COMPILER_MSVC
        #define CX_FUNCTION __FUNCSIG__
    #else
        #define CX_FUNCTION __func__
    #endif
#endif

#ifndef CX_JOIN
    #define CX_JOIN 1
    #define CX__JOIN2(a, b) a##b
    #define CX_JOIN2(a, b) CX__JOIN2(a, b)
    #define CX_JOIN3(a, b, c) CX_JOIN2(CX_JOIN2(a, b), c)
    #define CX_JOIN4(a, b, c, d) CX_JOIN2(CX_JOIN2(CX_JOIN2(a, b), c), d)
#endif

#ifndef CX_UNIQ
    #define CX__UNIQ(a, b) CX_JOIN2(a, b)
    #define CX_UNIQ(base) CX__UNIQ(base, __COUNTER__)
#endif

////////////////////////////////////////////
// Function like macros

// Signed memory operators.

#ifndef size_of
    #define size_of(...) (isize(sizeof(__VA_ARGS__)))
#endif
#ifndef offset_of
    #define offset_of(T, elem) (cast(isize) & ((cast(T*) 0)->elem))
#endif
#ifndef align_of
    #define align_of(X) isize(alignof(X))
#endif
#ifndef align_as
    #define align_as(X) alignas(X)
#endif
#ifndef count_of
    #define count_of(x) \
        ((size_of(x) / size_of(0 [x])) / (cast(isize, not (size_of(x) % size_of(0 [x])))))
#endif

#ifndef cx_between
    #define cx_between(lower, val, upper) lower < val&& val < upper
#endif
#ifndef cx_between_enum
    #define cx_between_enum(EnumName, val)                                   \
        cx_between(usize(EnumName_##Beg), isize(val), isize(EnumName_##End))
#endif
#ifndef cx_between_enum_ns
    #define cx_between_enum_ns(EnumName, val)                                      \
        cx_between(isize(NS_(EnumName) Beg), isize(val), isize(NS_(EnumName) End))
#endif

#ifndef cx_max2
    #define cx_max2(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef cx_max3
    #define cx_max3(a, b, c) cx_max2(cx_max2(a, b), c)
#endif
#ifndef cx_max4
    #define cx_max4(a, b, c, d) cx_max2(cx_max2(a, b), cx_max2(c, d))
#endif

#ifndef cx_min2
    #define cx_min2(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef cx_min3
    #define cx_min3(a, b, c) cx_min2(cx_min2(a, b), c)
#endif
#ifndef cx_min4
    #define cx_min4(a, b, c, d) cx_min2(cx_min2(a, b), cx_min2(c, d))
#endif

#ifndef cx_splat_8_16
    #define cx_splat_8_32(v8) ((u32(-1) / 255) * u8(v8))
#endif
#ifndef cx_splat_8_64
    #define cx_splat_8_64(v8) ((u64(-1) / 255) * u8(v8))
#endif
#ifndef cx_splat_32_64
    #define cx_splat_32_64(v32) ((u64(v32) << 32) | u32(v32))
#endif

#ifndef cx_mask_set
    #define cx_mask_set(var, set, mask) \
        if (set) {                      \
            (var) |= (mask);            \
        } else {                        \
            (var) &= ~(mask);           \
        }
#endif

// Array operations

#ifndef cx_extract4
    #define cx_extract4(src) {src[0], src[1], src[2], src[3]}
#endif
#ifndef cx_extract4_z
    #define cx_extract4_z(src) {src[0], src[1], src[2], src[3], 0}
#endif
#ifndef cx_extract4_rev
    #define cx_extract4_rev(src) {src[3], src[2], src[1], src[0]}
#endif
#ifndef cx_extract4_revz
    #define cx_extract4_revz(src) {src[3], src[2], src[1], src[0], 0}
#endif
#ifndef cx_print4_ul
    #define cx_print4_ul(src) printf("%ul %ul %ul %ul\n", src[0], src[1], src[2], src[3])
#endif
#ifndef cx_extract16
    #define cx_extract16(arr)                                                  \
        {arr[0], arr[1], arr[2],  arr[3],  arr[4],  arr[5],  arr[6],  arr[7],  \
         arr[8], arr[9], arr[10], arr[11], arr[12], arr[13], arr[14], arr[15]}
#endif

// Probability branch.

#ifndef CX_PROBABILITY_BRANCH
    #define CX_PROBABILITY_BRANCH
    #define cx_rare(_BLK_)                                    \
        [[unlikely]] { [&] noinln_clos { _BLK_; }(); }
    #define cx_rare_if(b, _BLK_)                              \
        if (b) [[unlikely]] { [&] noinln_clos { _BLK_; }(); }
    #define cx_often(_BLK_)                                   \
        [[likely]] { [&] inln_clos { _BLK_; }(); }
    #define cx_often_if(b, _BLK_)                             \
        if (b) [[likely]] { [&] inln_clos { _BLK_; }(); }
#endif

////////////////////////////////////////////
// Testing

#ifndef CX_TEST_DEFINE
    #define CX_TEST_DEFINE(name) inline auto test_##name() noexcept -> void
#endif

#ifndef CX_TEST_RUN
    #define CX_TEST_RUN(name)                                                                         \
        puts("\n////////////////////////////////////////////////////////////////////////////////\n"); \
        cx::test_##name();
#endif

#ifndef CX_TEST_CASE
    #define CX_TEST_CASE(name)                                                                        \
        puts(">>>>>>> running test `" STR_(name)"`");                                                 \
        cx::test_##name();                                                                            \
        puts(">>>>>>> test `" STR_(name) "` passed");                                                 \
        puts("\n////////////////////////////////////////////////////////////////////////////////\n");
#endif

////////////////////////////////////////////
// Foreach macros

#ifndef CX__EXPAND
    #define CX__EXPAND(x) x
#endif

#ifndef CX__COUNT_ARGS
    #define CX__COUNT_ARGS(                                                    \
        _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
        _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30,  \
        _31, _32, N, ...)                                                      \
        N
#endif

#define CX__COUNT_ARGS_(...)                                            \
    CX__EXPAND(CX__COUNT_ARGS(                                          \
        __VA_ARGS__,                                                    \
        32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, \
        16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1           \
    ))
#endif

#ifndef FOR__EACH_MACROS
    // Foreach base macro.
    #define FOR__EACH_MACROS 1
    #define CX__FOR_EACH_1(M, a) M(a)
    #define CX__FOR_EACH_2(M, a, ...) M(a) CX__FOR_EACH_1(M, __VA_ARGS__)
    #define CX__FOR_EACH_3(M, a, ...) M(a) CX__FOR_EACH_2(M, __VA_ARGS__)
    #define CX__FOR_EACH_4(M, a, ...) M(a) CX__FOR_EACH_3(M, __VA_ARGS__)
    #define CX__FOR_EACH_5(M, a, ...) M(a) CX__FOR_EACH_4(M, __VA_ARGS__)
    #define CX__FOR_EACH_6(M, a, ...) M(a) CX__FOR_EACH_5(M, __VA_ARGS__)
    #define CX__FOR_EACH_7(M, a, ...) M(a) CX__FOR_EACH_6(M, __VA_ARGS__)
    #define CX__FOR_EACH_8(M, a, ...) M(a) CX__FOR_EACH_7(M, __VA_ARGS__)
    #define CX__FOR_EACH_9(M, a, ...) M(a) CX__FOR_EACH_8(M, __VA_ARGS__)
    #define CX__FOR_EACH_10(M, a, ...) M(a) CX__FOR_EACH_9(M, __VA_ARGS__)
    #define CX__FOR_EACH_11(M, a, ...) M(a) CX__FOR_EACH_10(M, __VA_ARGS__)
    #define CX__FOR_EACH_12(M, a, ...) M(a) CX__FOR_EACH_11(M, __VA_ARGS__)
    #define CX__FOR_EACH_13(M, a, ...) M(a) CX__FOR_EACH_12(M, __VA_ARGS__)
    #define CX__FOR_EACH_14(M, a, ...) M(a) CX__FOR_EACH_13(M, __VA_ARGS__)
    #define CX__FOR_EACH_15(M, a, ...) M(a) CX__FOR_EACH_14(M, __VA_ARGS__)
    #define CX__FOR_EACH_16(M, a, ...) M(a) CX__FOR_EACH_15(M, __VA_ARGS__)
    #define CX__FOR_EACH_17(M, a, ...) M(a) CX__FOR_EACH_16(M, __VA_ARGS__)
    #define CX__FOR_EACH_18(M, a, ...) M(a) CX__FOR_EACH_17(M, __VA_ARGS__)
    #define CX__FOR_EACH_19(M, a, ...) M(a) CX__FOR_EACH_18(M, __VA_ARGS__)
    #define CX__FOR_EACH_20(M, a, ...) M(a) CX__FOR_EACH_19(M, __VA_ARGS__)
    #define CX__FOR_EACH_21(M, a, ...) M(a) CX__FOR_EACH_20(M, __VA_ARGS__)
    #define CX__FOR_EACH_22(M, a, ...) M(a) CX__FOR_EACH_21(M, __VA_ARGS__)
    #define CX__FOR_EACH_23(M, a, ...) M(a) CX__FOR_EACH_22(M, __VA_ARGS__)
    #define CX__FOR_EACH_24(M, a, ...) M(a) CX__FOR_EACH_23(M, __VA_ARGS__)
    #define CX__FOR_EACH_25(M, a, ...) M(a) CX__FOR_EACH_24(M, __VA_ARGS__)
    #define CX__FOR_EACH_26(M, a, ...) M(a) CX__FOR_EACH_25(M, __VA_ARGS__)
    #define CX__FOR_EACH_27(M, a, ...) M(a) CX__FOR_EACH_26(M, __VA_ARGS__)
    #define CX__FOR_EACH_28(M, a, ...) M(a) CX__FOR_EACH_27(M, __VA_ARGS__)
    #define CX__FOR_EACH_29(M, a, ...) M(a) CX__FOR_EACH_28(M, __VA_ARGS__)
    #define CX__FOR_EACH_30(M, a, ...) M(a) CX__FOR_EACH_29(M, __VA_ARGS__)
    #define CX__FOR_EACH_31(M, a, ...) M(a) CX__FOR_EACH_30(M, __VA_ARGS__)
    #define CX__FOR_EACH_32(M, a, ...) M(a) CX__FOR_EACH_31(M, __VA_ARGS__)

    #define CX__FOR_EACH_DISPATCH(N) CX_JOIN2(CX_FOR_EACH_, N)

    #define CX_FOR_EACH(M, ...)                                                    \
        CX__EXPAND(CX__FOR_EACH_DISPATCH(CX__COUNT_ARGS_(__VA_ARGS__))(M, __VA_ARGS__))

    // Foreach with arg macro.

    #define CX__FOR_EACH_WITH_ARG_1(M, arg, a) M(arg, a)
    #define CX__FOR_EACH_WITH_ARG_2(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_1(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_3(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_2(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_4(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_3(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_5(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_4(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_6(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_5(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_7(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_6(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_8(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_7(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_9(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_8(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_10(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_9(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_11(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_10(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_12(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_11(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_13(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_12(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_14(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_13(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_15(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_14(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_16(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_15(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_17(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_16(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_18(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_17(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_19(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_18(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_20(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_19(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_21(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_20(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_22(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_21(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_23(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_22(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_24(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_23(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_25(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_24(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_26(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_25(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_27(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_26(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_28(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_27(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_29(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_28(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_30(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_29(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_31(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_30(M, arg, __VA_ARGS__)
    #define CX__FOR_EACH_WITH_ARG_32(M, arg, a, ...) M(arg, a) CX__FOR_EACH_WITH_ARG_31(M, arg, __VA_ARGS__)

    #define CX__FOR_EACH_WITH_ARG_DISPATCH(N) CX_JOIN2(CX__FOR_EACH_WITH_ARG_, N)

    #define CX_FOR_EACH_WITH_ARG(M, arg, ...) \
        CX__EXPAND(CX__FOR_EACH_WITH_ARG_DISPATCH(CX__COUNT_ARGS_(__VA_ARGS__))(M, arg, __VA_ARGS__))

    // Foreach with comma macro.

    #define CX__FOR_EACH_COMMA_1(M, a) M(a)
    #define CX__FOR_EACH_COMMA_2(M, a, ...) M(a), CX__FOR_EACH_COMMA_1(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_3(M, a, ...) M(a), CX__FOR_EACH_COMMA_2(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_4(M, a, ...) M(a), CX__FOR_EACH_COMMA_3(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_5(M, a, ...) M(a), CX__FOR_EACH_COMMA_4(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_6(M, a, ...) M(a), CX__FOR_EACH_COMMA_5(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_7(M, a, ...) M(a), CX__FOR_EACH_COMMA_6(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_8(M, a, ...) M(a), CX__FOR_EACH_COMMA_7(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_9(M, a, ...) M(a), CX__FOR_EACH_COMMA_8(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_10(M, a, ...) M(a), CX__FOR_EACH_COMMA_9(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_11(M, a, ...) M(a), CX__FOR_EACH_COMMA_10(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_12(M, a, ...) M(a), CX__FOR_EACH_COMMA_11(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_13(M, a, ...) M(a), CX__FOR_EACH_COMMA_12(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_14(M, a, ...) M(a), CX__FOR_EACH_COMMA_13(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_15(M, a, ...) M(a), CX__FOR_EACH_COMMA_14(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_16(M, a, ...) M(a), CX__FOR_EACH_COMMA_15(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_17(M, a, ...) M(a), CX__FOR_EACH_COMMA_16(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_18(M, a, ...) M(a), CX__FOR_EACH_COMMA_17(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_19(M, a, ...) M(a), CX__FOR_EACH_COMMA_18(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_20(M, a, ...) M(a), CX__FOR_EACH_COMMA_19(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_21(M, a, ...) M(a), CX__FOR_EACH_COMMA_20(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_22(M, a, ...) M(a), CX__FOR_EACH_COMMA_21(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_23(M, a, ...) M(a), CX__FOR_EACH_COMMA_22(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_24(M, a, ...) M(a), CX__FOR_EACH_COMMA_23(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_25(M, a, ...) M(a), CX__FOR_EACH_COMMA_24(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_26(M, a, ...) M(a), CX__FOR_EACH_COMMA_25(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_27(M, a, ...) M(a), CX__FOR_EACH_COMMA_26(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_28(M, a, ...) M(a), CX__FOR_EACH_COMMA_27(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_29(M, a, ...) M(a), CX__FOR_EACH_COMMA_28(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_30(M, a, ...) M(a), CX__FOR_EACH_COMMA_29(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_31(M, a, ...) M(a), CX__FOR_EACH_COMMA_30(M, __VA_ARGS__)
    #define CX__FOR_EACH_COMMA_32(M, a, ...) M(a), CX__FOR_EACH_COMMA_31(M, __VA_ARGS__)
    
    #define CX__FOR_EACH_COMMA_DISPATCH(N) CX_JOIN2(CX__FOR_EACH_COMMA_, N)
    
    #define CX_FOR_EACH_COMMA(M, ...) \
    CX__EXPAND(CX__FOR_EACH_COMMA_DISPATCH(CX__COUNT_ARGS_(__VA_ARGS__))(M, __VA_ARGS__))

#endif

////////////////////////////////////////////
// Type related macros

// Generators for concepts.

#ifndef CX_CONCEPT_GEN
    #define CX_CONCEPT_GEN(TypeName, is_type_name, ConceptName)                  \
        template<typename T> predicate is_type_name = false;                     \
        template<> predicate is_type_name<TypeName> = true;                      \
        template<typename T> concept ConceptName = is_type_name<cx::rm_cvref<T>>
#endif
#ifndef CX_CONCEPT_GEN_TEMPL  // use VA_ to pass _<ARGS>_ arguments
    #define CX_CONCEPT_GEN_TEMPL(                                                \
            TypeName, is_type_name, ConceptName, _TPARAMS_, _TARGS_              \
        )                                                                        \
        template<typename T> predicate is_type_name = false;                     \
        template<_TPARAMS_> predicate is_type_name<TypeName<_TARGS_>> = true;    \
        template<typename T> concept ConceptName = is_type_name<cx::rm_cvref<T>>
#endif

#ifndef CX_DEFINE_MEMBER_TYPES
    #define CX_DEFINE_MEMBER_TYPES(E, S)     \
        using Elem = E;                      \
        using Size = S;                      \
        using Iter = Elem*;                  \
        using Kter = Elem const*;            \
        using Rter = cx::RevIterator<Iter>;  \
        using KRter = cx::RevIterator<Kter>
#endif
#ifndef CX_USING_MEMBER_TYPES_OF
    #define CX_USING_MEMBER_TYPES_OF(cont)                        \
        using Elem = typename cx::rm_cvref<decltype(cont)>::Elem; \
        using Size = typename cx::rm_cvref<decltype(cont)>::Size; \
        using Iter = Elem*;                                       \
        using Kter = Elem const*;                                 \
        using Rter = cx::RevIterator<Iter>;                       \
        using KRter = cx::RevIterator<Kter>
#endif

#ifndef CX_TYPE_ASSERT
    #define CX_BASIC_TYPE_ASSERT(_BODY_)         \
        static_assert(cx::is_def_ctble<_BODY_>); \
        static_assert(cx::is_cp_ctble<_BODY_>);  \
        static_assert(cx::is_mv_ctble<_BODY_>);  \
        static_assert(cx::is_cp_asble<_BODY_>);  \
        static_assert(cx::is_mv_asble<_BODY_>);  \
        static_assert(cx::is_dtble<_BODY_>)
#endif

#ifndef CX_TRIVIAL_TYPE_ASSERT
    #define CX_TRIVIAL_TYPE_ASSERT(_BODY_)           \
        static_assert(cx::is_triv_cp_ctble<_BODY_>); \
        static_assert(cx::is_triv_mv_ctble<_BODY_>); \
        static_assert(cx::is_triv_cp_asble<_BODY_>); \
        static_assert(cx::is_triv_mv_asble<_BODY_>); \
        static_assert(cx::is_triv_dtble<_BODY_>)
#endif

////////////////////////////////////////////
// Miscellaneous

#ifndef BEG_
    #define BEG_ {
#endif
#ifndef END_
    #define END_ }
#endif
#ifndef CX_HASH
    #define CX_HASH #
#endif
#ifndef CX_EMPTY
    #define CX_EMPTY
#endif

////////////////////////////////////////////
// Undefine macros

#ifndef CX_UNDEF_KEYWORDS
    #define CX_UNDEF_KEYWORDS \
        #undef priv           \
        #undef glob           \
        #undef persist        \
        #undef onedef         \
        #undef fn             \
        #undef noexce         \
        #undef where          \
        #undef proc           \
        #undef func           \
        #undef clos           \
        #undef comp           \
        #undef cons           \
        #undef predicate      \
        #undef cast           \
        #undef nil            \
#endif
      
#endif  // CX_CONF_MACRO

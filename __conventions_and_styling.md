libcx — Conventions and Styling

Working specification · 20 November 2026

This document consolidates the conventions agreed for libcx, a minimal C++23 library with a
procedural, C with concepts approach. It is a standalone reference and a candidate for a
future merge into AGENTS.md. It describes the intended style, including decisions that may
not yet be reflected throughout the codebase. Existing code is a reference, not an excuse to
reintroduce an obsolete convention.

Interpretation: must denotes a design constraint; prefer denotes a guideline that can be
overridden when the local code is clearer. Examples illustrate a convention; they do not
automatically authorize a new API or change an existing API's semantics.

1. Convention inventory

Use this checklist to audit future edits and the eventual AGENTS.md merge. The sections below
expand groups of related items, usually with Rule / Preferred / Avoid examples.

Philosophy and dependencies

Procedural C++23: plain data, free functions, explicit control flow, and explicit ownership.

All ordinary operations must be free functions. Member functions are not a stylistic alternative.

No OOP architecture, object-centered APIs, inheritance, virtual behavior as an ordinary design
pattern, implicit resource management, or routine RAII.

Borrow useful ideas from functional programming without adopting it wholesale.

Pragmatism overrides prescriptive "Clean Code" rules: cohesion and readability matter more
than function length, parameter count, or the number of extracted helpers.

A long function with many arguments is acceptable when it performs one cohesive task.

Avoid excessive fragmentation into tiny functions or files, unnecessary wrappers,
single-use utilities, speculative abstractions, and defensive code.

There are no arbitrary limits on function length, argument count, or header length.

libcx aims to replace the C++ standard library, not build on it silently.

Report any proposed standard-library dependency or implement the needed alternative in libcx.

Use platform/compiler facilities and the existing low-level runtime interfaces as appropriate.

Architectural references: Odin's standard library, the Odin compiler's C++ source, and libcx.

No exceptions, RTTI, or implicit allocations; no revival of abandoned error abstractions.

Names, scope, and language extensions

Types and concepts: UpperCamelCase; functions, variables, and traits: snake_case.

Traits/propositions use is_...; modules use singular lowercase names.

Enumerators: EnumName_Value; full names for types, abbreviated flag/mode families.

Parameter names are short when their types are self-explanatory, descriptive otherwise.

cx is the only non-inline namespace; most modules use inline namespace <module>.

Foundational traits/concepts and similar facilities may live directly in cx.

Free functions in cx do not carry a redundant cx_ prefix.

Private functions need not start with _ when priv conveys visibility.

Public helper macros: cx_snake_case; code-generation macros: CX_UPPER_CASE.

Internal implementation macros: cx__snake_case / CX__UPPER_CASE.

Keyword macros such as fn, cons, and inln are exceptions to prefix rules.

Preserve libcx's keyword aliases: fn, func, onedef, cons, inln, noexce, with.

none means the zero/no-error value; null communicates a null pointer.

ZeroType supplies both; rename the distinct NoneType to EmptyType (pending code edit).

Formatting and templates

Four-space indentation; east const (T const&); normally at most 100 columns.

Short trait/concept declarations may exceed 100 columns slightly to remain readable.

Function bodies open on a new line, including after multiline signatures.

Struct, class, enum, and union bodies open on a new line; namespace braces stay inline.

Control-flow braces stay with the statement, including } else {.

Single-line signatures when they fit; progressively wrap constraints, parameters, templates.

Apply the entire signature-wrapping decision tree; do not jump straight to multiline parameters.

Keep opening braces for empty functions on their own lines; empty records stay compact.

Keep lambda, catch, else, while, case, and extern bracing consistent.

Put a with clause outside a template's angle brackets.

Put template<...> on its own line before ordinary declarations; traits/concepts are exceptions.

Keep simple traits/concepts inline; expand noisy requires expressions selectively.

Align function parameters and call arguments, enum entries/constants, and related struct fields.

Avoid alignment in unrelated declarations or expressions.

Use one struct field or enum item per line.

Data, memory, and errors

Pass small values by value, read-only objects by T const&, mutable objects by T&.

Use pointers for raw memory, optional/null data, and inherently pointer-oriented APIs.

Prefer aggregates, explicit factories, and passive values; avoid custom special members.

PassiveValue combines value-initializability, trivial copyability, trivial destruction.

Distinguish raw storage, byte zeroing, value initialization, and explicit destruction/cleanup.

Favor libcx memory primitives and copying for passive types; avoid placement new by default.

No automatic recursive cleanup of resources held by passive structs.

Use Allocator and explicit, correctly aligned allocation/deallocation.

DynamicArray: buf, len, cap, alc; minimum capacity 16 is specific to this API.

DynamicArray: factories first, then memory management, then other operations and helpers.

Go/Odin-style explicit Res<Ts...> / ErrorCode results; prefer or_return / or_else.

Use soft cx_assert for caller contracts and expected invariants, not recoverable failures;
distinguish caller preconditions from internal consistency checks.

cx_assert and cx_debug may disappear in release; cx_assert_hard and cx_panic remain.

Documentation, comments, and headers

Documentation blocks use /** ... **/, four-space interior indentation, no leading *.

Use backticks around identifiers; keep sections and lists compact and purposeful.

Function tags: @par, @arg, @req, @ret, @inf; omit irrelevant sections.

@req contains only runtime obligations of the caller that the function does not handle
as recoverable errors; it is not a list of all checks in the function.

Match each soft cx_assert about a caller precondition with the corresponding positive
@req condition; do not document soft assertions of purely internal invariants as @req.

Do not put static/concept/template checks, hard-asserted failures, or validated and returned
error conditions in @req.

@ret explains results/recoverable errors when useful; omit for void.

Struct tags: @par, @rep, @inv, @inf; no duplicated operations list.

Annotated comments use // TODO:(manu), NOTE, FIX, XXX; use them sparingly.

Simple comments use //; inline comments, when unavoidable, follow exactly two spaces.

No decorative comment banners or gratuitous section separators.

.hh headers, direct includes, include guards, and coherent dependency layering.

Prefer cohesive headers; do not split a module merely to shorten a file.

Declaration order: types/traits, free comparisons, factories, memory, operations, helpers.

Keep edits local; respect repository scope, approval boundaries, and existing changes.

Update AGENTS.md for accepted reusable conventions, once this specification is approved.

2. Programming model

Rule. libcx deliberately rejects the object-oriented programming model as its default
architecture. It is procedural C++: structs describe data and its representation; free
functions describe operations on that data; and allocation, cleanup, and control flow remain
explicit. This is not simply "prefer free functions where convenient": for ordinary libcx
APIs, free functions are mandatory.

Do not generate public or private member functions, fluent interfaces, classes that encapsulate
operations, inheritance hierarchies, virtual-dispatch-based designs, or friend functions in
place of ordinary free operations. Avoid using member-function syntax merely because a C++
library convention would normally recommend it. A genuine ABI/low-level mechanism such as a
vtable can justify a narrowly scoped exception. The current minimal iterator-related members
are provisional and will be revisited alongside the iterator/range API; they do not establish
a precedent for other modules.

Preferred

struct Counter
{
    isize value;
};

inln fn increment(Counter& counter) noexce -> void
{
    counter.value++;
}

Avoid

class Counter {
public:
    void increment() { ++value; }
private:
    long value = 0;
};

Pragmatism over prescriptive Clean Code

Rule. The conventional "Clean Code" rulebook is not a design authority for libcx.
In particular, reject prescriptions such as "functions must be short", "functions must take
only a few arguments", "every step deserves its own helper", or "all duplication must be
eliminated". These are not project constraints. Readability comes from explicit semantics,
cohesion, predictable control flow, and locality of the code that must be understood together.

A function can be long and accept many parameters without being poorly designed.
If validation, capacity calculation, allocation, copying, and committing new state are parts
of one operation, it can be clearer to keep them together. Do not split them solely to meet
a line count or parameter count. Similarly, a cohesive header may remain large.

Preferred — preserve a cohesive operation

reserve(arr, new_cap):
    validate the request
    compute growth and alignment
    allocate new storage
    preserve existing elements
    commit the result after successful allocation

This is pseudocode describing one cohesive operation, not a new API or an implementation
that may be copied verbatim. An actual free function may have additional arguments and many
more lines when its task needs them. Do not add one comment per stage just to imitate this
outline; the real code should explain itself where practical.

Avoid — fragmentation without a semantic gain

reserve(...)
  -> validate_reserve_input(...)
  -> calculate_reserve_growth(...)
  -> allocate_reserve_buffer(...)
  -> copy_reserve_elements(...)
  -> finish_reserve_update(...)

That decomposition is not automatically wrong if the helpers have real independent value.
It is wrong when performed mechanically: five single-use utilities in different headers can
make one operation harder to understand and maintain than a single well-structured function.
Create an abstraction when it provides genuine reuse, a meaningful independent operation, or
a demonstrable clarity or testing benefit. Small local duplication can be preferable.

Avoid: introducing a one-field options object solely to reduce the visible parameter count;
splitting code based only on length; wrapping a library operation only to rename it; hiding
resource ownership or errors behind architectural layers; treating every conditional as
evidence that another helper is needed.

Rule. Functional-programming ideas may inform pure transformations or composability, but
the dominant model remains explicit procedural code. No OO architecture, exception flow,
implicit resource management, or hidden allocation.

3. Standard-library independence

Rule. The objective is a genuine alternative to the C++ standard library. Do not silently
introduce <vector>, <memory>, <type_traits>, <new>, or similar dependencies. When an
implementation requires a standard-library facility, report the dependency and/or implement an
appropriate libcx alternative within the authorized task scope. Compiler builtins and
platform/OS facilities are appropriate foundations; inspect existing platform abstractions
before adding one. Do not redesign an API just to remove a dependency without authorization.

Preferred: reuse a libcx trait, allocator, mem_copy, or the project's compiler/OS adapter.

Avoid: importing a standard-library convenience merely to save a small amount of code.

When an implementation appears to need std::, make a deliberate choice:

Check whether the required primitive or type already exists in libcx, including its
traits, concepts, memory helpers, macros, and platform abstraction layer.

If not, determine whether a compiler builtin or a platform/system facility provides the
necessary foundation without pulling in a C++ standard-library header.

If the missing operation belongs in libcx and is within the task's scope, implement an
appropriately small libcx alternative in the relevant module.

Otherwise, surface the dependency/design question to the user. Do not silently add a
standard-library include or expand the project beyond the requested edit.

This is an architectural dependency rule, not an instruction to reimplement unrelated
pieces of the standard library on every edit. For example, a compiler-provided trivial-copy
trait is preferable to importing <type_traits>, and passive-value operations should not
introduce <new> merely for placement construction. Existing OS/ABI interfaces need to be
inspected in context instead of guessed or redesigned.

Architectural references, in priority for design inspiration rather than blind copying:

Existing libcx patterns, especially include/libcx/arr/.

Odin standard library (Ginger Bill).

Odin compiler source code in C++, including its low-level implementation idioms.

4. Naming conventions and namespaces

Construct

Convention

Example

Struct/type

UpperCamelCase

DynamicArray, Allocator

Concept

UpperCamelCase

PassiveValue, ValueInitble

Trait/proposition

is_snake_case

is_passive_value

Free function

snake_case

mem_copy, reserve

Variable/parameter

snake_case

new_cap, num

Module

Singular lowercase

mem, trait, concept

Enumerator

EnumName_Value

AllocMode_Alloc

Public helper macro

cx_snake_case

cx_assert

Code-generation macro

CX_UPPER_CASE

CX_DEFINE_MEMBER_TYPES

Internal helper macro

cx__snake_case

cx__internal_helper

Internal generation macro

CX__UPPER_CASE

CX__IMPL

Rule. Prefer fully spelled-out type names; abbreviations are acceptable in related flag/mode
families (AllocFlag_*, AllocMode_*). Parameter names can be short where the type supplies
context (Allocator a), otherwise be descriptive. A function marked priv need not also use an
underscore prefix (priv fn rep_ok(...) is fine).

Preferred

namespace cx {
inline namespace arr {

struct ArrayInfo
{
    isize len;
    isize cap;
};

inln priv fn rep_ok(ArrayInfo const& a) noexce -> bool
{
    return a.len >= 0 && a.len <= a.cap;
}

}  // namespace arr
}  // namespace cx

Foundational traits, concepts, and similar cross-module utilities may instead be declared
directly in namespace cx. Do not introduce other ordinary nested namespaces merely for
namespacing. Free functions inside cx do not need a cx_ prefix.

Zero, absence, and errors

ZeroType supports zero/null semantics; none and null are named values of that type.

Prefer null for null pointers; prefer none for successful/no-error results.

Keep both names: they convey different intent, even though they are not distinct types.

Rename the separate NoneType to EmptyType in a future code edit; renaming the corresponding
value None to Empty is proposed but should be confirmed before changing call sites.

When zero means “no error,” it need not be a named member of an error enumeration; use the
project's established zero/no-error representation instead.

Preferred

return {null, Invalid_Ptr};
return {ptr, none};

5. Semantic macros and keywords

Rule. Preserve existing language aliases. In function declarations prefer fn; use func
for function types where applicable; use onedef for definitions of variables. Retain cons,
inln, noexce, with, and related established aliases rather than rewriting them into
conventional syntax.

Rule. Internal implementation macros of public macro facilities use CX__ or cx__.
Keyword aliases are intentional exceptions. Do not expose an implementation-detail macro
as though it were public API.

Avoid: renaming existing semantic macros, removing them, or creating new aliases without a
specific need and approval.

6. Layout, whitespace, braces, and alignment

Rule. Four spaces per indentation level, east const (T const&), and a general 100-column
limit. Single-line traits/concepts may slightly exceed 100 columns if wrapping harms clarity.
Struct/enum/union/class bodies and function bodies put { on a new line. Namespace and ordinary
control-flow braces stay on the same line. Keep else with the preceding closing brace.

Preferred

namespace cx {
inline namespace mem {

struct Span
{
    byteptr  buf;
    isize    len;
};

inln fn valid(Span const& span) noexce -> bool
{
    if (span.buf == null) {
        return span.len == 0;
    } else {
        return span.len >= 0;
    }
}

}  // namespace mem
}  // namespace cx

Avoid: putting function braces on the signature line; Allman-style braces after if/for;
unnecessary multiline conditions, loops, or extra blank lines.

Bracing reference

The chosen style is a customized AfterFunction: true: function bodies always open on a
new line, including when the declaration has been wrapped. Other bracing decisions follow:

AfterFunction: true
AfterClass: true
AfterStruct: true
AfterEnum: true
AfterUnion: true
AfterNamespace: false
AfterControlStatement: Never
AfterObjCDeclaration: false
AfterCaseLabel: false
AfterExternBlock: false
BeforeCatch: false
BeforeElse: false
BeforeLambdaBody: false
BeforeWhile: false
SplitEmptyFunction: true
SplitEmptyRecord: false

Traits, concepts, and similar declarations are exempt from record-style brace wrapping when
their concise inline form is more readable. The matrix does not override the dedicated
signature and proposition rules below.

Preferred — wrapped condition, compact control braces, lambda, empty bodies

if (first_condition && second_condition &&
    third_condition) {
    act();
} else {
    skip();
}

auto predicate = [](isize x) { return x > 0; };

inln fn noop() noexce -> void
{
}

struct EmptyRecord
{};

Avoid: putting { on a new line after a wrapped if; splitting an otherwise simple
lambda; expanding an empty record over several lines without a reason.

Signature wrapping order

Rule. Consider the entire signature, including attributes, the function name, all
parameters, qualifiers, the return type, and any trailing with clause. Keep the whole
signature on one line if it fits within 100 columns. The template declaration appears
directly above it. If the full signature does not fit, try in order:

If only the with constraint makes it too long, move with to a separate line, indented
four spaces.

Otherwise, put parameters on separate lines and close the parameter list on its own line.
Keep with (<constraints>) beside the return type if that resulting line fits.

If the constraint still does not fit beside the return type, move it to its own line,
indented four spaces.

If that line is still too long, split the constraint over multiple lines, aligning
subsequent conditions under the first condition (or another clearly related column).

Independently, if the template parameter list exceeds the limit, break template<...>
itself across lines, one parameter per line.

Regardless of wrapping, the function's opening brace remains on a new line.
The following examples deliberately demonstrate every branch of this decision tree.

Preferred — case A: everything fits on one line

template<typename T>
cons fn read(T const& value) noexce -> T with PassiveValue<T>
{
    return value;
}

Preferred — case B: only with exceeds the line limit

template<typename T>
inln cons fn validate_container_element(T const& value) noexce -> bool
    with (PassiveValue<T> && TrivCopyable<T>)
{
    cx_unused(value);
    return true;
}

Preferred — case C: parameters wrap, but return type and with still fit together

template<typename E, typename S>
cons fn adjust_capacity(
    DynamicArray<E, S>&  arr,
    S                    requested_cap,
    AllocFlag            flags  = AllocFlag_None
) noexce -> ErrorCode with PassiveValue<E>
{
    // ...
}

Preferred — case D: parameters wrap, with needs its own line

template<typename E, typename S, typename Source>
cons fn allocate_array_storage(
    Allocator       alc,
    Source const*   source,
    S               requested_cap,
    AllocFlag       flags  = AllocFlag_Zero
) noexce -> Res<DynamicArray<E, S>, ErrorCode>
    with (PassiveValue<E> && same_as<PlainT<E>, PlainT<Source>>)
{
    // ...
}

Preferred — case E: the constraint also needs wrapping

template<typename DestinationElement, typename SourceElement, typename Index>
cons fn copy_selected_elements(
    DestinationElement*   destination,
    SourceElement const*  source,
    Index                 count
) noexce -> ErrorCode
    with (PassiveValue<DestinationElement> &&
          is_triv_copyable<SourceElement> &&
          same_as<PlainT<DestinationElement>, PlainT<SourceElement>>)
{
    // ...
}

Preferred — case F: the template parameter list itself needs wrapping

template<
    typename DestinationElement,
    typename SourceElement,
    typename ElementIndex,
    typename AllocatorType
>
cons fn copy_with_allocator(
    AllocatorType        alc,
    DestinationElement*  destination,
    SourceElement*       source,
    ElementIndex         count
) noexce -> ErrorCode
    with (PassiveValue<DestinationElement> &&
          same_as<PlainT<DestinationElement>, PlainT<SourceElement>>)
{
    // ...
}

These are formatting illustrations, not proposals to add the named functions. Keep
qualifiers and constraint placement consistent with the actual libcx grammar and semantic
macros; do not introduce an overload solely to mirror an example.

Preferred — parameters and defaults, without a trailing with

inln fn mem_alloc(
    Allocator  a,
    isize      size,
    isize      align  = DEF_ALIGN,
    AllocFlag  flags  = AllocFlag_None
) noexce -> Res<mutaptr, ErrorCode>
{
    // ...
}

Template formatting — separate from function wrapping

Rule. The template declaration is on its own line for normal functions, structs,
and aliases, even when it is short. A trait or concept may put a short template<...> and
its proposition/concept definition on a single line. Constraints expressed using with
stay outside the template angle brackets. Do not wrap template parameters purely because
they form a list: wrap them when the line genuinely becomes too long.

Preferred — short constrained template

template<typename T> with PassiveValue<T>
struct Holder
{
    T value;
};

Preferred — long constrained template

template<
    typename Element,
    typename Size,
    typename Alloc
> with PassiveValue<Element>
struct ArrayStorage
{
    Element*  buf;
    Size      len;
    Alloc     alc;
};

Preferred — trait/concept exception

template<typename T> proposition is_triv_copyable = __is_trivially_copyable(T);
template<typename... Ts> concept TrivCopyable = (is_triv_copyable<Ts> && ...);

Avoid: keeping with inside angle brackets, splitting every short template into four
lines, or placing a normal function definition on the same line as template<...>.

Alignment

Align only related function parameters, call arguments, enum items, enum-like constants,
and struct fields when doing so improves scanning. Use the existing two-space column
separation where applicable. Do not align ordinary assignments, conditions, or unrelated
local declarations.

Preferred

enum Color
{
    Color_Red    = 1,
    Color_Green  = 2,
    Color_Blue   = 3
};

onedef cons isize PTR_SIZE   = size_of(mutaptr);
onedef cons isize PTR_ALIGN  = align_of(mutaptr);

Avoid: alignment columns everywhere, especially across unrelated statements.
Apply the same principle to multiline call arguments, without introducing columns where
the arguments have no corresponding elements to scan.

Preferred — related call arguments

auto values = select_pairs(
    first_left,   first_right,
    second_left,  second_right
);

Do not extend column alignment to ordinary local variables merely because they are
adjacent. Keep those declarations unaligned:

auto left = combine(a1, b1, mode);
auto right = combine(a2, b2, mode);

7. Traits, concepts, and constraints

Rule. Use existing proposition traits and compose higher-level concepts from them.
Concept names are UpperCamelCase; trait names are is_snake_case. Keep simple compositions
inline, including several plain traits. Use multiline braced form primarily when requires
expressions introduce visual noise; do not expand three clean trait names merely to occupy
more lines.

Preferred — single trait and composed concept

template<typename T> proposition is_triv_copyable = __is_trivially_copyable(T);
template<typename... Ts> concept TrivCopyable = (is_triv_copyable<Ts> && ...);

template<typename T> proposition is_passive_value = is_value_initble<T> && is_triv_copyable<T> && is_triv_dtble<T>;

Preferred — noisy requires

template<typename T> proposition is_def_initble = {
    requires { T{}; } &&
    requires { []{ T x; cx_unused(x); }(); }
};

template<typename T> proposition is_value_initble = {
    is_def_initble<T> &&
    requires { T(); }
};

Avoid: replacing an existing concept with duplicated builtin checks; inventing a concept
only to rename another one; forcing multiline formatting onto a clean one-line expression.

8. Parameter passing and struct interfaces

Rule. Use references for ordinary object parameters, pointers for raw memory and pointer
semantics, and values for cheap scalar data. Prefer T const& for read-only larger objects and
T& for mutable ones. Passing a nullable pointer should be a meaningful API choice, not a
workaround to avoid references. Keep low-level pointer arithmetic in explicitly low-level code.

Form

Typical use

Intent

T value

Integers, flags, simple handles, cheap passive values

Independent value

T const& value

Read-only struct or substantial object

Borrow without copying

T& value

Ordinary operation that mutates an existing struct

Non-null mutable object

T* / mutaptr

Raw buffers, address arithmetic, optionally absent data

Pointer and storage semantics

T const* / readptr

Read-only raw buffers and ranges

Pointer and storage semantics

This is a guideline for semantics, not a rule that all non-scalar values must use
references. A value may be the clearest choice for a small passive struct. Conversely, do not
turn a reference into a pointer just because a function mutates its argument; a mutable
reference already expresses that the object must exist.

Preferred

cons fn reserve(DynamicArray<i32>& arr, isize new_cap) noexce -> ErrorCode
{
    // ...
}

cons fn compare(ArrayInfo const& a, ArrayInfo const& b) noexce -> i32
{
    // ...
}

inln fn mem_copy(mutaptr dst, readptr src, isize size) noexce -> void
{
    // ...
}

Avoid: pointer-to-pointer or pointer-to-const gymnastics when a normal reference expresses
the contract; T* everywhere solely for aesthetic consistency. References to raw arrays can
preserve the extent when useful (char (&arr)[10] is not a pointer-to-pointer).

Rule. For operations on a structure, the structure is normally the first parameter,
T& when mutable and T const& when read-only. Factories are the natural exception because
they create the structure rather than operate on an existing one.

Rule. Prefer no custom constructors, destructors, move/copy operations, or routine RAII.
Keep operator overloading to the minimum truly useful set. Equality (==) and ordering (<=>)
operators are ordinary free functions outside the struct, not friends. Current minimal
iterator support is a deliberately deferred exception; revisit it when designing the iterator
and range API.

9. Explicit allocation, lifetime, and PassiveValue

Rule. PassiveValue describes the intended family of C-like values: value-initializable,
trivially copyable, and trivially destructible. A passive value can contain a pointer and own
storage by explicit convention without introducing an automatic destructor. It need not
have an all-zero value representation: T{} may initialize a member to 42.

Rule. Keep four distinct operations conceptually separate:

Raw allocation: acquire sufficiently sized and aligned storage; no initialized values.

Byte zeroing: set the bytes to zero, possibly via an allocator flag.

Value initialization: produce a T{} value, respecting default member initializers.

Resource cleanup/deallocation: explicitly release separately owned resources and storage.

Byte zeroing is not automatically value initialization. Trivial destruction does not release
resources referenced by a struct's pointer members.

Preferred — distinguish zeroed bytes from T{}

struct ExampleValue
{
    i32 value = 42;
};

ExampleValue initialized{};  // `value` is 42, not zero.

This is the kind of PassiveValue for which the distinction matters. An allocator's
AllocFlag_Zero affects the bytes of the storage; it does not perform C++ value
initialization and must never be used as a substitute for T{} in resize or init_value.
Similarly, AllocFlag_None means no zeroing pass, not that the allocator constructed
elements in some other way.

Preferred — passive initialization using a template value

/**
    Initializes `num` contiguous `T` values at `ptr` from a single `T{}` value.
    @arg
    - `ptr`: destination storage.
    - `num`: number of values to initialize.
    @req
    - `ptr` is not null and is properly aligned for `T`.
    - `num` is greater than zero.
    - `ptr` designates enough writable storage for `num` values of `T`.
**/
template<typename T>
inln cons fn init_value(mutaptr ptr, isize num) noexce -> void with PassiveValue<T>
{
    cx_assert(ptr != null && num > 0);

    T value{};
    T* buf = cast(T*, ptr);

    for (isize i = 0; i < num; i++) {
        mem_copy(buf + i, &value, size_of(T));
    }
}

Avoid: replacing T{} with mem_zero when typed initialization is required; placement
new in ordinary passive-value container operations; implicit recursive cleanup. If a C++
object-lifetime rule becomes relevant to a proposed memory technique, surface the issue rather
than silently changing the API's chosen semantics.

Rule. Memory helpers for passive types normally use libcx primitives (mem_copy,
mem_move) rather than construction/copy machinery for nontrivial C++ objects. Generic helper
names established so far are init_value, init_args, and init_list; their role is to copy
explicitly initialized values into the designated storage. Optional tagged placement support
may exist separately, disabled by default (CX_ENABLE_PLACEMENT), but is not the default
path for these types.

init_value creates a T{} template value and copies its representation into the requested
slots. Its preconditions concern valid aligned storage and a meaningful element count.

init_args creates a value using the supplied arguments and then copies that value as
appropriate for passive types; ordinary repetition does not justify repeated construction.

init_list copies explicitly supplied elements into the destination storage.

These are low-level helpers, not implicit ownership managers. They do not recursively
initialize or deinitialize resources referenced by passive structs.

Do not assume that trivially copyable alone excuses arbitrary invalid pointer access or
misaligned storage. C++ object-lifetime and implicit-object-creation rules still matter: when
a proposed byte-copy implementation would rely on specific lifetime behavior, verify it
against the selected language standard and the actual memory primitive. The goal is
C-like semantics implemented correctly, not undefined behavior disguised as simplicity.

Explicit cleanup: a passive aggregate may hold pointers to separately owned memory, yet
its trivial destructor releases nothing. The module that owns the resource should expose a
specific free/deinit operation when necessary (for example, deinit_company for a hypothetical
owning Company); a generic deinit_type adds no useful semantics for merely passive values.
Do not infer ownership from the presence of a pointer alone: owned and borrowed pointers may
have the same physical representation. A function that explicitly frees an owning struct must
respect its stored allocator's capabilities and the existing error contract.

DynamicArray as an API example, not a universal container rule

Four aggregate fields in order: buf, len, cap, alc; no default member initializers.

Element constraint: PassiveValue.

make: explicit allocation, default heap allocator, default AllocFlag_Zero.

AllocFlag_None: opt out of byte zeroing; neither flag constructs elements.

Minimum allocated capacity of 16 for this container, including empty requests.

dynamic_array(...): convenient explicit factory from values/prvalues; optional allocator.

reserve: change capacity without changing logical length.

resize: change logical length; value-initialize newly exposed elements.

shrink: request smaller capacity while preserving live data and allocator limitations.

free: explicit deallocation, preserving ownership on recoverable failures.

Always preserve alignment, existing elements, and the invariant 0 <= len <= cap.

10. Errors and assertions

Rule. Errors are explicit, Go/Odin-inspired results. Res<Ts...> represents a tuple-like
multiple return, with ErrorCode explicitly included where needed. Prefer or_return and
or_else to repetitive handwritten if (err) branches. Do not restore the abandoned
ResultPair/error-pointer model or invent a new error framework without authorization.

Preferred

auto err = reserve(arr, new_cap) or_return err;

Avoid

auto err = reserve(arr, new_cap);
if (err) {
    return err;
}

Use or_else in the same spirit when the error needs local recovery or transformation.
Follow the actual macro definitions and existing call sites; do not invent new propagation
syntax, new result wrappers, or a branch solely because it feels more familiar than the macro.

Rule. Distinguish recoverable failures from caller-contract violations:

Mechanism

Role

Release behavior

ErrorCode / Res

Recoverable failure

Must remain meaningful

cx_assert

Expected invariant / unchecked caller contract

May be disabled

cx_debug

Debug diagnostics

May be disabled

cx_assert_hard

Mandatory runtime assertion

Remains active

cx_panic

Fatal failure

Remains active

Do not use a soft assert as the sole protection against a failure that can legitimately occur
in release. Avoid repeating checks for facts guaranteed by types, constraints, or contracts.

11. Function documentation and contracts

Rule. Documentation is informative, compact, and proportional to complexity. Use
/** ... **/ blocks, four-space interior indentation, no leading * on each line,
backticks around identifiers, and lists under the section markers. Keep sections adjacent
without empty separator lines unless a gap materially improves readability. Use a period at
the end of list descriptions consistently. Not all functions require every section.

Tag

Meaning

Include when

@par

Template parameters

Their role needs explanation

@arg

Runtime function parameters

Their meaning is not self-evident

@req

Runtime preconditions guaranteed by the caller

They are not handled as recoverable failures

@ret

Return values and recoverable errors

A useful non-void result exists

@inf

Additional information

It affects correct understanding

@ret is omitted for void functions. Never add sections to satisfy a template mechanically.
For a trivial function, one short descriptive sentence may suffice.

Exact meaning of @req

Rule. @req has a narrow, contractual meaning. Include a statement if and only if it
is a condition that:

Must hold at runtime for correct use of the function.

Is the caller's responsibility to guarantee before calling.

Is not already handled by the function as a recoverable error or an always-active
runtime check.

In particular, @req is not the place to list the requirements of a with clause,
template constraints, concepts, traits, compiler diagnostics, static_assert, or any other
compile-time guarantee. It is also not an inventory of every if or every assertion appearing
in the function body.

Rule — @req and soft cx_assert must agree. A soft cx_assert guarding a runtime
precondition imposed on the caller is the debug-time manifestation of that same contract.
The corresponding @req must state the same valid condition, not the invalid condition
that triggers the assertion. For example:

Source code

Correct @req

Invalid state (the opposite)

cx_assert(ptr != null);

ptr must not be null

ptr == null

cx_assert(num > 0);

num must be positive

num <= 0

cx_assert(len <= cap); on an incoming value

len <= cap

len > cap

Both sides must describe the same predicate. An agent must not add an @req that
contradicts a soft assert, nor omit the documented caller precondition just because the debug
build asserts it. Soft asserts may disappear entirely in release. Conversely, a soft
cx_assert(rep_ok(arr)) checking the function's own postcondition is an internal invariant,
not an extra caller requirement. Only an asserted condition on the incoming state that the
caller is responsible for belongs in @req.

Preferred — matching contract and debug-time check

/**
    Returns the first element's address in a nonempty range.
    @arg
    - `ptr`: pointer to the first element.
    - `num`: number of elements in the range.
    @req
    - `ptr` is not null.
    - `num` is greater than zero.
    @ret
    - `ptr`, the address of the first element.
**/
template<typename T>
inln fn first_element(T* ptr, isize num) noexce -> T*
{
    cx_assert(ptr != null);
    cx_assert(num > 0);
    return ptr;
}

The @req conditions match the two runtime caller-precondition assertions exactly.
Conditions such as sufficient buffer extent, accessible storage, correct alignment, or
non-overlap may also belong in @req when they are genuine caller obligations, even when a
pointer-only API cannot reliably check them locally. Do not invent a fake runtime check to
force a mechanical one-to-one assertion count. The correspondence rule applies to soft
assertions that actually encode caller preconditions, not to postcondition checks or
internal implementation assertions.

Preferred — explicitly handled conditions are recoverable errors, not @req

/**
    Validates a pointer and returns it on success.
    @arg
    - `ptr`: pointer to validate.
    @ret
    - `[ptr, none]` when `ptr` is not null.
    - `[null, Invalid_Ptr]` when `ptr` is null.
**/
inln fn checked_ptr(mutaptr ptr) noexce -> Res<mutaptr, ErrorCode>
{
    if (ptr == null) {
        return {null, Invalid_Ptr};
    }
    return {ptr, none};
}

No @req is present: the function checks the null case and returns an error. A runtime
case handled using cx_assert_hard likewise is not an unchecked caller precondition; the
check remains active. Compile-time constraints also stay out of @req even if they are
essential to the template's validity.

Decision guide

Condition in or around a function

Document in @req?

Where it belongs instead

Caller runtime precondition, soft-asserted

Yes

Matching positive @req statement

Caller runtime precondition, otherwise unchecked

Yes

@req

Internal invariant or postcondition, soft-asserted

No

Implementation; struct @inv if relevant

Runtime case checked and returned as ErrorCode

No

@ret when useful; otherwise behavior description

Runtime condition enforced by cx_assert_hard

No

Behavior/@inf if its consequence needs explaining

Template with, concept, trait, static check

No

Declaration itself; @par only for parameter meaning

Avoid: putting a checked null-pointer failure in both @req and @ret; documenting
T{args...} being well-formed as a runtime requirement; using a soft cx_assert for an
error that can legitimately happen and must be recoverable; listing internal checks as caller
obligations; repeating a trivial failure across opening sentence, @req, and @ret.

Rule. The opening sentence explains the function's behavior/effects. @ret may explain
meaningful error branches; commonplace error details can be omitted when a familiar
convention makes them obvious. Avoid redundancy. The @req section is a contract, not a
second error-handling or type-checking section.

12. Struct documentation

Rule. Document a struct's purpose, representation, and invariants rather than duplicating
the free-function API. Struct documentation uses an analogous compact block but a different
vocabulary:

Tag

Meaning

@par

Template parameters

@rep

Representation and member semantics

@inv

Invariants of a valid instance

@inf

Additional information, notably ownership conventions

No @arg, @ret, or @ope for an ordinary struct. The operations have their own function
documentation; documenting them again on the struct would be redundant. @inv describes
properties of a valid object state, whereas @req describes a function caller's runtime
obligations. Not every struct needs all four tags.

Preferred

/**
    Dynamic array with explicitly managed storage.
    @par
    - `E`: element type.
    - `S`: size type.
    @rep
    - `buf`: pointer to the allocated storage.
    - `len`: number of logical elements.
    - `cap`: allocated capacity.
    - `alc`: allocator used for storage management.
    @inv
    - `0 <= len <= cap`.
    - A null `buf` implies zero length and capacity.
    @inf
    - Allocation and deallocation are explicit.
**/
template<typename E, typename S = isize> with PassiveValue<E>
struct DynamicArray
{
    E*         buf;
    S          len;
    S          cap;
    Allocator  alc;
};

Avoid: a @ope catalog of free functions, routine member-function descriptions, or a
dozen sections for a two-field passive record. The precise granularity of @rep and @inv
remains subject to review against representative structs.

13. Ordinary comments

Rule. Most internal comments are short // lines placed above the relevant code.
Use the exact annotated marker shape for substantial notes and discoverability in Neovim:

// TODO:(manu) Add the missing platform branch.
// NOTE:(manu) Allocation ownership stays with the caller.
// FIX:(manu) Correct this invariant before enabling the optimization.
// XXX:(manu) Revisit this design choice.

Do not tag every trivial observation. A minimal end-of-line comment is allowed when it helps;
insert exactly two spaces before //.

Preferred

isize offset = 0;  // Offset is measured in bytes.

Avoid: decorative // ----- section separators, repetitive comments explaining an
obvious statement, or lengthy editorial notes inside a critical code path.

14. Header organization

Rule. Header files use .hh, a file comment where appropriate, include guards, direct
includes, and cohesive inline-module namespaces. Do not depend on transitive includes or
make a lower-level header depend on a higher-level abstraction. Do not split a coherent header
solely to reduce line count.

Preferred high-level order, subject to declaration dependencies:

File comment and include guard.

Direct dependencies.

Namespace opening.

Foundational constants/definitions.

Types/structs/enums and associated aliases.

Module-specific traits/concepts (or earlier when required).

Free equality and comparison operators.

Factories and creation (make, dynamic_array, etc.).

Memory management (reserve, resize, shrink, free, etc.).

Primary operations (insert/remove/search/modify).

Additional utilities and helpers.

Authorized local tests, if appropriate, then namespace and guard closure.

Within structure-oriented free functions, the structure is generally the first parameter by
reference. Factory functions do not take an existing structure and are the natural exception.
Do not add decorative section banners just to expose this order.

15. Workflow and OpenCode instructions

Read the project's current AGENTS.md and relevant existing code before editing; the
task's authorization and repository instructions determine scope.

Inspect directly relevant source, dependencies, and tests as needed. Avoid editing generated,
third-party, or unrelated configuration files without a reason within the requested task.

Add no dependency without approval; report unavoidable dependencies and design tradeoffs.

Ask before broad structural changes or public API redesigns.

Preserve user edits and unrelated code; make focused changes.

Use targeted compile checks and tests appropriate to the requested changes; do not turn a
focused task into a repository-wide refactor or test-suite redesign.

Keep OpenCode prompts concise and actionable; if instructed to edit only, do not waste
tokens on lengthy summaries or speculative work.

Update AGENTS.md when a new reusable convention is accepted, not for every local edit.

When the user authorizes the merge, consolidate these rules into the dedicated
Styling and Conventions section of AGENTS.md, using this document as the detailed
reference and applying its principles to the other sections without gratuitous duplication.

Treat this document as a reviewable milestone; do not silently overwrite AGENTS.md with it.

16. Deferred decisions and migration notes

These are intentionally not unconditional instructions to modify the codebase now:

NoneType → EmptyType is accepted as a future rename; whether its constant None becomes
Empty should be confirmed at implementation time. Keep none and null distinct in usage.

The exact iterator/range/member-operator API remains a separate design discussion.

The struct @rep/@inv granularity, and whether every mundane error belongs in @ret,
should be checked on more examples before locking a final documentation policy.

Old headers may still use @pre, @note, @nota, placement construction, or legacy layout;
do not treat such occurrences as endorsed conventions.

The documentation tag system is a libcx house style; verify external documentation-tool
compatibility separately if machine parsing is required.

Audit existing standard-library includes and decide replacements as focused work, rather
than performing a repository-wide rewrite without authorization.


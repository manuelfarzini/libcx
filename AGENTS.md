# libcx

libcx is a minimal C++23 library with a procedural, “C with concepts” design and the
ambition to be a genuine alternative to the C++ standard library. This guide is
self-contained: **must** denotes a design constraint; **prefer** denotes a guideline
that may yield to clearer local code. Examples illustrate conventions, not authorization
for new APIs or changed semantics. Existing code may predate these conventions.

## Design philosophy

- Structs describe data and representation; **ordinary operations must be free functions**.
  Keep allocation, ownership, cleanup, and control flow explicit. Do not introduce OOP
  architecture, object-centered or fluent APIs, ordinary member or friend functions,
  inheritance, virtual dispatch as a normal pattern, routine RAII, or implicit resource
  management. A genuine low-level or ABI mechanism can justify a narrow exception.
  Existing minimal iterator members are provisional, not a precedent.
- Functional ideas may inform pure transformations and composition, but do not displace the
  procedural model. No exceptions, RTTI, implicit allocations, or abandoned error abstractions.
- Favor cohesion, locality, and understandable control flow over prescriptive Clean Code
  rules. There are no arbitrary limits on function length, parameter count, or header length.
  Avoid speculative abstractions, wrappers that merely rename, unnecessary defensive checks,
  one-field options objects made only to reduce parameters, and mechanical extraction into
  single-use helpers. Extract when there is meaningful independent value, reuse, clarity,
  or a demonstrable testing benefit; some local duplication is acceptable.
- Design references are existing libcx patterns (especially `include/libcx/arr/`), Odin's
  standard library, and the Odin compiler's C++ source. Use for inspiration, not blind copying.

### Standard-library independence

Do not silently introduce C++ standard-library dependencies such as `<vector>`, `<memory>`,
`<type_traits>`, or `<new>`. First look for libcx primitives, then compiler builtins and
existing platform/OS facilities. If appropriate and within scope, implement a small libcx
alternative; otherwise surface the dependency or design question. Do not reimplement
unrelated facilities or redesign an API merely to remove a dependency without authorization.
Existing low-level runtime and OS/ABI interfaces must be inspected in context.

## Styling and conventions

### Naming, namespaces, and semantic macros

- Types and concepts use `UpperCamelCase`; functions and variables use `snake_case`;
  traits/propositions use `is_snake_case`; modules use singular lowercase names.
  Enumerators use `EnumName_Value`, e.g. `Color_Red`.
- Established fundamental types and aliases such as `initls`, `cstring`, and `dtype`
  retain lowercase names. Do not mechanically rename primitive-like facilities.
- Prefer full type names (`Allocator`); abbreviations are appropriate in related flag/mode
  families (`AllocFlag_*`, `AllocMode_*`). Use short parameter names where the type supplies
  context (`Allocator a`), descriptive names otherwise. A function marked `priv` needs no
  leading underscore.
- `cx` is the only non-inline namespace. Most modules use `inline namespace <module>`;
  foundational traits, concepts, and similar facilities may live directly in `cx`. Free
  functions in `cx` do not carry a redundant `cx_` prefix.
- Public helper macros use `cx_snake_case`, code-generation macros `CX_UPPER_CASE`;
  implementation macros use `cx__snake_case` or `CX__UPPER_CASE`. Keyword aliases are
  exceptions. Do not expose internal macros as public APIs.
- Preserve semantic aliases such as `fn`, `func`, `onedef`, `cons`, `inln`, `noexce`, and
  `with`. Prefer `fn` for functions, `func` for function types where applicable, and `onedef`
  for variable definitions. Do not rename/remove aliases or add new ones without a specific
  need and approval.
- `none` and `null` are both `ZeroType` values: use `none` for zero/no-error results and
  `null` for null pointers. Zero need not be a named error enumerator.

### Boolean types

- Use `b32` for ordinary boolean parameters, return values, local variables, and fields.
  This is the project's default; use `b8` or `b16` when storage size matters. A possible
  switch to `b64` remains a future evaluation, not the current convention.
- Keep C++ `bool` where needed for metaprogramming: `proposition`, concepts, boolean
  template parameters, and compile-time trait helpers/constants. Do not mechanically
  replace it in language interoperability mechanisms such as contextual `operator bool`
  conversions or the low-level C compatibility definitions.
- Built-in comparison/logical expressions still produce C++ `bool`; the rule concerns
  ordinary declared boolean interfaces and storage, not changing the language's operators.
- When equality operators return `b32`, provide an explicit `operator!=` if needed;
  C++'s rewritten `!=` candidates require `operator==` to return exactly `bool`.

### Layout and braces

Use four-space indentation, east const (`T const&`), and normally at most 100 columns.
Short traits/concepts may slightly exceed the limit when that preserves clarity. Put function,
struct, class, enum, and union opening braces on a new line. Namespace and control-flow braces
stay on the same line for single-line conditions; multiline `if`, `for`, `while`, and similar
conditions put the opening brace on a separate line. Keep `else` attached to the preceding
closing brace. Lambda, `catch`, `while`,
case-label, and `extern` braces follow the same compact style. Empty functions retain a
separate opening brace; empty records stay compact. Traits/concepts can use compact braces.

```cpp
if (short_condition) {
    act();
}

if (first_long_condition && second_long_condition &&
    third_long_condition)
{
    act();
}
```

```cpp
namespace cx {
inline namespace mem {
struct Span
{
    byteptr  buf;
    isize    len;
};

inln fn valid(Span const& span) noexce -> b32
{
    if (span.buf == null) {
        return false;
    } else {
        return true;
    }
}
}       // namespace mem
}       // namespace cx
```

Keep simple expressions, loops, conditions, and lambdas inline. Use one struct field or enum
item per line. Align related parameters, call arguments, enum items/constants, and struct
fields where useful, with two-space column separation where applicable. Do not align unrelated
local declarations, assignments, or conditions.

### Signatures, templates, and constraints

Consider the complete signature, including attributes, qualifiers, return type, and trailing
`with`. Keep it on one line if it fits. If not, apply this order:

1. If only `with` makes it too long, move it to its own line, indented four spaces.
2. Otherwise put parameters one per line, with the closing parenthesis on its own line;
   keep the constraint beside the return type if it fits.
3. Otherwise move the constraint to its own indented line; if still too long, wrap conditions
   aligned beneath the first condition or another clearly related column.
4. Independently wrap an overlong `template<...>` list, one parameter per line.

The function body always opens on a new line. Align parameter names and defaults:

```cpp
template<typename E, typename S>
cons fn adjust_capacity(
    DynamicArray<E, S>&  arr,
    S                    requested_cap,
    AllocFlag            flags  = AllocFlag_None
) noexce -> ErrorCode with PassiveValue<E>
{
    // ...
}
```

For ordinary functions, structs, and aliases, put the template declaration on its own line;
keep `with` outside the angle brackets. Do not wrap short template lists merely because they
contain several parameters. Short traits and concepts may put the template and declaration on
one line. Compose concepts from existing propositions, not duplicated builtin checks or
aliases that merely rename another concept. Keep simple compositions inline, even with several
traits; use compact braced multiline form mainly for noisy `requires` expressions:

```cpp
template<typename T> proposition is_triv_copyable = __is_trivially_copyable(T);
template<typename... Ts> concept TrivCopyable = (is_triv_copyable<Ts> && ...);

template<typename T> proposition is_value_initble = {
    is_def_initble<T> &&
    requires { T(); }
};

template<typename T> with PassiveValue<T>
struct Holder
{
    T value;
};
```

### Parameters and data structures

Pass cheap scalars, handles, and small passive values by value; ordinarily use `T const&`
for read-only substantial objects and `T&` for mutable objects. Pointers express raw memory,
nullable data, pointer arithmetic, or inherently pointer-oriented APIs, not merely mutation.
References to raw arrays can preserve their extent. For structure operations put the structure
first, normally by reference; factories naturally need no existing structure.

Prefer aggregates and explicit factories; avoid custom constructors, destructors, copy/move
operations, and routine RAII. Keep operator overloading minimal; equality and ordering are free
functions outside the struct, not friends. Iterator/range member support remains provisional.
`defer` is an accepted narrow destructor-based scope-exit exception, not a precedent for
routine RAII or implicit resource ownership.

### Memory, lifetime, and ownership

`PassiveValue` means value-initializable, trivially copyable, and trivially destructible.
A passive struct may explicitly own storage through a pointer, but its trivial destructor does
not release it. Do not infer ownership from pointer representation or perform automatic
recursive cleanup. Resource-specific cleanup must respect allocator capabilities and existing
error contracts.

Keep raw allocation, byte zeroing, value initialization (`T{}`), and resource cleanup /
deallocation distinct. A `T{}` value can have a nonzero member initializer; `AllocFlag_Zero`
zeroes bytes and does not construct values, while `AllocFlag_None` omits zeroing and does not
construct values either. Use `Allocator` and preserve size and alignment. Prefer libcx memory
primitives and copying over placement construction for passive types:

- `init_value` creates one `T{}` template value and copies it into valid aligned slots.
- `init_args` creates a value from the supplied arguments and copies it as appropriate;
  repetition ordinarily does not require repeated construction.
- `init_list` copies supplied elements. None of these helpers recursively manages resources.

Optional tagged placement support (`CX_ENABLE_PLACEMENT`) is separate and disabled by default.
Keep custom placement new and matching delete, but do not use them unless explicitly
requested; retain a `NOTE:(manu)` beside their definitions stating this restriction.
Verify C++23 lifetime and implicit-object-creation rules against the actual memory primitive
when a byte-copy technique relies on them; do not hide undefined behavior or silently change
API semantics.

For `DynamicArray` specifically, the intended aggregate fields are `buf`, `len`, `cap`, `alc`
in that order, without default member initializers; elements are `PassiveValue`. `make` uses
explicit allocation, by default the heap allocator and `AllocFlag_Zero`; minimum allocated
capacity 16 is specific to this API. `dynamic_array` is an explicit value factory; `reserve`
changes capacity, `resize` value-initializes newly exposed elements, `shrink` preserves live
data and allocator limitations, and `free` explicitly deallocates while preserving ownership
on recoverable failure. Maintain alignment, elements, and `0 <= len <= cap`. These examples
do not authorize API changes or impose the same capacity policy on other containers.

### Errors and contracts

Use Go/Odin-style explicit multiple returns: `Res<Ts...>` is a tuple-like result, with
`ErrorCode` explicitly included where appropriate. Prefer `or_return` and `or_else` over
repetitive handwritten propagation, following their actual definitions. Do not revive
`ResultPair`/error-pointer or invent another error framework without approval.

- `ErrorCode` / `Res`: recoverable failures, meaningful in release builds.
- `cx_assert`: soft debug check for unchecked caller contracts or expected internal
  invariants; may disappear in release. Distinguish incoming caller obligations from internal
  consistency checks and postconditions.
- `cx_debug`: debug diagnostics, potentially disabled in release.
- `cx_assert_hard`: mandatory runtime assertion; `cx_panic`: fatal failure. Both remain active.
- `cx_assume`: optimization assumption, not runtime validation; use only when the condition is
  already guaranteed by the contract or implementation.

Do not rely on a soft assertion for a legitimate recoverable failure. Avoid rechecking facts
already guaranteed by types, constraints, or contracts.

### Documentation and comments

Documentation is compact and proportional to complexity. Use `/** ... **/`, four-space
interior indentation, no leading `*`, backticks around identifiers, and compact lists with
periods. Keep sections adjacent unless spacing materially helps; omit irrelevant sections.
A simple function may need only a sentence.

Function tags: `@par` for template parameter meaning, `@arg` for non-obvious runtime
parameters, `@req` for unchecked runtime caller obligations, `@ret` for useful non-void
results and recoverable errors, `@inf` for additional important information. Omit `@ret` for
void. The opening sentence describes behavior/effects; avoid repetition.

**Rule — `@req` and soft assertions:** document a condition there only if the caller must
ensure it at runtime and the function does not handle it as a recoverable error or enforce it
with an always-active check. A soft `cx_assert` of a caller precondition must match a positive
`@req` condition (e.g. `ptr != null` means “`ptr` is not null”). Also document genuine
unchecked obligations such as alignment, extent, accessibility, or non-overlap when relevant;
do not invent checks just to match documentation. Internal invariants and postconditions are
not caller requirements. Compile-time constraints, traits, concepts, static checks, recoverable
error cases, and hard-asserted failures do not belong in `@req`; describe meaningful errors in
`@ret` and hard-check behavior in `@inf` when useful.

```cpp
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
```

Struct documentation describes purpose, representation, invariants, and ownership, using
`@par`, `@rep`, `@inv`, and `@inf` as needed. Do not use function tags `@arg`/`@ret` or an
`@ope` catalog that duplicates the free-function API. `@inv` describes valid object state,
not a function's `@req`.

Ordinary comments are usually short `//` lines above the code. Use substantial annotations
sparingly, in the exact form `// TODO:(manu) ...`, `// NOTE:(manu) ...`, `// FIX:(manu) ...`,
or `// XXX:(manu) ...`. When an end-of-line comment helps, put exactly two spaces before
`//`. Avoid decorative banners, obvious comments, and lengthy editorial notes in critical paths.

### Header and API organization

Use `.hh` headers, file comments where appropriate, include guards, direct includes, and
coherent module namespaces. Do not rely on transitive includes or introduce a lower-level
header's dependency on a higher-level abstraction. Prefer cohesive headers over arbitrary
splitting. Subject to declaration dependencies, order contents as follows:

1. File comment, guard, direct includes, namespace opening.
2. Foundational constants/definitions; types, structs, enums, and aliases; module traits and
   concepts (earlier if needed).
3. Free comparisons; factories; memory management; primary operations; utilities/helpers.
4. Authorized local tests if appropriate, namespace closure, guard closure.

Do not add decorative section banners merely to show this order.
Mark an entirely embryonic/prototype module directory with a `__` prefix; in an otherwise
usable module, prefix only its prototype files. Do not add redundant prefixes to every file
inside an already marked prototype directory. Preserve extensions. Archived implementations
use `__<name>_old.<extension>`, for example `arr/__multi_array_old.hh`. Update includes,
file-path comments, and include guards when moving files. Prototype markers indicate work
status, not additional C++ namespaces, and do not imply the remaining code is finished.

At module header closures, align namespace and include-guard comments with `//` in column 9:

```cpp
}       // namespace mem
}       // namespace cx
#endif  // CX_MEM_ALLOCATOR_HH
```

### Local validation tests

- Do not use the global `CX_TEST` switch. Gate important structures' local tests with
  meaningful contextual flags such as `CX_TEST_DYNAMIC_ARRAY`, enabled only through compiler
  flags when needed. Headers must not define their own test-enabling flags.
- Use one simple executable per module (for example `arr/test_array.cc`) to invoke cases
  through `CX_TEST_DEFINE`, `CX_TEST_CASE`, and `CX_TEST_RUN`.
- Local tests check basic behavior and the caller-side interface; broader testing lives in
  `./test`. This does not lift the workflow restriction on that directory.
- For now correct existing documentation; do not add missing documentation or specifications
  unless requested.

## Development workflow and restrictions

- Work primarily in `include/` and `src/`; inspect relevant code and direct dependencies.
- Do not inspect or modify `test/`, `extern/`, `build/`, `.cache/`, `.clang-format`,
  or `Doxyfile*` unless requested.
- Preserve user changes and unrelated code; make focused changes. Add no dependencies
  without approval. Report unavoidable dependencies and design tradeoffs.
- Ask before changing public APIs or making broad structural changes.
- Verify affected headers with targeted compile checks when feasible; use tests appropriate
  to the authorized scope. Do not undertake unrelated test-suite or repository-wide refactors.
- Update this guide when a reusable convention is accepted, not for every local edit.

## Provisional decisions and migration

- The separate empty sentinel is officially `EmptyType`, with value `Empty`; retain
  `ZeroType`, `none`, and `null` for their distinct zero/no-error/null semantics.
- Retain the existing `<initializer_list>` dependency for now; its replacement is deferred.
- The iterator/range/member-operator API remains a separate design discussion.
- The granularity of struct `@rep`/`@inv` and whether mundane errors need `@ret` remain
  subject to review. Verify external documentation-tool compatibility if machine parsing
  is required.
- Legacy tags, placement construction, and old layout are not endorsements. Audit existing
  standard-library includes and replace them as focused, authorized work, not a silent
  repository-wide rewrite.

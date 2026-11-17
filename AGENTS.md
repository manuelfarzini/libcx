# libcx

Minimal C++23 library with a procedural, "C with concepts" design.
Use `include/libcx/arr/` as the reference for style and API design.

## Style

- Prefer plain structs, free functions, and explicit allocation/ownership.
- Types: `UpperCamelCase`; functions and variables: `snake_case`.
- Module names are singular (e.g. `trait`, `concept`).
- Use full names for types (`Allocator`); abbreviate only in related flags and
  modes (`AllocFlag_*`, `AllocMode_*`).
- Enumerators use `EnumName_Value`: `AllocMode_Alloc`, `Color_Red`.
- Pick parameter names by context: concise when the type makes them obvious
  (`Allocator` → `a`, `Persona` → `p`), explicit otherwise.
- `namespace cx` is the only non-inline namespace. Each module lives in an
  `inline namespace <module>`, and free functions inside `cx` take no `cx_`
  prefix.
- Function-like helper macros: `cx_snake_case`; code-generation macros:
  `CX_UPPER_CASE`. Keyword aliases such as `fn` and `cons` are exceptions.
- Keep these semantic macros; do not replace them with conventional C++ syntax
  without a reason.
- In functions always prefer `fn`. Use `func` only for function types that
  cannot be inline, and `onedef` for variables.
- Declarations with several parameters or types use one per line. Keep the
  opening paren on the function line and put the closing paren, return type,
  and body brace on their own line. Align types two spaces after the longest
  type name, and align defaults the same way:

  ```cpp
  inln fn mem_alloc(
      Allocator  a,
      isize      size,
      isize      align  =  DEF_ALIGN,
      AllocFlag  flags  =  AllocFlag_Default
  ) noexce -> Res<mutaptr, ErrorCode> {
      ...
  }
  ```

  Short signatures may stay on one line, with the body brace on the next line.
  Structs and enums follow the same one-member-per-line rule.
- Keep short boolean expressions, loop headers, and similar constructs inline.
  If a condition must wrap, break it and put `{` on its own line:

  ```cpp
  if (first_condition && second_condition && third_condition &&
      fourth_condition && fifth_condition && sixth_condition)
  {
      ...
  }
  ```

- Keep simple propositions and concepts inline. For propositions whose
  `requires` expressions would be noisy inline, use the compact braced form:

  ```cpp
  template<typename T> proposition is_value_initble = {
      is_def_initble<T> &&
      requires { T(); }
  };
  ```

  Do not expand a proposition merely because it composes several simple
  traits. Short trait or proposition declarations may slightly exceed the
  100-column limit when that keeps them inline.
- Maximum line length is 100 columns; otherwise follow the surrounding style.
- Handle errors explicitly with Go-style multiple returns: `Res<Ts...>` is
  an alias for `Tuple<Ts...>`, using `ErrorCode` explicitly.
- No exceptions, RTTI, implicit allocations, or revival of the experimental
  `ResultPair`/error-pointer system.

## Memory and errors

- `PassiveValue` containers favor explicit memory operations and trivial
  copying over placement construction.
- Prefer libcx's own memory utilities over unnecessary standard library
  facilities.
- Prefer `or_return` and `or_else` for idiomatic error handling.

## Headers

- Include direct dependencies; do not rely on transitive includes.
- Keep lower-level headers independent of higher-level abstractions.
- Favor cohesive modules over arbitrary splitting; long headers are fine.

## Workflow

- Work primarily in `include/` and `src/`.
- Do not inspect or modify `test/`, `extern/`, `build/`, `.cache/`,
  `.clang-format`, or `Doxyfile*` unless requested.
- Keep changes small; add no dependencies without approval.
- Ask before changing public APIs or making broad structural changes.
- Verify affected headers with targeted compile checks when feasible.

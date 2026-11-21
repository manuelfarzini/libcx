# libcx — Style and conventions checklist

## 1. General philosophy and architecture

- C++23, with a procedural **“C with concepts”** approach.
- Procedural programming is mandatory, not merely preferred.
- Use plain, C-like `struct` types for data representation.
- Use free functions for ordinary operations; member functions are prohibited except for narrowly justified mechanisms such as vtables or the provisional minimal iterator interface.
- Avoid object-oriented programming, inheritance, class hierarchies, and unnecessary encapsulation.
- Operator overloading must be minimal and justified.
- Prefer external free-function operators over member or `friend` operators.
- Incorporate useful aspects of functional programming where appropriate, without adopting the paradigm wholesale.
- No exceptions or RTTI.
- No implicit allocation, deallocation, or resource ownership.
- Avoid RAII, user-defined constructors/destructors, and custom copy/move behavior for ordinary passive structures.
- `defer` is an accepted narrow destructor-based scope-exit exception, not a precedent for implicit ownership.
- Explicit data flow, control flow, error handling, and ownership.
- Favor predictability, simplicity, performance, and transparent behavior.
- Use the existing libcx code, the Odin standard library, and the Odin compiler’s C++ source as primary architectural references.

## 2. Standard-library independence

- libcx aims to be a genuine alternative to the C++ standard library.
- Avoid dependencies on standard C++ library headers and facilities.
- Platform-specific operating-system, compiler, and runtime facilities are acceptable where appropriate.
- Prefer libcx implementations of functionality ordinarily supplied by the standard library.
- When necessary functionality is missing, report the dependency or implement a suitable libcx alternative.
- Do not silently introduce standard-library dependencies.
- Do not add external dependencies without approval.
- Preserve necessary compiler and language intrinsics when they provide appropriate low-level functionality.
- Avoid unnecessary wrappers around existing libcx primitives.
- Keep lower-level modules independent of higher-level abstractions.

## 3. Pragmatism over dogmatic Clean Code

- Apply **pragmatic cleanliness**, not the prescriptive Clean Code methodology.
- Do not impose arbitrary limits on function length.
- Do not impose arbitrary limits on the number of parameters.
- Long functions are acceptable when their responsibilities are cohesive.
- Functions with many arguments are acceptable when those arguments are genuinely needed.
- Prefer one coherent implementation over many artificially fragmented helpers.
- Do not extract functions merely to reduce line counts.
- Avoid single-use helpers unless they provide real semantic, structural, or technical value.
- Avoid creating separate headers for trivial implementation details.
- Cohesive, long headers are acceptable.
- Limited duplication can be preferable to unnecessary abstraction.
- Introduce abstractions only when they meaningfully improve reuse, comprehension, correctness, or performance.
- Avoid wrapper functions that add no meaningful semantics.
- Avoid excessive indirection.
- Keep the control flow straightforward.
- Avoid defensive programming and speculative edge-case handling.
- Do not complicate normal execution paths to handle implausible programmer mistakes.
- Prefer contracts and assertions over redundant validation when appropriate.
- Never sacrifice correctness for simplicity or performance without explicitly recognizing the trade-off.

## 4. Naming conventions

- Types: `UpperCamelCase` — `DynamicArray`, `Allocator`, `ErrorCode`.
- Established fundamental types and aliases such as `initls`, `cstring`, and `dtype` retain lowercase names.
- Functions: `snake_case` — `mem_copy`, `init_value`, `reserve`.
- Variables: `snake_case` — `new_cap`, `old_size`.
- Module names: singular lowercase — `trait`, `concept`, `mem`.
- Enumerators: `EnumName_Value` — `AllocMode_Alloc`, `Color_Red`.
- Constants follow existing libcx conventions, including uppercase names for module-level constants where appropriate.
- Prefer complete, meaningful type names.
- Abbreviations are acceptable for related flags and modes, such as `AllocFlag` and `AllocMode`.
- Parameter names may be short when their types make the meanings obvious.
- Prefer descriptive names when context does not make their meanings clear.
- Trait propositions: `is_snake_case` — `is_passive_value`.
- Concepts: `UpperCamelCase` — `PassiveValue`.
- Function-like macros: `cx_snake_case`.
- Code-generation and uppercase macros: `CX_UPPER_CASE`.
- Internal implementation macros: `CX__UPPER_CASE` or `cx__snake_case`.
- Semantic keyword aliases are exceptions to ordinary macro naming conventions.
- Private functions may use ordinary names when marked `priv`; a leading underscore is not required.
- Prefer names that describe semantics rather than implementation mechanisms.
- Avoid unnecessary suffixes such as `_native`, `_raw`, or `_impl` when they do not add a useful distinction.
- Use `buf` consistently for array-like buffer members where appropriate.
- Prefer idiomatic operation names such as `reserve`, `resize`, and `shrink`.
- Use `b32` for ordinary boolean parameters, return values, local variables, and fields; use `b8` or `b16` where storage size matters. A possible future `b64` default is not yet adopted.
- Keep C++ `bool` where needed for metaprogramming (`proposition`, concepts, boolean template parameters, and compile-time trait helpers/constants), contextual `operator bool` conversions, and low-level C compatibility definitions.
- Built-in logical/comparison expressions still produce C++ `bool`; the convention governs ordinary declared interfaces and storage.
- Provide an explicit `operator!=` where needed for `b32` equality operators; C++'s rewritten `!=` candidates require `operator==` to return exactly `bool`.

## 5. Basic formatting

- Use four spaces for indentation.
- Use a general maximum line length of 100 columns.
- Short trait and proposition declarations may slightly exceed 100 columns when keeping them inline improves readability.
- Follow the established surrounding style when a rule does not explicitly cover the situation.
- Use east-const syntax: `T const&`, `T const*`.
- Preserve compact formatting when readability is not compromised.
- Keep simple boolean expressions and loop headers inline.
- Avoid unnecessary line breaks.
- Avoid excessive whitespace and decorative formatting.
- Align only related arguments, parameters, enum entries, enum-like constants, and struct members.
- Do not align unrelated local variables or arbitrary expressions.
- Use two spaces between aligned type/name or name/default columns where applicable.
- Align default argument values consistently with the surrounding parameter declarations.
- Keep related declarations visually consistent.

## 6. Bracing conventions

- Function bodies begin on a new line, including short functions.
- `struct`, `class`, `union`, and `enum` opening braces begin on a new line.
- Namespace opening braces remain on the declaration line.
- Control-statement braces remain on the control-statement line when the condition fits.
- If a control condition wraps across multiple lines, place the opening brace on its own line.
- Keep `else`, `catch`, and `while` attached to the preceding closing brace according to the agreed style.
- Case labels do not require a separate brace style.
- Lambda bodies use compact bracing.
- Trait and proposition definitions may use specialized inline or braced-expression formatting.
- Empty functions retain their opening brace on a separate line.
- Empty records may remain compact.
- Avoid mechanically applying clang-format rules when they conflict with the established custom conventions.

## 7. Function signatures

- Use the conventional libcx signature structure with semantic keywords and trailing return types.
- Keep the entire function signature on one line when it fits within 100 columns.
- If only the `with` clause makes the signature exceed the limit, move that clause to a separate line.
- If the rest of the signature is too long, move parameters onto separate lines.
- Keep the opening parenthesis attached to the function name.
- Place the closing parenthesis and following qualifiers on the same line when possible.
- If constraints still do not fit, place `with` on a separate line.
- If necessary, break complex constraints across multiple lines with consistent indentation.
- If template parameters are too long, expand the `template<...>` declaration across multiple lines.
- Maintain the specified order of formatting transformations rather than wrapping arbitrarily.
- Preserve consistent indentation of continuation lines.
- Always keep the function body opening brace on the following line.
- Use `noexce` and other semantic macros according to the existing API conventions.

## 8. Templates and constraints

- Put ordinary `template` declarations on their own line.
- Keep short template parameter lists inline.
- Expand template parameter lists when necessary.
- Keep constraints outside the template angle brackets using the established `with` syntax when appropriate.
- Preserve concise constrained-template syntax where it already expresses the requirement clearly.
- Keep short constraints inline.
- Break complex constraints only when needed for readability or line length.
- For multiline struct templates, use the agreed layout with the expanded template parameter list and its constraints.
- Traits and concepts may place `template` and the declaration on the same line.
- Avoid unnecessary template metaprogramming complexity.
- Prefer existing traits and concepts over new ad hoc constraint mechanisms.
- Do not duplicate existing static requirements as runtime checks or documentation preconditions.

## 9. Traits and concepts

- Use `proposition` for trait predicates.
- Use `concept` for semantic type constraints.
- Prefer compiler intrinsics where they directly express a required low-level property.
- Define higher-level traits by composition of existing traits.
- Define concepts through the corresponding traits.
- Preserve established variadic concepts and fold-expression conventions.
- Keep simple propositions inline.
- Keep compositions of simple traits inline when readable.
- Use multiline braced proposition initialization primarily when `requires` expressions introduce visual noise.
- Do not use multiline braced formatting merely because several simple traits are combined.
- Keep simple concept declarations inline.
- Keep static constraints out of runtime `@req` documentation.
- Avoid redundant or speculative concepts.
- `PassiveValue` represents value-initializable, trivially copyable, trivially destructible types.
- `PassiveValue` does not require a trivial default constructor.
- Default member initializers are compatible with the intended `PassiveValue` model.
- Prefer passive types throughout libcx containers and data structures.

## 10. Semantic macros and language utilities

- Preserve existing aliases such as `fn`, `cons`, `inln`, `noexce`, `onedef`, and `priv`.
- Prefer `fn` for ordinary function declarations.
- Use `func` for function types where appropriate.
- Use `onedef` for variable definitions.
- Avoid replacing semantic macros with ordinary C++ syntax without a concrete reason.
- Use libcx's existing casting, forwarding, ownership, and type-utility facilities.
- Prefer existing libcx utilities over unnecessary standard-library substitutions.
- Keep compiler-specific facilities encapsulated or clearly identified where appropriate.
- Treat `cx_assume` as an optimization facility, not as error handling or input validation.
- Use optimization assumptions only when their preconditions are genuinely established.

## 11. Namespaces and modules

- `namespace cx` is the primary public namespace.
- Modules normally use `inline namespace <module>`.
- Foundational traits, concepts, and comparable utilities may be declared directly in `namespace cx`.
- Do not introduce unnecessary nested namespaces.
- Free functions exposed within `cx` do not need a `cx_` prefix.
- Keep module names singular.
- Keep module boundaries cohesive.
- Avoid splitting tightly related functionality across numerous files.
- Preserve intentional public and internal header organization.
- Prefix entirely embryonic/prototype module directories with `__`; in otherwise usable modules, prefix only the individual prototype files. Do not redundantly prefix every file inside an already marked prototype directory.
- Name archived implementations `__<name>_old.<extension>`, preserving their extension. Update includes, path comments, and include guards after renaming.
- Prototype path markers indicate work status, not new C++ namespaces or a claim that other files are finished.
- Keep lower-level functionality independent of higher-level modules.

## 12. Parameter passing

- Pass small, inexpensive values by value.
- Pass larger read-only objects using `T const&`.
- Pass mutable objects using `T&`.
- Use pointers for raw memory, buffers, pointer arithmetic, and low-level operations.
- Use pointers when nullability is meaningful.
- Use pointers when changing the referenced address or manipulating ownership is part of the operation.
- Prefer ordinary references for non-null object access rather than emulating references with annotated pointers.
- Use east-const consistently.
- Avoid unnecessary copies of large objects.
- Do not use references or pointers purely out of ideological preference; follow their actual semantics.
- Container operations normally take a mutable or const reference to the container as their first argument.
- Factory functions naturally omit the container argument because they create it.

## 13. Data structures and ownership

- Use plain aggregates whenever practical.
- Keep structures primarily as data representations.
- Avoid ordinary member methods.
- Avoid user-defined constructors and destructors for passive types.
- Avoid implicit copy/move ownership semantics.
- Prefer explicit initialization and cleanup functions.
- Use external `make`, `init`, `free`, or `deinit` functions where appropriate.
- Structure-specific cleanup must explicitly release owned resources.
- Trivial destruction does not imply that externally owned resources have been released.
- Copying a passive structure containing pointers is a shallow copy unless an explicit operation performs deep copying.
- Avoid automatic recursive cleanup of nested resources.
- Preserve clear ownership contracts for aggregates and containers.
- Do not add default member initializers when a structure's API deliberately requires explicit initialization.
- Preserve simple, predictable member layouts.

## 14. Memory management

- Allocation, byte zeroing, value initialization, copying, relocation, and deallocation are distinct operations.
- Use the allocator API for explicit memory management.
- Maintain appropriate alignment for allocated storage.
- Use the existing alignment constants and utilities.
- Respect allocator-specific supported and unsupported operations.
- Prefer passive-value byte copying through libcx memory utilities where semantically valid.
- Do not use placement-new unnecessarily for passive types.
- Keep optional custom placement-new support disabled by default.
- Do not introduce `<new>` solely for placement construction.
- Do not add an artificial placement-delete abstraction.
- Retain custom placement new and matching delete, disabled by default. Do not use them unless explicitly requested; keep a `NOTE:(manu)` beside the definitions stating this restriction.
- Use a single initialized template value and replicate it when appropriate for generic passive-value initialization.
- `init_value` uses a `T{}` template value.
- `init_args` uses a `T{args...}` template value.
- `init_list` copies or converts the supplied list elements.
- Generic initialization helpers may return `void` when they have no recoverable failure modes.
- Use soft assertions for their runtime preconditions.
- Do not confuse a zero-filled representation with `T{}`.
- Preserve default member initializer semantics during value initialization.
- Avoid unnecessary double initialization or zeroing passes.
- Respect C++ object-lifetime requirements even when exposing a C-like programming model.
- Use explicit cleanup for resources rather than generic destructor-based cleanup.
- Propagate genuine allocation failures through the existing error API.

## 15. Container API conventions

- Use free functions for container operations.
- Keep container structures as passive aggregates.
- Separate creation, capacity management, logical length management, and deallocation.
- `make` allocates storage and initializes container metadata.
- Zero-filled allocation may be the default when explicitly established for a container.
- Permit allocation flags to control optional zeroing.
- `reserve` manages capacity without changing logical length.
- `resize` changes logical length and initializes newly added values according to the documented semantics.
- `shrink` reduces capacity where supported without changing logical length.
- `free` explicitly releases backing storage.
- Preserve valid container state after recoverable allocation failures.
- Preserve existing values during reallocation.
- Respect allocator limitations rather than concealing unsupported operations.
- Define representation invariants through helpers such as `rep_ok`.
- Do not require private invariant helpers to have leading underscores.
- Preserve minimum-capacity policies where established, such as 16 elements for `DynamicArray`.
- Keep operator overloading minimal.
- Keep iterator-related member functions minimal until the iterator API is reviewed.

## 16. Error handling

- Use explicit Go/Odin-inspired error handling.
- Represent recoverable failures through `ErrorCode`.
- Use `Res<Ts...>` for functions returning values alongside errors.
- Prefer `or_return` and `or_else` where applicable.
- Avoid unnecessary explicit `if (err)` branches.
- Use ordinary branches when the macros cannot express the required behavior correctly.
- Do not introduce alternative error-handling frameworks.
- Do not revive the old experimental `ResultPair`/error-pointer model.
- Do not automatically panic on ordinary allocation failures.
- Let callers decide whether a recoverable failure should become fatal.
- Distinguish programmer contract violations from legitimate runtime failures.
- Prefer explicit contracts to defensive programming.
- Avoid speculative checks for implausible inputs.
- Preserve state and resource ownership when recovering from failures.

## 17. Assertions, diagnostics, and optimization

- `cx_assert` is a soft assertion, removable in release builds.
- `cx_debug` is a debug facility that may be disabled in release.
- `cx_assert_hard` remains active in release.
- `cx_panic` remains active in release.
- Use `cx_assert` to verify programmer preconditions during development.
- Use `cx_assert` to verify representation invariants and internal assumptions.
- Soft assertions may replace runtime branching for conditions guaranteed by the caller's contract.
- Do not use soft assertions as the sole handling of genuinely recoverable runtime failures.
- Use hard assertions or fatal handling when failure must remain observable in release.
- Use `cx_assume` only to communicate established facts to the optimizer.
- Do not treat `cx_assume` as a runtime check.
- Do not assume that an internal representation-invariant assertion creates a caller precondition.
- Keep assertions and documentation semantically consistent.

## 18. `none`, `null`, and empty values

- Preserve distinct semantic uses of `none` and `null`.
- Use `null` for null pointers and absent addresses.
- Use `none` for successful error results or semantically absent numeric values.
- Both currently originate from `ZeroType`, so the distinction is semantic rather than type-enforced.
- Rename the separate `NoneType` type to `EmptyType`.
- Rename the corresponding `None` value to `Empty`.
- Preserve `ZeroType` for zero/null conversions.
- Avoid introducing unnecessary zero-valued enum cases when a suitable absence representation already exists.
- Follow established API semantics for success and error codes rather than mixing conventions arbitrarily.

## 19. Function documentation

- Use compact Doxygen-like `/** ... **/` blocks.
- Indent contents by four spaces.
- Do not prefix internal documentation lines with `*`.
- Begin with a concise description of the function's behavior.
- Use backticks around parameter names, identifiers, and expressions.
- Use compact lists for documented sections.
- Avoid unnecessary blank lines between sections.
- Use consistent punctuation, generally ending descriptive list items with a period.
- Use `@par` for template parameters.
- Use `@arg` for ordinary function arguments.
- Use `@req` for runtime caller preconditions.
- Use `@ret` for return values and meaningful recoverable errors.
- Use `@inf` for useful additional information.
- Omit irrelevant or empty sections.
- Omit `@ret` for `void` functions.
- Allow a minimal one-line description for trivial functions.
- Keep documentation proportional to API complexity.
- Avoid mechanically documenting obvious details.

## 20. Precise `@req` semantics

- `@req` documents **runtime conditions that the caller must guarantee**.
- `@req` does not document compile-time constraints.
- Do not include concept requirements, trait constraints, `static_assert` conditions, or type-system guarantees.
- Do not include conditions that the function checks and handles by returning a recoverable error.
- Do not include conditions handled through `cx_assert_hard` as ordinary caller preconditions.
- Include runtime preconditions verified with soft `cx_assert`.
- Include runtime preconditions that cannot be checked locally, such as sufficient accessible storage.
- Soft assertions checking caller preconditions must agree with the documented `@req` conditions.
- The assertion may express the inverse of an invalid condition, but it must enforce the same contract.
- Assertions on internal invariants or postconditions do not automatically belong in `@req`.
- Do not repeat the same condition under multiple documentation sections.
- Distinguish documented caller requirements from implementation checks.
- Keep the contract accurate even when soft assertions are disabled in release.
- Do not add trivial static facts merely to make the documentation look complete.

## 21. Return-value documentation

- Use `@ret` when the return value or recoverable failure behavior requires explanation.
- Describe successful results concisely.
- Describe meaningful recoverable errors where helpful.
- Avoid listing every trivial error mechanically.
- Never put a handled error condition in `@req`.
- Avoid repeating an error condition in both the introductory description and `@ret`.
- Prefer the location that communicates the behavior most clearly.
- Do not document `void` returns.
- Keep return-value documentation aligned with the actual API semantics.

## 22. Struct documentation

- Use the same compact `/** ... **/` presentation as functions.
- Begin with a concise description of the represented type.
- Use `@par` for template parameters.
- Use `@rep` to describe members and representation.
- Use `@inv` for meaningful representation invariants.
- Use `@inf` for additional information.
- Do not use `@arg` for struct fields.
- Do not use `@req` as the primary struct-invariant section.
- Do not use `@ret` for structs.
- Do not introduce `@ope` merely to enumerate available free functions.
- Document operations individually at their free-function declarations.
- Include only relevant sections.
- Simple structs may need only a brief description.
- Describe ownership and layout invariants when important.
- Avoid duplicating the documentation of related operations.

## 23. Ordinary comments

- Use `//` for ordinary comments.
- Keep comments concise and useful.
- Prefer comments above the relevant code.
- Use inline comments only when they add meaningful information.
- Separate inline comments from the preceding code by exactly two spaces.
- Avoid unnecessary commentary describing self-evident statements.
- Avoid decorative separator comments made of repeated hyphens or other characters.
- Do not use comments as substitutes for clear naming.
- Prefer comments that explain assumptions, rationale, or non-obvious behavior.
- Avoid stale or speculative commentary.

## 24. Annotation comments

- Use established annotations such as `TODO:(manu)`, `NOTE:(manu)`, `FIX:(manu)`, and `XXX:(manu)`.
- Preserve the colon immediately after the annotation keyword.
- Preserve the established author suffix where appropriate.
- Prefer annotations above the affected code.
- Use annotations for significant pending work, problems, or important observations.
- Avoid annotation comments for trivial reminders.
- Preserve formatting compatible with Neovim's TODO-comment detection.
- Keep multiline annotations concise and consistently indented.

## 25. Header organization

- Use `.hh` headers.
- Use include guards.
- Include direct dependencies explicitly.
- Avoid relying on transitive includes.
- Keep module dependencies appropriately layered.
- Prefer cohesive headers over excessive splitting.
- Avoid artificial section dividers.
- Order definitions according to logical dependency and API responsibility.
- Place constants and fundamental declarations early.
- Follow with data structures, aliases, and relevant traits/concepts.
- Place equality and comparison operators after the associated structure.
- Follow with representation checks and the principal free-function API.
- Within container APIs, place factories and memory-management operations first.
- Follow with ordinary operations such as insertion, removal, modification, and search.
- Place miscellaneous helpers and utilities afterward where appropriate.
- Retain test-related code only where the repository's established organization warrants it.
- Keep namespace and include-guard closure clear and consistent.
- **At the end of a header belonging to a module, align the trailing `// namespace ...` comments and the `#endif` guard comment in the same column.** Specifically, the `//` begins in column 9 (seven spaces after `}` and two after `#endif`), as in:

  ```cpp
  }       // namespace mem
  }       // namespace cx
  #endif  // CX_MEM_ALLOCATOR_HH
  ```

## 26. Operators and iterators

- Avoid unnecessary operator overloading.
- Prefer free-function equality and three-way comparison operators.
- Avoid `friend` operators unless a compelling technical requirement exists.
- Do not use operators to introduce object-oriented abstractions.
- Keep the minimal iterator-support members currently necessary for container interoperability.
- Treat the existing iterator exception as provisional.
- Revisit iterator, range, and traversal API design separately.
- Do not redesign iterators as an unrelated side effect of other refactors.

## 27. Workflow and agent behavior

- Treat the established conventions as authoritative when editing libcx.
- Use existing libcx implementations as references, while recognizing that some older code may not yet follow the latest conventions.
- Respect explicit decisions over stale examples.
- Keep changes focused on the requested task.
- Do not introduce unrelated refactoring.
- Avoid unnecessary dependencies and abstractions.
- Ask before changing public APIs or making broad structural changes when clarification is required.
- Verify affected code with targeted compilation or tests where practical.
- Do not edit unrelated build, generated, vendor, or configuration files without a reason.
- Do not produce lengthy reports unless requested.
- Update `AGENTS.md` when explicitly established, reusable conventions change.
- Keep general conventions in the dedicated styling and conventions section.
- Preserve project-specific workflow instructions separately.
- Distinguish mandatory rules from stylistic preferences.
- Use concrete examples where they prevent ambiguity.
- Avoid turning individual implementation decisions into universal restrictions.
- Keep documentation and examples synchronized with the actual library architecture.
- Prioritize the user's explicitly established design decisions over generic programming advice.
- Do not use the global `CX_TEST` switch. Use meaningful contextual `CX_TEST_<CONTEXT>` flags for important structures, enabled through compiler flags only; headers must not enable their own tests.
- Use one simple executable per module, for example `arr/test_array.cc`, calling cases through `CX_TEST_DEFINE`, `CX_TEST_CASE`, and `CX_TEST_RUN`.
- Local tests validate basic behavior and caller-side interfaces; broader testing remains in `./test`.
- Correct existing documentation for now; do not add missing documentation or specifications unless requested.

## 28. Established references and pending decisions

- Primary existing code reference: `include/libcx/arr/`.
- Additional architectural references: Odin standard library and Odin compiler C++ source.
- `__conventions_and_styling.md` is the detailed historical reference; explicitly established later decisions in this checklist and `AGENTS.md` supersede conflicting older rules.
- `AGENTS.md` is the consolidated operational guide for coding agents.
- The `EmptyType`/`Empty` rename is established but may still require application throughout the source.
- The iterator and range APIs require a separate design review.
- Some allocator behavior, particularly physical shrinking, remains implementation-dependent.
- Keep the existing `<initializer_list>` dependency for now; replacing it is deferred.
- Existing source files may require gradual migration to the new documentation conventions.
- Unresolved implementation questions should remain explicitly provisional rather than being presented as finalized global rules.

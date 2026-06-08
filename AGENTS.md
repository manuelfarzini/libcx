# libcx

libcx is a minimal C++23 library intended as an alternative to (much of) the standard library, with a C-like
style and a "C with concepts" design.

## Scope

- Focus on the core library code only.
- Prefer working in `src/` and `include/`.
- Do not inspect or modify `test/`, `extern/`, `build/`, or `.cache/`.
- Do not inspect or modify `.clang-format`, `Doxyfile`, or `Doxyfile.bck`.

## Style

- Prefer procedural style.
- Prefer free functions over methods.
- Keep allocation patterns explicit.
- Do not add dependencies unless explicitly requested.
- Keep changes small and local.
- Ask before changing public-facing APIs or broad project structure.

# Contributing to rapidyenc

Thanks for contributing. Keep changes narrowly scoped, preserve the C API and
runtime dispatch behavior, and validate changes on the architectures they
affect.

## Prerequisites

- CMake 2.8.9 or newer
- A C compiler and a C++ compiler with C++98 support
- Git

Configure and build a redistributable Release build before opening a pull
request:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
printf 'hello yenc\n' | ./build/rapidyenc_cli e | ./build/rapidyenc_cli d | cmp - <(printf 'hello yenc\n')
```

On multi-configuration generators, the CLI may be in `build/Release/`.
Do not set `BUILD_NATIVE=ON` when validating release artifacts: it intentionally
optimizes for the build host and is not redistributable.

## Pull requests

1. Open an issue before large API, ABI, architecture, or release changes.
2. Preserve public API and ABI compatibility unless the breaking change is
   intentional, documented, and versioned accordingly.
3. Update public documentation when behavior, build options, or release assets
   change.
4. Add deterministic tests when introducing a testable behavior. Until a test
   suite exists, include a focused CLI round-trip and relevant platform build
   validation.
5. For SIMD or performance changes, identify the affected CPU/compiler matrix
   and include benchmark evidence where practical.
6. Do not commit credentials, tokens, generated binaries, build directories,
   or private data.

## Commits and releases

Use focused Conventional Commit subjects where practical:

- `feat(scope): ...` triggers a minor release.
- `fix(scope): ...` triggers a patch release.
- `chore(scope): ...` and `docs(scope): ...` do not trigger a release.
- Use `fix(deps): ...` for Dependabot updates.

Useful scopes include `encode`, `decode`, `crc`, `simd`, `cmake`, `ci`, `deps`,
and `docs`. Mark breaking changes with `!`, for example `feat(api)!: ...`, and
include a `BREAKING CHANGE:` footer.

Release Please maintains the release pull request, `CHANGELOG.md`,
`version.txt`, and the immutable `vX.Y.Z` tag. Maintainers should not manually
move release tags.

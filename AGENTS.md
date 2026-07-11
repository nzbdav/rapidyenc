# rapidyenc Agent Guide

## Purpose and stack

- rapidyenc is a C-compatible native library for yEnc encode/decode and CRC32.
- The public C API is `rapidyenc.h`; `rapidyenc.cc` contains its C++ wrappers.
- CMake is the only build system. Library code targets C++98; `tool/bench.cc`
  uses C++11.
- The project is CC0. `crcutil-1.0/` is a vendored Apache-2.0 dependency.

## Repository layout

- `src/`: runtime platform detection and ISA-specific encoder, decoder, and
  CRC implementations.
- `rapidyenc.h`: public ABI, kernel identifiers, and `0xMMmmpp` version macro.
- `rapidyenc.cc`: exported C API wrappers.
- `tool/cli.c`: incremental C API example; `tool/bench.cc`: benchmark.
- `crcutil-1.0/`: vendored x86 CRC implementation. Do not update it casually.
- `.github/`: CI, security, release, and dependency automation.

## Architecture and invariants

- Call each relevant `rapidyenc_*_init()` once before that subsystem is used.
  Initializers mutate global state and are not thread-safe; other API calls are
  thread-safe after initialization.
- rapidyenc performs raw encoding/decoding only. Header parsing and
  application-level validation belong to the caller.
- Runtime dispatch in `src/platform.cc` selects the best supported kernel.
  Preserve the generic fallback and register any new ISA implementation in the
  appropriate dispatch and CMake source/compile-flag paths.
- Keep library code compatible with C++98 and preserve the no-exceptions,
  no-RTTI build assumptions.
- `BUILD_NATIVE=ON` is for local performance experiments only; release and
  redistributable builds must use `OFF`.
- Public function signatures, enum values, structures, and kernel identifiers
  are ABI-sensitive. Isolate intentional API/ABI breaks and document them.

## Development workflow

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
printf 'hello yenc\n' | ./build/rapidyenc_cli e | ./build/rapidyenc_cli d | cmp - <(printf 'hello yenc\n')
./build/rapidyenc_bench
```

On multi-configuration generators, tools may be under `build/Release/`.
There is no automated unit-test suite yet; add deterministic CTest coverage
when changing behavior that can be tested. For SIMD work, build and benchmark
the affected CPU/compiler combinations without changing unrelated ISA files.

## Versioning and releases

- `version.txt` is the semantic version maintained by Release Please.
- `RAPIDYENC_VERSION` remains `0xMMmmpp`; its component macros in
  `rapidyenc.h` are updated by Release Please and must stay synchronized.
- `CHANGELOG.md`, `.release-please-manifest.json`, and version macros are
  release-managed. Do not hand-edit them outside a release bootstrap or repair.
- Releases are immutable `vX.Y.Z` tags and attach shared-library archives for
  Linux x64/arm64, Windows x64, and macOS x64/arm64.
- RapidYencSharp vendors rapidyenc source and packages native assets. Consider
  its integration when public APIs or release artifacts change.

## Commit and Git safety

- Use one logical change per commit and scoped Conventional Commits:
  `feat(scope):`, `fix(scope):`, `chore(scope):`, or `docs(scope):`.
- Typical scopes: `encode`, `decode`, `crc`, `simd`, `cmake`, `ci`, `deps`,
  and `docs`. `feat` releases minor, `fix` releases patch, and `fix(deps)` is
  used for Dependabot updates.
- Mark breaking changes with `!` and a `BREAKING CHANGE:` footer.
- Only commit when explicitly requested. Never skip hooks, force-push `main`,
  or move release tags. Amend only an unpushed commit created in the same
  session when explicitly requested or required by a hook.

## Start here

Read `README.md`, `rapidyenc.h`, `rapidyenc.cc`, `CMakeLists.txt`,
`src/platform.cc`, the relevant ISA file, and `tool/cli.c` before changing
library behavior.

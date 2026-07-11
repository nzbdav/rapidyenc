# rapidyenc

[![CI](https://github.com/nzbdav/rapidyenc/actions/workflows/ci.yml/badge.svg)](https://github.com/nzbdav/rapidyenc/actions/workflows/ci.yml)
[![CodeQL](https://github.com/nzbdav/rapidyenc/actions/workflows/codeql.yml/badge.svg)](https://github.com/nzbdav/rapidyenc/actions/workflows/codeql.yml)
[![Release](https://img.shields.io/github/v/release/nzbdav/rapidyenc)](https://github.com/nzbdav/rapidyenc/releases)

High-performance, C-compatible [yEnc](http://www.yenc.org/yenc-draft.1.3.txt)
encoding, decoding, and CRC32 routines with runtime SIMD dispatch. rapidyenc is
maintained by [nzbdav](https://github.com/nzbdav) and is intended for consumers
that need a small native library with predictable, incremental processing.

Note that it only handles the underlying encoding/decoding routines - yEnc headers aren’t handled.

## Features

- Runtime CPU detection and dispatch across x86, ARM, and RISC-V.
- SIMD implementations for ARMv7 NEON, ARMv8 ASIMD, x86 SSE2/SSSE3/AVX/AVX2,
  AVX512-BW/VBMI2 (and compatible AVX10.1/256 targets), and RISC-V RVV where
  supported by the compiler and CPU.
- Incremental encoding and decoding, including yEnc and NNTP article-end
  detection in the decoder.
- Raw yEnc encoding with configurable line lengths.
- CRC32 through [crcutil](https://code.google.com/p/crcutil/), x86 PCLMULQDQ,
  ARM CRC/PMULL, or RISC-V Zbkc acceleration when available.
- CRC32 combination, zero-byte extension/inversion, multiplication, and powers.

## Requirements

- CMake 3.12 or newer.
- A C compiler and C++ compiler. Library code targets C++98; the benchmark
  additionally requires C++11 support.
- No downloaded build dependencies. `crcutil-1.0/` is a vendored, x86-only
  acceleration dependency.

## Build

Use an out-of-tree Release build:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

This produces a static library, a shared library (unless disabled), and:

- `rapidyenc_cli`, a stdin-to-stdout encode/decode example.
- `rapidyenc_bench`, a local performance benchmark.

## Build Options

The following options can be passed into CMake:

- **BUILD_NATIVE**: optimise only for the build host's CPU. Do not use this for
  distributable packages or release assets.
- **DISABLE_AVX256**: disable 256-bit AVX instructions on x86 processors.
- **DISABLE_CRCUTIL**: disable crcutil (it is only considered on x86).
- **DISABLE_ENCODE**: omit encoding functions, except
  `rapidyenc_encode_max_length`.
- **DISABLE_DECODE**: omit decoding functions.
- **DISABLE_CRC**: omit CRC32 functions; also disables crcutil.
- **DISABLE_TOOL**: do not build the CLI and benchmark tools.
- **DISABLE_SHARED**: do not build the shared library.

## API

Before any encoding/decoding/CRC functions can be used, the respective `_init` function must be called. These functions set up the necessary state for computation. Note that `_init` functions aren’t thread-safe, but all others are.

Functions documented in the [header file](rapidyenc.h).

[cli.c](tool/cli.c) is a simple command-line application which encodes/decodes stdin to stdout. It demonstrates how to do incremental encoding/decoding/CRC32 using this library.

### Minimal C example

```c
#include "rapidyenc.h"

rapidyenc_encode_init();
size_t required = rapidyenc_encode_max_length(input_length, 128);
size_t written = rapidyenc_encode(input, output, input_length);
```

`output` must have at least `required` bytes. For incremental streams, use
`rapidyenc_encode_ex` and the decoder state APIs described in
[`rapidyenc.h`](rapidyenc.h).

## Releases and versioning

Releases use semantic version tags (`vX.Y.Z`) and are generated from
Conventional Commits by Release Please. The public header exposes the same
version as `RAPIDYENC_VERSION` in `0xMMmmpp` form.

Each GitHub release provides source archives and shared-library archives for
`linux-x64`, `linux-arm64`, `win-x64`, `osx-x64`, and `osx-arm64`. Release
archives contain `rapidyenc.h`, the platform shared library, license material,
and a checksum manifest. Download them from
[Releases](https://github.com/nzbdav/rapidyenc/releases).

## Other language bindings

* [node-yencode](https://github.com/animetosho/node-yencode): for NodeJS/Bun
* [go-yencode](https://github.com/mnightingale/go-yencode): for Golang
* [sabctools](https://github.com/sabnzbd/sabctools): for Python (Sabnzbd specific binding)
* [RapidYencSharp](https://github.com/nzbdav/RapidYencSharp): for C#/.NET

## Development

See [CONTRIBUTING.md](CONTRIBUTING.md) for build, validation, and pull request
guidance. Report security vulnerabilities through the private process described
in [SECURITY.md](SECURITY.md). Changes in released versions are recorded in
[CHANGELOG.md](CHANGELOG.md).

## Algorithm

A brief description of the SIMD yEnc encoding algorithm
[is available here](https://github.com/animetosho/node-yencode/issues/4#issuecomment-330025192).

## License

This module is Public Domain or [CC0](https://creativecommons.org/publicdomain/zero/1.0/legalcode) (or equivalent) if PD isn’t recognised.

[crcutil](https://code.google.com/p/crcutil/), used for CRC32 calculation, is licensed under the [Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0).

[zlib-ng](https://github.com/Dead2/zlib-ng), from where the CRC32 calculation using folding approach was derived, is under a [zlib license](https://github.com/Dead2/zlib-ng/blob/develop/LICENSE.md).

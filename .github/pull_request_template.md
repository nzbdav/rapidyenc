## Summary

- What changed?
- Why is the change needed?

## Validation

- [ ] `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release`
- [ ] `cmake --build build --config Release`
- [ ] CLI encode/decode round trip completed.
- [ ] Relevant CPU/compiler platforms were built or documented.
- [ ] Documentation and deterministic tests are updated where needed.
- [ ] No credentials, tokens, generated binaries, or private data are included.

## Compatibility and performance

Describe public API, ABI, platform, SIMD dispatch, release-asset, or performance
impact. Identify intentional breaking changes.

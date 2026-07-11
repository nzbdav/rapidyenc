# Security Policy

## Supported versions

Security fixes are provided for the latest released version. Upgrade to the
newest release before reporting an issue that may already be resolved.

## Reporting a vulnerability

Do not open a public issue for a suspected vulnerability.

Use [GitHub private vulnerability reporting](https://github.com/nzbdav/rapidyenc/security/advisories/new)
to provide affected versions, a minimal reproduction, impact, and any proposed
mitigation. We will acknowledge the report within seven days and coordinate
validation, remediation, and disclosure through the private advisory.

If private reporting is unavailable, open a discussion requesting a private
contact channel without including vulnerability details.

## Security expectations

Release binaries are built from immutable release tags with `BUILD_NATIVE=OFF`.
Do not rely on a CPU-specific build for portability, and do not treat
untrusted encoded input as validated application protocol data: rapidyenc
does not parse yEnc headers or enforce application-level size limits.

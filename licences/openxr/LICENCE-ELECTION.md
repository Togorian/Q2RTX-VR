# OpenXR 1.1.62 licence election and origin

Q2RTX VR uses the offered MIT alternative for eligible Khronos OpenXR headers, loader code and generated loader sources, and the MIT terms offered for JsonCpp. Original copyright, attribution and dual-licence text remain intact, including the accompanying Apache texts. This election covers only material offering that choice; Apache-only files keep their own notices. It does not change the combined engine's GPLv2 distribution basis.

The preserved loader source is the named Khronos OpenXR-SDK-Source release 1.1.62 archive at `dependencies/archives/OpenXR-SDK-Source-release-1.1.62.tar.gz` in the corresponding-source tree. It includes registry/generators and JsonCpp. The public build instructions describe extracting that release and building the loader target; no unversioned SDK-main archive is selected.

The Windows x64 `openxr_loader.dll` is 2,669,568 bytes, SHA-256 `c03b44a09072dc276aabe5b390f642054fac83b38c1c575fcfce56f0efc37ea4`. It equals the x64 member of the official 1.1.62 binary release archive whose published SHA-256 is recorded in `docs/DEPENDENCIES.md`. This byte comparison does not establish a verified signature or a reproducible local rebuild. The earlier signature attempt did not verify a signer; no successful signature verification is claimed.

COPYING.adoc explicitly covers generated loader sources under the offered dual licence. Where generated files also carry older boilerplate, both notices remain. Retain COPYING.adoc, binary-package-LICENSE.txt, MIT.txt, upstream-Apache-LICENSE.txt, JsonCpp notices, generator notices and the original source-file headers.

See the root PROVENANCE.md and docs/DEPENDENCIES.md for the library selection, origin evidence and remaining verification limits.

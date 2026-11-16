# RC3 connection identifier correction

The narrow 0.8.4-rc.4 candidate separates the network client identifier from the displayed product label. RC3 sent a version userinfo value beginning with "Q2RTX VR", while the existing server restriction accepts the lowercase "q2rtx" prefix. This could prevent both New Game and save loading from reaching the local server.

VERSION supplies both identities. The generated Q2RTX_CLIENT_VERSION begins with "q2rtx"; Q2RTX_PRODUCT_LABEL retains "Q2RTX VR". Resources, console labels, launcher and logs still derive from that version. The version cvar keeps its serverinfo, userinfo and read-only flags. Server restriction, authentication and protocol checks are unchanged; no setting or save migration is involved.

The isolated CPU fixture under connection/checks compiles the actual selected cvar, shared userinfo and hash utility C sources. Its generated include contains exact pinned declarations, registration statement and existing server restriction block from selected production sources. It executes real Cvar_Get, Cvar_BitInfo, Info_SetValueForKey and Info_ValueForKey. Only allocation, diagnostics and inactive prompt/client-effect boundaries are stand-ins. Unrelated command/file imports from the complete cvar source trap if called; none are used by the checked path.

From the native x64 MSVC environment, configure connection/checks in a fresh separate directory with CMake/Ninja and Python, build connection_checks serially, then run it with the expected VERSION and observed RC3 version field as two arguments. Cases cover new userinfo/serverinfo propagation, refused RC3 value, existing compatible values, and incompatible, empty, uppercase and short values.

This fixture does not start an engine/server, send traffic, read a user home/save or exercise graphics/headsets. It verifies this compatibility boundary. Actual native product outcomes belong to release-build.json or packaged BUILD-RECORD.json. The broader deferred runtime/controller/recovery work is outside this correction.

Historical record: 11 November 2026. The RC3/RC4 strings retained in `connection/checks/checks.c` are deliberate legacy rejection cases, not the current product version.

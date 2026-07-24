# Provenance

Where DuskScreen and its dependencies come from — the fork lineage, the pinned
upstream revisions, and the local patches carried on top of each.

## Graph

```mermaid
graph TD
    %% ── DuskScreen fork lineage ──
    LS["ckaiser/Lightscreen<br/>GPL-2.0-or-later<br/>© 2008–2021 Christian Kaiser"]
    DS["DuskScreen (this repo)<br/>GPL-2.0-or-later<br/>Qt 6 port + slim-down © 2026 Myggiz"]
    LS -->|"fork: Qt 6 port + slim-down"| DS

    %% ── SingleApplication dependency ──
    SA["itay-grudev/SingleApplication<br/>v3.5.6 · MIT<br/>© Itay Grudev 2015–2023"]
    SA -->|"wrap-git @ a218603 + Meson overlay"| DS

    %% ── UGlobalHotkey lineage ──
    PX["bakwc/Pastexen<br/>origin of the hotkey code"]
    FE["falceeffect/UGlobalHotkey<br/>standalone Qt lib · Public Domain<br/>⚠ repo deleted (404)"]
    CK["ckaiser/UGlobalHotkey<br/>Public Domain<br/>(Lightscreen-flavoured fork)"]
    PX --> FE --> CK
    CK -->|"wrap-git @ 231b101 + Qt 6 diff_files patch"| DS

    classDef dead stroke-dasharray:5 5,stroke:#c0392b,color:#c0392b;
    classDef repo stroke-width:2px;
    class FE dead;
    class DS repo;
```

## Components

| Component | Role | Upstream | Pinned revision | License | Local patches |
|---|---|---|---|---|---|
| **DuskScreen** | This application | fork of [ckaiser/Lightscreen](https://github.com/ckaiser/Lightscreen) | — | GPL-2.0-or-later | Qt 6 port, slim-down (see [README](README.md)) |
| **SingleApplication** | Single-instance guard + message forwarding | [itay-grudev/SingleApplication](https://github.com/itay-grudev/SingleApplication) (v3.5.6) | `a218603d76a55dddb65b84f2b49ecaa8efc074ea` | MIT | Meson build overlay only (upstream ships CMake + qmake) |
| **UGlobalHotkey** | System-wide global hotkeys | [ckaiser/UGlobalHotkey](https://github.com/ckaiser/UGlobalHotkey) | `231b10144741b29037f0128bb7a1cd7176529f74` | Public Domain | Qt 6 port + `operator[]` bug fix via `diff_files` patch; Meson build overlay |
| **GoogleTest** _(test-only)_ | Unit-test framework | [google/googletest](https://github.com/google/googletest) (v1.17.0, WrapDB `1.17.0-4`) | source hash `65fab701…` | BSD-3-Clause | none — WrapDB `[wrap-file]`, fetched only when no system gtest is present |

The two runtime dependencies are Meson **`wrap-git`** subprojects (SHA-pinned);
GoogleTest is a **`[wrap-file]`** from WrapDB (source + patch hash-pinned), used only
by the test suite. Each `.wrap` records an immutable pin and Meson fetches the source
at `meson setup` time. For the runtime deps, our changes live in
`subprojects/packagefiles/<name>/`: **additive files** (each `meson.build`) are applied
as a `patch_directory` overlay, while **modifications to upstream source** (UGlobalHotkey's
Qt 6 port) are applied as a `diff_files` **patch** — auditable against the pinned SHA and
loud on drift. Upstream is never modified in place.

## The UGlobalHotkey lineage note

The hotkey code was written by [bakwc](https://github.com/bakwc) and extracted from
[Pastexen](https://github.com/bakwc/Pastexen); **falceeffect** turned it into a
standalone Qt library, but `falceeffect/UGlobalHotkey` **no longer exists (404)**.
[ckaiser](https://github.com/ckaiser) (the Lightscreen author) forked it with code-style
and Windows-support changes. Diffing DuskScreen's previously-vendored copy against
`ckaiser/UGlobalHotkey@231b101` is **byte-identical** except for the Qt 6 port (and an
`operator[]` off-by-one fix) — which confirms ckaiser is the true, still-live ancestor
and the correct wrap source.

## Details

- **SingleApplication** — overlay: [`subprojects/packagefiles/singleapplication/meson.build`](subprojects/packagefiles/singleapplication/meson.build); wrap: [`subprojects/singleapplication.wrap`](subprojects/singleapplication.wrap).
- **UGlobalHotkey** — full patch breakdown: [`subprojects/packagefiles/uglobalhotkey.PROVENANCE.md`](subprojects/packagefiles/uglobalhotkey.PROVENANCE.md); wrap: [`subprojects/uglobalhotkey.wrap`](subprojects/uglobalhotkey.wrap).

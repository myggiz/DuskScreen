# DuskScreen

A lightweight Windows screenshot tool — capture the whole screen, a selected area,
or a specific window, straight to disk, with global hotkeys and a tray icon.

DuskScreen is a modernized fork of
[Christian Kaiser's Lightscreen](https://github.com/ckaiser/Lightscreen),
ported to **Qt 6** and slimmed down to a focused capture-to-disk tool.

## What's different from upstream Lightscreen

- **Ported to Qt 6** (builds against Qt 6.11 / MinGW). Every removed-in-Qt6 API was
  replaced — QtWinExtras, `QDesktopWidget`, `QSound`, `QDirModel`,
  `QSysInfo::WindowsVersion`, `QRegExp`, and more.
- **Slimmed down:** removed the image-upload subsystem (Imgur / Pomf clones), the
  screenshot preview dialog, and the SQLite history — captures go straight to disk.
- **Bug fixes** surfaced by the port, e.g. *Go to Folder* now reveals the file in
  Explorer again instead of opening it in an image viewer.
- **Dependencies** (`SingleApplication`, `UGlobalHotkey`) are Meson `wrap-git`
  subprojects, SHA-pinned to their upstreams with the Qt 6 patches carried as a
  `packagefiles` overlay. `meson setup` fetches them automatically — no `git submodule`
  steps — so the repo builds with a single configure (network access is needed the
  first time you configure). See [PROVENANCE.md](PROVENANCE.md) for the full lineage
  graph, pinned revisions, and licenses.

## Antivirus warnings

Release builds are **not code-signed**, so some antivirus products — notably AVG/Avast
CyberCapture — may sandbox `duskscreen.exe` or flag it as "suspicious" on first run.

This is a reputation heuristic rather than a detection: the binary is unsigned and rare,
and every release is a fresh build with a new hash. DuskScreen is also, by design, a
program that captures your screen, registers global hotkeys and can add itself to
startup — behaviour that heuristics treat with suspicion when the publisher is unknown.

Every release publishes **SHA-256 checksums** so you can verify you have the exact
published build:

```powershell
Get-FileHash .\DuskScreen-1.0.3-win64.zip -Algorithm SHA256
```

The source is all here, and the binaries are never packed or obfuscated. See
[#11](https://github.com/myggiz/DuskScreen/issues/11) for the code-signing plan.

## Building

DuskScreen uses **Meson**. It needs Qt 6, Meson, Ninja, and a C++17 compiler. A
single `meson setup` fetches the wrapped dependencies automatically (network access
is needed the first time you configure).

```bash
meson setup build
meson compile -C build
./build/duskscreen        # duskscreen.exe on Windows
meson test -C build       # run the unit suite
```

See **[BUILDING.md](BUILDING.md)** for the full guide — prerequisites, per-platform
instructions (Linux / Windows-MSYS2 / self-contained Qt), build options, testing,
coverage, and troubleshooting.

## License

GPL v2-or-later, unchanged from upstream — see [LICENSE](LICENSE).

Original code © 2008–2021 Christian Kaiser. Qt 6 port, slim-down, and DuskScreen
rebrand © 2026 Myggiz. This is an independent fork and is not endorsed by the
original author.

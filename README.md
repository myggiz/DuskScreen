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
- **Bundled dependencies** (`SingleApplication`, `UGlobalHotkey`) are vendored in-tree
  and patched for Qt 6, so the repo clones and builds with no submodule setup.

## Building

Requires **Qt 6.x** (tested with 6.11, MinGW 13) and `qmake`:

```bash
qmake duskscreen.pro
mingw32-make            # or nmake / jom for an MSVC kit
```

To produce a standalone, redistributable folder, run `windeployqt` on the built
`duskscreen.exe` (and copy the `sounds/` folder next to it).

## License

GPL v2-or-later, unchanged from upstream — see [LICENSE](LICENSE).

Original code © 2008–2021 Christian Kaiser. Qt 6 port, slim-down, and DuskScreen
rebrand © 2026 Myggiz. This is an independent fork and is not endorsed by the
original author.

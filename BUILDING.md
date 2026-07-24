# Building DuskScreen

DuskScreen builds with **[Meson](https://mesonbuild.com/)** and **Ninja**. A single
`meson setup` fetches the two wrapped dependencies (`SingleApplication`,
`UGlobalHotkey`) automatically — there are no `git submodule` steps — so a clean
checkout builds with one configure. **Network access is required the first time you
configure** so Meson can fetch the wraps (and, if you don't have a system `gtest`,
the test framework).

## Prerequisites

| Tool | Minimum | Notes |
|---|---|---|
| Meson | ≥ 1.1 | `project(meson_version: '>=1.1')` |
| Ninja | any recent | default Meson backend |
| C++ compiler | C++17 | GCC, Clang, or MinGW/MSVC |
| pkgconf / pkg-config | any | used to locate Qt 6 |
| Qt 6 | 6.x (built/tested against 6.11) | modules: **Core, Gui, Widgets, Network, Multimedia** |

Platform libraries are found automatically by Meson and need no manual flags:

- **Linux:** `X11` (and xcb/X11 at runtime).
- **Windows:** `gdi32`, `user32`, `ole32`, `shell32`, `shlwapi`, `comctl32`.

`gcovr` (≥ 5.0) is required **only** for the coverage targets — see
[Coverage](#coverage-opt-in-dev-only). It is not needed to build or test.

## Quick start

```bash
meson setup build
meson compile -C build
./build/duskscreen        # duskscreen.exe on Windows
```

### Linux (Arch example)

```bash
sudo pacman -S --needed meson ninja gcc pkgconf qt6-base qt6-multimedia
meson setup build
meson compile -C build
./build/duskscreen
```

Use your distro's equivalent Qt 6 packages elsewhere (e.g. Debian/Ubuntu:
`qt6-base-dev qt6-multimedia-dev`, plus `meson ninja-build pkg-config`).

### Windows (MSYS2 / MinGW)

```bash
pacman -S --needed mingw-w64-x86_64-{toolchain,meson,ninja,pkgconf,qt6-base,qt6-multimedia}
meson setup build
meson compile -C build
```

Then run [`windeployqt`](https://doc.qt.io/qt-6/windows-deployment.html) on
`build/duskscreen.exe` to produce a redistributable folder with the required Qt DLLs.

For iterative development, skip the deploy step and launch through
[`meson devenv`](https://mesonbuild.com/Commands.html#devenv) — see
[Running from the build tree](#running-from-the-build-tree).

### Self-contained Qt (no system install)

If you don't have (or don't want) a system Qt, fetch a prebuilt Qt into a local
prefix with [`aqtinstall`](https://github.com/miurahr/aqtinstall):

```bash
pip install aqtinstall
```

> **How Meson finds this Qt — put `bin/` on `PATH`.** Meson's `qt6` module locates
> Qt via pkg-config or `qmake`; it has **no CMake backend**, so `CMAKE_PREFIX_PATH`
> is ignored, and aqt-installed Qt ships **no pkg-config `.pc` files**. The reliable
> lever is to put the prefix's `bin/` directory (which contains `qmake6`) on `PATH`
> before `meson setup`. `Core`/`Gui`/`Widgets`/`Network` live in `qtbase` and are
> installed automatically — only `Multimedia` needs `-m qtmultimedia`.

**Linux (desktop GCC):**
```bash
aqt install-qt --outputdir "$HOME/qt" linux desktop 6.11.1 linux_gcc_64 -m qtmultimedia
export PATH="$HOME/qt/6.11.1/linux_gcc_64/bin:$PATH"
meson setup build
meson compile -C build
```
The Linux desktop GCC arch is `linux_gcc_64` for Qt ≥ 6.7 (it was `gcc_64` on 6.6 and
earlier).

**Windows (MinGW):** `aqt install-qt` ships Qt's binaries but **not** a compiler, so
also fetch the matching MinGW toolchain — its ABI must match Qt's build, so prefer
aqt's toolchain over an unrelated MSYS2 one.
```bat
aqt install-qt  --outputdir C:\Qt windows desktop 6.11.1 win64_mingw -m qtmultimedia
:: Run `aqt list-tool windows desktop tools_mingw` to get the exact package name,
:: then install the matching MinGW toolchain, e.g.:
aqt install-tool --outputdir C:\Qt windows desktop tools_mingw qt.tools.win64_mingw1310
set "PATH=C:\Qt\6.11.1\win64_mingw\bin;%PATH%"
meson setup build
meson compile -C build
```

> **Windows + Qt 6.11 caveat.** Qt 6.11.0/6.11.1 changed the Windows download layout,
> which the current PyPI `aqtinstall` (3.3.0) cannot parse (fails on `Updates.xml`).
> Until the fix ships to PyPI, either pin Qt **6.10.1** on Windows, or install aqt
> from git main:
> `pip install "aqtinstall @ git+https://github.com/miurahr/aqtinstall.git@main"`.
> Linux is unaffected — 6.11.1 works with `aqtinstall` 3.3.0.

## Build options

Set with `-Doption=value` at `meson setup` (or change later with
`meson configure build -Doption=value`).

| Option | Type | Default | Purpose |
|---|---|---|---|
| `app_url` | string | `https://duskscreen.com` | Homepage + updater version/whatsnew endpoint base URL. |
| `tests` | feature (`auto`/`enabled`/`disabled`) | `auto` | Build the GoogleTest unit suite. `auto` builds it when gtest is resolvable; pass `-Dtests=enabled` to make a missing suite a hard error instead of a silent skip. `-Dtests=disabled` builds the app only. |

Standard Meson options also apply, e.g. `--buildtype=debug|release`,
`-Db_coverage=true`, `--prefix`. `meson install -C build` installs the executable.

## Running from the build tree

To launch the freshly built executable without installing or bundling DLLs, use
Meson's built-in [developer environment](https://mesonbuild.com/Commands.html#devenv):

```bash
meson devenv -C build ./duskscreen        # ./duskscreen.exe on Windows
# or open an interactive shell in the devenv:
meson devenv -C build
```

`meson devenv` is cross-platform — it sets `PATH`, `LD_LIBRARY_PATH`,
`PKG_CONFIG_PATH` and friends to whatever the build tree needs at runtime — and is
the recommended dev-run command on every OS, so IDE run configurations (CLion, VS
Code Meson extension, Qt Creator) can point at the same command everywhere.

What it resolves differs by platform:

- **Windows:** effectively required — there is no RPATH, so DLLs resolve only via
  `PATH`. `meson.build` prepends Qt's `bindir` to the devenv `PATH` so `Qt6*.dll`
  and the MinGW runtime resolve without a `windeployqt` bundle.
- **Linux:** optional for this project. Qt 6 is a system install on the loader's
  default search path, and both wrap subprojects (`SingleApplication`,
  `UGlobalHotkey`) link **statically**, so the executable carries no build-tree
  shared-library dependencies — `./build/duskscreen` runs directly. `meson devenv`
  still works and is worth using for parity and IDE configs; it becomes necessary
  only if a dependency is later built as a shared library.

## Testing

DuskScreen has a GoogleTest unit suite. gtest is sourced via a Meson wrap
(`subprojects/gtest.wrap`): `meson setup` uses a system `gtest` if present, otherwise
fetches it at configure time — no host install required. Tests run **headless**
(`QT_QPA_PLATFORM=offscreen`) and hit no network.

```bash
meson setup build
meson test -C build            # runs the unit suite
```

## Coverage (opt-in; dev only)

Coverage requires **`gcovr` ≥ 5.0** on `PATH` (`pip install 'gcovr>=5.0'` or your
distro's package). Unlike gtest, gcovr **cannot** be vendored as a subproject —
Meson's coverage machinery shells out to a literal `gcovr` on `PATH` at configure
time and never consults `find_program`/`override_find_program`. Building and running
the tests need only Qt 6 (+ X11); gcovr is needed solely for the `coverage-*` targets.

```bash
meson setup build-cov -Db_coverage=true --buildtype=debug
meson test -C build-cov
ninja -C build-cov coverage-html   # → build-cov/meson-logs/coveragereport/index.html
# or: coverage-text (→ meson-logs/coverage.txt)
#     coverage-xml  (→ meson-logs/coverage.xml)
```

Coverage is scoped to project source via `gcovr.cfg`: generated moc/ui/qrc files and
vendored subprojects are excluded — **except** `uglobalhotkey/ukeysequence.{cpp,h}`,
which we patch (Qt 6 port + `operator[]` bounds fix) and unit-test, so it counts as
first-party. If a combined `ninja … coverage-html coverage-xml` invocation fails
transiently under gcovr, run the coverage targets as separate invocations.

## Troubleshooting

- **First `meson setup` fails with a network/wrap error** — the wraps are fetched at
  configure time; ensure network access on the first configure. Subsequent configures
  reuse `subprojects/packagecache/`.
- **`meson setup` fails on MSYS2 with `CERTIFICATE_VERIFY_FAILED` fetching wraps** —
  MSYS2's mingw64 Python ships without a CA trust store, so `urllib` (which Meson uses
  to download wraps) rejects every HTTPS response. Install the CA packages and point
  Python at the bundle:
  ```bash
  pacman -S --needed mingw-w64-x86_64-ca-certificates mingw-w64-x86_64-python-certifi
  echo 'export SSL_CERT_FILE=/mingw64/etc/ssl/certs/ca-bundle.crt' >> ~/.bashrc
  ```
  This is an MSYS2 packaging gap ([MINGW-packages#1086](https://github.com/msys2/MINGW-packages/issues/1086)),
  not a DuskScreen issue.
- **Qt 6 not found** — ensure the Qt 6 dev packages are installed and discoverable via
  `pkg-config` (`pkg-config --modversion Qt6Core`), or point `PKG_CONFIG_PATH` /
  `CMAKE_PREFIX_PATH` at your Qt prefix.
- **`gcovr` not found / coverage targets missing** — install `gcovr>=5.0`; it is a dev
  prerequisite, not a build dependency.
- **`gcovr` install on MSYS2 fails (`No module named pip` / PEP 668 / `lxml` build
  error)** — MSYS2's mingw64 Python ships without pip and rejects direct
  `pip install` (PEP 668), and `gcovr`'s `lxml` dep needs `libxml2` headers to build
  from source. Use pipx with the prebuilt lxml:
  ```bash
  pacman -S --needed mingw-w64-x86_64-python-pipx mingw-w64-x86_64-python-lxml
  pipx install --system-site-packages 'gcovr>=5.0'
  pipx ensurepath        # adds ~/.local/bin to PATH
  ```

## Dependency provenance

`SingleApplication` and `UGlobalHotkey` are Meson `wrap-git` subprojects, SHA-pinned
to their upstreams with the Qt 6 patches carried as a `packagefiles` overlay. See
[PROVENANCE.md](PROVENANCE.md) for the full lineage graph, pinned revisions, and
licenses.

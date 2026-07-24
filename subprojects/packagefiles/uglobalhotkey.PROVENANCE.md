# UGlobalHotkey — wrap-git subproject (ckaiser/UGlobalHotkey)

## Upstream

- **Repo:** https://github.com/ckaiser/UGlobalHotkey
- **Pinned revision:** `231b10144741b29037f0128bb7a1cd7176529f74`
  (master HEAD, "Merge pull request #1 from focusn1k/master").
- Fetched via `subprojects/uglobalhotkey.wrap` (`[wrap-git]`, SHA-pinned).

### Why ckaiser, not falceeffect

The original library was written by [bakwc](https://github.com/bakwc), extracted from
[Pastexen](https://github.com/bakwc/Pastexen), and turned into a standalone Qt library
by **falceeffect** — but `falceeffect/UGlobalHotkey` **no longer exists (404)**.
[ckaiser](https://github.com/ckaiser) (author of Lightscreen) forked it with "code
style changes, better Windows support and whatever else I might need for Lightscreen"
per the vendored `README.md`. Diffing DuskScreen's previously-vendored copy against
`ckaiser/UGlobalHotkey@231b101` shows the trees are **byte-identical** except for the
changes carried in `0001-qt6-port.patch` — so ckaiser is the confirmed ancestor
and the correct, still-live wrap source.

## Local changes

Two mechanisms, both driven from `subprojects/packagefiles/uglobalhotkey/`. Meson
applies the `patch_directory` overlay first, then the `diff_files` patch.

### 1. Additive overlay (`patch_directory`)

- **`meson.build`** — Meson build (upstream ships only qmake `.pro`/`.pri`). Purely
  additive; upstream has no equivalent, so it is overlaid rather than patched.

### 2. Source patch (`diff_files = uglobalhotkey/0001-qt6-port.patch`)

A real `git diff` against upstream `231b101` — auditable, and it makes `meson setup`
**fail loudly** if upstream ever drifts (a `git apply` failure aborts configure),
instead of silently masking changes the way a full-file overlay would. It touches
`uglobalhotkeys.h`, `uglobalhotkeys.cpp`, `ukeysequence.cpp`, `ukeysequence.h`:

- **Qt 6 port:**
  - Native-event result params `long *result` → `qintptr *result`
    (Qt 6 `nativeEvent`/`nativeEventFilter` signature change).
  - X11 connection lookup moved from the private QPA API
    (`qApp->platformNativeInterface()->nativeResourceForWindow("connection", …)`,
    which required `QT += gui-private` under qmake) to the public
    `QNativeInterface::QX11Application` interface (`qApp->nativeInterface<…>()`),
    with a null-guard bail-out for non-X11 platforms (e.g. Wayland).
  - `xcb_key_symbols_free` guarded by `if (X11KeySymbs)`, plus member initializers
    (`X11Connection = nullptr`, `X11Wid = 0`, `X11KeySymbs = nullptr`) so the
    non-X11 early-return leaves no dangling frees.
  - `addKey((Qt::Key) seq[0])` → `addKey(seq[0].key())`
    (Qt 6 `QKeySequence::operator[]` now returns `QKeyCombination`).
- **Bug fix (`ukeysequence.h`):** `UKeySequence::operator[]` guard
  `if ((int)n > mKeys.size())` → `if (n >= static_cast<size_t>(mKeys.size()))`.
  The upstream `>` is off-by-one: at `n == size()` it fell through and read
  `mKeys[size()]` out of bounds (SIGABRT under Qt's debug assertions). Covered by
  the `UKeySequenceIndex.OutOfRangeReturnsUnknown` regression test in `tests/`.

The upstream-identical files (`hotkeymap.h`, `uglobal.h`, `README.md`) come straight
from the fetched tree.

To audit: `git clone https://github.com/ckaiser/UGlobalHotkey && cd UGlobalHotkey &&
git checkout 231b101` then `git apply .../0001-qt6-port.patch` and inspect, or diff
the fetched `subprojects/uglobalhotkey/` tree after `meson setup`.

## Facts

- **Purpose:** system-wide global hotkeys (Win32 `RegisterHotKey` / X11 `xcb_grab_key`).
- **License:** Public Domain, per the vendored `README.md` ("UGlobalHotkey library is
  licensed as Public Domain, so you are free to do anything with it.").

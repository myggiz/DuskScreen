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
three Qt 6 port files carried in this overlay — so ckaiser is the confirmed ancestor
and the correct, still-live wrap source.

## Local patches (this overlay's `patch_directory`)

Meson copies every file in `subprojects/packagefiles/uglobalhotkey/` over the fetched
tree at configure time. The overlay carries:

- **`meson.build`** — Meson build (upstream ships only qmake `.pro`/`.pri`).
- **`uglobalhotkeys.h`**, **`uglobalhotkeys.cpp`**, **`ukeysequence.cpp`** — the Qt 6
  port. Exactly these changes vs upstream `231b101`:
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

The upstream-identical files (`hotkeymap.h`, `uglobal.h`, `ukeysequence.h`,
`README.md`) are **not** overlaid — they come straight from the fetched tree.

To audit: `git clone https://github.com/ckaiser/UGlobalHotkey && cd UGlobalHotkey &&
git checkout 231b101` then diff its `uglobalhotkeys.{h,cpp}` / `ukeysequence.cpp`
against this overlay.

## Facts

- **Purpose:** system-wide global hotkeys (Win32 `RegisterHotKey` / X11 `xcb_grab_key`).
- **License:** Public Domain, per the vendored `README.md` ("UGlobalHotkey library is
  licensed as Public Domain, so you are free to do anything with it.").

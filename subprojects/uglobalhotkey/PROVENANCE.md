# UGlobalHotkey — vendored

- Upstream: https://github.com/falceeffect/UGlobalHotkey (confirmed by the vendored
  `README.md`: "extracted from Pastexen [by bakwc] ... turned into a shared library
  by falceeffect"). This copy also carries local Lightscreen-derived modifications
  ("code style changes, better Windows support ... for Lightscreen") per the same
  README, so it is a fork-of-a-fork, not a pristine falceeffect checkout.
- Commit / version: unknown — verify upstream. The vendoring commit in this repo's
  history (`7139bc7 Make this an independent fork: vendor deps, fork README, drop
  upstream origin`, found via `git log --follow -- tools/UGlobalHotkey/uglobalhotkeys.cpp`)
  imported the sources directly with no submodule pointer or recorded upstream SHA;
  there is no `.gitmodules` entry for this path. Diffing against the live
  falceeffect/UGlobalHotkey and bakwc/Pastexen histories was not performed as part
  of this task — do that before treating any specific upstream commit as authoritative.
- Local patches:
  - Qt 6 port (this migration).
  - X11 connection lookup moved from the private QPA API
    (`qApp->platformNativeInterface()->nativeResourceForWindow("connection", ...)`,
    requiring `QT += gui-private` under qmake) to the public
    `QNativeInterface::QX11Application` interface (`qApp->nativeInterface<...>()`),
    with a null-guard bail-out for non-X11 platforms (e.g. Wayland).
  - Vendored into a Meson subproject (`subprojects/uglobalhotkey/meson.build`),
    replacing the qmake `.pro`/`.pri` build files.
- Purpose: system-wide global hotkeys (Win32 `RegisterHotKey` / X11 `xcb_grab_key`).
- License: Public Domain, per the vendored `README.md` ("UGlobalHotkey library is
  licensed as Public Domain, so you are free to do anything with it.").

# Platforms: Windows only

**DuskScreen is a Windows application.** There is no Linux or macOS build, no
download for either, and no intention to add one in this repository.

It did not start that way — the tree carried Linux and X11 code, and a stray
macOS conditional inherited from Lightscreen. Both are gone.

The reason is plain rather than technical: DuskScreen is a tool its author uses
daily on Windows, there is no Linux or macOS machine here to test a build on, and
nobody is asking for one. Code that cannot be run or tested is not support, it is
a liability in every platform conditional.

This file is the handover, not a plan. Nothing below is scheduled or intended;
it is what someone would have to build if the need ever arrived, written down
while it is still fresh. A separate fork is a reasonable home for either
platform.

## macOS was never supported

One conditional of our own, in `dialogs/namingdialog.cpp`, rejected `:` in the
date format on macOS. It came in with `6242557` — an upstream Lightscreen
commit, inherited at the fork rather than written here. It was removed along
with `CONFIG -= app_bundle` in the test project, a macOS-only qmake setting that
did nothing on a Windows build.

**That is not the same as there being no macOS wiring.** Be careful here if you
are planning a port, because the build is less bare than it looks:

- `duskscreen.pro` includes `tools/UGlobalHotkey/uglobalhotkey.pri`, which pulls
  in `uglobalhotkey-libs.pri` — and that links `-framework Carbon` on macOS.
- That vendored library also carries a complete macOS hotkey implementation:
  `QtKeyToMac` plus a Carbon `RegisterEventHotKey` handler.
- The platform-specific functions in `tools/os.cpp` have `#else` fallbacks
  rather than Windows-only bodies. `os::grabWindow` falls back to
  `QScreen::grabWindow`, and `os::cursor` returns an empty pixmap.

So a macOS build would plausibly have compiled. What is actually missing is
everything after that: it was never built, never run and never tested, there is
no `macx:` section configuring one, and nothing was ever written to make capture
work there.

If anyone ever did port it, the real work would be capture and permissions
rather than hotkeys:

- **Capture.** The `QScreen::grabWindow` fallback is not a screenshot tool's
  answer on a current macOS — it needs `ScreenCaptureKit`, and the user has to
  grant screen recording in System Settings before anything returns pixels.
- **The window picker** needs the accessibility permission to see other
  applications' windows.
- **Cursor capture** has no implementation for macOS at all.
- **Packaging** — a bundle, an icon, and signing and notarisation, without which
  a downloaded build will not open.

Hotkeys are the one piece already present, by way of the vendored library. None
of the rest has been investigated, and none of it can be tested here — there is
no Mac to run it on.

## Linux was removed

1.0.5 ported the Qt 5-only APIs that had blocked compilation (`x11extras`,
`QX11Info`, the old `QPixmap::grabWindow`) and the tree built and ran on Linux
from then on — but it was never Linux *support*: capture returned nothing on
Wayland, global hotkeys were unreliable or unavailable, and the window picker did
not work.

Making those work needs a different capture and input stack rather than more
fixes to the X11 one, which was assessed in October 2026 and found to be a large
piece of work with no overlap with the Windows product. Rather than keep
half-working code in every platform conditional, it was removed.

### What was removed

All of it was DuskScreen's own code. The last commit that contains it is
**`4c65798`** — `git show 4c65798:tools/os.cpp` and friends, or
`git log -S Q_OS_LINUX` to follow it.

| Where | What |
|---|---|
| `tools/os.cpp`, `tools/os.h` | the X11 display helper, `findRealWindow` and `windowUnderCursor` (lifted from KSnapshot), the `Window`/`XID` typedefs, and the `~/.config/autostart` `.desktop` writer |
| `tools/windowpicker.cpp` | the X11 picker: `XQueryPointer` window resolution, `WM_NAME` title reading via `XGetTextProperty`/`XmbTextPropertyToTextList`, and `_NET_WM_ICON` parsing with its bounds checks |
| `tools/screenshot.cpp` | active-window capture via `XGetInputFocus`, and the `optipng` lookup through `QStandardPaths::findExecutable` |
| `dialogs/optionsdialog.cpp` | Oxygen/KDE layout tweaks, the OptiPNG availability check, and the hiding of the sound-cue and cursor options that Linux could not honour |
| `dialogs/namingdialog.cpp` | rejecting `/` in the date format |
| `dialogs/areadialog.cpp` | `Qt::X11BypassWindowManagerHint` on the area selector |
| `duskscreenwindow.cpp` | two guards that *skipped* tray show/hide around a capture, because X was not quick enough and the notification landed anywhere but the icon |
| `duskscreen.pro` | `unix:LIBS += -lX11` |

The Windows binary is byte-for-byte identical before and after, because every
one of those lived in a branch the Windows compiler never reached.

### What Linux support would actually require

- **Capture.** Wayland gives no equivalent of an X11 root-window grab. It needs
  the XDG desktop portal `org.freedesktop.portal.Screenshot` (or
  ScreenCast + PipeWire for anything live), which is asynchronous and prompts
  the user for permission — a different shape from the synchronous grab the code
  is built around. Tracked historically as GitHub issue #7.
- **Global hotkeys.** On Wayland the compositor owns shortcuts, so there is
  nothing to grab and nothing to detect a conflict against. It needs
  `org.freedesktop.portal.GlobalShortcuts`, which is newer than the other
  portals and unevenly implemented across desktops.
- **The window picker.** Wayland deliberately denies an application any view of
  other windows' geometry or titles. There is no portal for this; on Wayland the
  feature cannot be built as designed.
- **Packaging.** An install target, a `.desktop` file and an icon — none of
  which ever existed. A spec-compliant autostart entry needs proper `Exec=`
  quoting, a `Name=` key, and `XDG_CONFIG_HOME` honoured rather than
  `~/.config` assumed.
- **Cursor capture** on X11 was never implemented; the option was hidden
  instead.

The short version: on X11 most of this worked and was merely buggy. On Wayland —
which is what a new Linux user actually runs — capture, hotkeys and window
picking each need a portal, and one of them has no portal at all.

### The defects that were recorded against it

These were open issues when Linux was dropped. They are cancelled rather than
fixed, and are kept in Linear for whoever picks this up — all six carry the
`future` label, which is retained for exactly that purpose, so filtering the
DuskScreen team by it finds the set without relying on the IDs below:

| Issue | Problem |
|---|---|
| DUSK-4 | global hotkeys stop working while Caps Lock is on |
| DUSK-17 | the X11 window picker captures an arbitrary window over the bare root, and garbles `WM_NAME` titles |
| DUSK-18 | a hotkey conversion failure grabs an arbitrary key and reports success |
| DUSK-19 | the autostart `.desktop` entry is not spec-compliant: escaping, missing `Name=`, ignores `XDG_CONFIG_HOME` |
| DUSK-31 | no install target, `.desktop` file or icon, and cursor capture on X11 was an untracked TODO |
| DUSK-36 | UGlobalHotkey's X11 event filter casts every native event without checking its type |

DUSK-36 is in vendored code that is still present, so it is a live defect for
anyone who re-enables the Linux path — it is simply unreachable on Windows.

## What is still here

The vendored libraries keep their Linux and macOS code, deliberately:

- `tools/UGlobalHotkey` — `QtKeyToLinux`, `regLinuxHotkey` and the xcb event
  filter; `QtKeyToMac`, the Carbon hotkey handler and
  `mac: LIBS += -framework Carbon`.
- `tools/SingleApplication` — recorded as unmodified upstream v3.5.6, including
  its own `Q_OS_MACOS` branch.

Editing either one forks it from upstream and turns every future update into a
hand merge. Their non-Windows branches are inert in a Windows build, so they
cost nothing to keep. A fork that wants another platform back starts with these
already in place.

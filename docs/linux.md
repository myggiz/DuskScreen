# Linux: why there isn't any

**DuskScreen is a Windows application.** It targets Windows only, and there is
no Linux build, no Linux download, and no intention to add one in this
repository.

Earlier versions carried Linux and X11 code. 1.0.5 ported the Qt 5-only APIs
that had blocked compilation (`x11extras`, `QX11Info`, the old
`QPixmap::grabWindow`) and the tree built and ran on Linux from then on — but it
was never Linux *support*: capture returned nothing on Wayland, global hotkeys
were unreliable or unavailable, and the window picker did not work.

Making those work needs a different capture and input stack rather than more
fixes to the X11 one, which was assessed in October 2026 and found to be a large
piece of work with no overlap with the Windows product. Rather than keep
half-working code in every platform conditional, it was removed.

This file is the handover: what went, where it was, and what a future attempt
would have to build. A separate fork is a reasonable home for it.

## What was removed

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

## What is still here

The vendored libraries keep their Linux code, deliberately:

- `tools/UGlobalHotkey` — `QtKeyToLinux`, `regLinuxHotkey`, the xcb event filter.
- `tools/SingleApplication` — recorded as unmodified upstream v3.5.6.

Editing either one forks it from upstream and turns every future update into a
hand merge. Their Linux branches are inert in a Windows build, so they cost
nothing to keep. A fork that wants Linux back starts with these already in
place.

## What Linux support would actually require

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

## The defects that were recorded against it

These were open issues when Linux was dropped. They are cancelled rather than
fixed, and are kept in Linear for whoever picks this up:

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

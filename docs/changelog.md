# Changelog

All notable changes to DuskScreen, newest first. Each version links to its
GitHub release, which carries the full prose, the download and the SHA-256
checksums.

Versions follow [Semantic Versioning](https://semver.org/spec/v2.0.0.html); the
format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), with
one addition: **Internal** groups changes with no user-visible effect. Anything
rendering this for users can drop those sections and lose nothing.

Issue numbers refer to [GitHub issues](https://github.com/myggiz/DuskScreen/issues)
up to and including 1.0.14. Work tracked after that lives in Linear.

## [Unreleased]

### Fixed

- **Hotkeys on punctuation keys register the key you actually pressed.** `Ctrl+.`
  grabbed `Ctrl+Delete` system-wide — so Ctrl+Delete stopped reaching other
  programs — and `[`, `]` and `\` grabbed the Windows and menu keys. The virtual
  key now comes from the active keyboard layout instead of a fixed table, so
  punctuation also works on layouts other than US: on a Swedish keyboard `/` was
  binding the `'` key. Characters that only exist outside ASCII, such as `å`,
  `ä` and `ö`, can now be bound at all.
- A hotkey DuskScreen cannot map is reported as not registered, rather than
  silently grabbing an unrelated key. A stored hotkey naming no key at all used
  to grab `Ctrl+F2`.
- Escape and the title-bar close button now dismiss the quit confirmation
  instead of being ignored; both answer "Don't Quit", and "Quit" is still the
  default button (#88).

### Internal

- Added a QtTest target, `tests/tests.pro`, covering `UKeySequence` and the
  Windows key map. Known bugs are recorded as expected failures naming their
  issue, so fixing one reports an XPASS (DUSK-25).
- Rebuilt the quit confirmation without the deprecated `QMessageBox::question()`
  overload, which identified the answer by button index. This was the last
  deprecation warning in DuskScreen's own code (#88).

## [1.0.14] - 2026-08-06

### Added

- **WEBP save format**, alongside PNG, JPG and BMP — roughly half the size of
  JPEG at the same quality on a full desktop capture, without the softening
  around text. Choose it in Options → Filename → Format.

### Changed

- The format list greys out formats the build cannot write, instead of accepting
  the choice and then failing to save.

### Fixed

- Screenshots no longer stay stuck at "processing" when PNG optimisation is on
  and the optimiser cannot start. The count climbed with every capture, and
  quitting from the tray asked about work that had already finished.
- The Options window no longer shows blank Format and Filename boxes when the
  settings file holds a value it doesn't recognise; both fall back to a default.
- Hotkey parsing no longer writes chatter to the system log on every start.

## [1.0.13] - 2026-08-04

### Fixed

- Screenshot numbering no longer skips ahead when the folder holds a file that
  wraps around the naming prefix: `2screenshot.5.png` beside `screenshot.3.png`
  numbered the next capture 26 instead of 4.
- The window title and tray tooltip now show that a capture is in progress —
  most visibly while the window picker waits for a choice. They were only
  updated once the capture had already finished.
- Cancelling "Save as" no longer starts optipng against the file the cancelled
  save never created.

### Internal

- Replaced several deprecated Qt calls in the Windows-specific code, and
  silenced a flood of compiler warnings in the hotkey key map — the Windows
  build went from roughly four hundred warnings to one.

## [1.0.12] - 2026-08-02

### Changed

- Screenshots are encoded and written in the background, so DuskScreen no longer
  stops responding while it saves, and captures taken in quick succession don't
  queue behind each other.
- Default JPEG quality is now 90 rather than 100 — roughly a third of the file
  size and half the write time, with no visible difference on screenshot
  content. **Existing settings are left alone**: this applies to new
  installations only.

## [1.0.11] - 2026-08-02

### Fixed

- Long window titles in the picker now end in `...` and read more of the title
  before shortening, instead of being cut off with nothing to indicate it.

### Internal

- Replaced the vendored single-instance library with the current upstream
  release, moving the DuskScreen-specific behaviour into DuskScreen's own code.
  `--screen`, `--area` and `--quit` hand off to a running instance as before.
- Added a static analysis configuration, removed a parameter that could never
  have worked, and modernised code left behind by the Qt 6 port.

## [1.0.10] - 2026-07-31

### Changed

- Dragging an area selection redraws only the part of the screen that changed,
  rather than the entire desktop on every mouse movement — tens of megabytes of
  image copying per frame on a large or multi-monitor display.
- The Options window no longer stalls while typing in the Filename field. Each
  character re-scanned the whole screenshots folder to compute the preview
  number; the preview now waits for a pause in typing.

### Fixed

- Linux: assigning a hotkey already held by another program is now reported
  instead of appearing to succeed and doing nothing. **X11 only** — on Wayland
  the compositor holds shortcuts, so there is nothing to detect.
- Linux: fixed a memory leak each time hotkeys were registered, which happens
  again whenever the Options window closes.

### Removed

- A large amount of unreachable code, including Qt 6 port leftovers that
  pretended to drive the Windows taskbar progress indicator, and with them a
  latent crash that would have surfaced if anything had switched them on.

## [1.0.9] - 2026-07-30

### Fixed

- The area selector's accept and reject buttons are laid out properly. They were
  positioned against the whole screen rather than their container, so they
  floated at the wrong size in the wrong place and the container stayed empty.
- Pressing F5 to refresh the frozen desktop no longer locks the interface for a
  fifth of a second — and is more reliable: the old pause could end before the
  screen had updated, so the overlay sometimes ended up in the refreshed image.
- Capture requests are ignored while an area selection is in progress. Hotkeys
  stayed active during selection, so triggering one captured the selection
  overlay or opened a second selector on top of the first.
- Duplicate screenshot names now use the first free number. The " (2)" suffix
  was worked out by reading numbers out of every similarly-named file, so a
  folder containing `shot.7.png` jumped straight to `shot. (8).png`.
- The main window is no longer left in the wrong state after a capture when the
  "hide while taking a screenshot" setting changed mid-session; visibility was
  remembered from the first capture and never updated.
- Fixed a memory leak each time a second copy of DuskScreen was launched, and a
  case where a failed save was reported twice internally.

## [1.0.8] - 2026-07-30

The largest bug-fix release so far, including one crash and two security fixes.

### Security

- "Run at system startup" wrote the program path **unquoted** on Windows, so on
  a system with a file named `C:\Program.exe` the wrong program could run at
  login.
- Linux: the window picker could read past the end of the icon data it was
  given — data supplied by whichever window the pointer was over.

### Fixed

- Hotkeys using punctuation never fired — binding `Ctrl+-` or `Ctrl+/`
  registered the *numeric keypad* key instead, so the shortcut appeared set and
  did nothing.
- The sound cue never played unless DuskScreen was launched from its own folder,
  so effectively never when started from a shortcut or at login.
- "Run at system startup" never worked at all on Linux.
- `--quit` started DuskScreen instead of quitting it, when nothing was running.
- Fixed a crash when opening the window picker with the pointer on a coordinate
  no monitor covers — after unplugging a display, or in a gap between
  differently-sized ones.
- Cancelling Options → Import no longer saves the settings anyway.
- A corrupted settings file could produce filenames containing a literal `%1`.
- Failures to create the screenshot folder are reported instead of swallowed.
- Linux: the window picker read application icons incorrectly, showing a garbled
  thumbnail; also fixed a stray `optipng` process and several internal leaks.

### Changed

- PNG screenshots are around 12% smaller — compression had been effectively
  disabled by a setting that was overridden internally.

## [1.0.7] - 2026-07-29

### Fixed

- **The update check works again.** Whether it ran depended on the day of the
  year it last ran, so for many installs it stopped checking entirely and never
  resumed. It now reads this repository's releases directly, so there is no
  separate update server to drift out of step. A version can be skipped, or the
  check turned off; nothing is downloaded or installed for you.
- The window picker could capture a different window than the one the crosshair
  was released over, and sometimes returned the entire desktop instead of a
  window. Mostly affected the Linux build.
- The update check no longer leaks memory on each run, and can no longer hang
  indefinitely when the network is unavailable.

> Because the old update check was broken, 1.0.6 and earlier may never prompt
> about this release.

## [1.0.6] - 2026-07-29

### Fixed

- Screenshots stopped working after heavy use: every *window* capture leaked a
  Windows graphics handle. Measured across 50 window captures, the count no
  longer moves. (Fullscreen captures were unaffected.)
- A second capture taken within about a third of a second of the first used the
  *first* one's mode, and could leave DuskScreen stuck hiding itself twice a
  second, taking no screenshots at all until restarted. Each capture now keeps
  its own settings.
- "Save as" no longer produces `screenshot.1.png.png` when the suggested
  filename is accepted.
- Area captures are no longer a pixel short on scaled displays: at 125% or 150%
  a 301-pixel selection saved as 451 pixels instead of 452. Selections now round
  outward.

### Internal

- Corrected how the window picker reads application icons. No visible change.

## [1.0.5] - 2026-07-28

### Fixed

- A screenshot that could not be taken — target window closed mid-grab, the
  system refused, an allocation failed — was treated internally as though it had
  been *cancelled*, so the error message was skipped and nothing appeared at
  all. Failures are now reported; deliberate cancellation stays silent.

### Internal

- The source tree compiles and runs on Linux for the first time: the Qt 5-only
  APIs that blocked it (`x11extras`, `QX11Info`, the old `QPixmap::grabWindow`)
  are ported to Qt 6 equivalents, and the code no longer crashes on Wayland.

  **This is not Linux support and there is no Linux download.** Capture returns
  nothing on Wayland, global hotkeys are unreliable or unavailable, and the
  window picker does not work; that needs XDG desktop portals (#7). Windows is
  unaffected — every change is inside Linux-only paths, verified by comparing
  this build against 1.0.4: identical warning sets, identical binary size.

## [1.0.4] - 2026-07-27

### Fixed

- Out-of-bounds read in `UKeySequence::operator[]`: the bounds guard used `>`
  instead of `>=`, so asking for the key at index `size()` read one element past
  the end, and the index cast mismatched the `qsizetype` that `QList::size()`
  returns on Qt 6. Not reachable from DuskScreen's own code — a latent defect in
  the vendored hotkey library's public API (#53).
- The area-mode help banner came out up to 1px shorter than intended; its height
  was computed with integer division inside what looked like a float rounding
  expression, so the rounding never did anything (#54).

## [1.0.3] - 2026-07-23

### Fixed

- The mouse pointer is no longer stamped into area captures. With "capture mouse
  cursor" enabled, area mode froze the pointer at the position it had when the
  capture was *triggered* — on the DuskScreen UI when triggered from the tray,
  or wherever the pointer had been when triggered by hotkey. Area captures are
  now always cursor-free; whole-screen and window captures still honour the
  option.

## [1.0.2] - 2026-07-23

### Fixed

- **The black square around the captured mouse cursor.** With "capture cursor"
  enabled the pointer was composited with its full 32×32 bounding box painted
  solid black — most visible in area mode, where the frozen pointer is baked
  into the overlay background. A Qt 6 port regression: `QImage::fromHBITMAP()`
  discards the alpha channel the Qt 5 code preserved. The cursor bitmap is now
  read with `GetDIBits()` so its transparency survives (#9).
- A GDI handle leak in the same path — two bitmap handles leaked on every
  capture with the cursor enabled — and a `DeleteObject()` call on the
  system-owned cursor icon handle.

## [1.0.1] - 2026-07-22

### Fixed

- A memory leak: the Options dialog's Options-button menu was created without a
  parent and leaked on every open (#1).
- A one-time memory leak: the tray context menu was created without a parent
  (#2).

## [1.0.0] - 2026-07-22

First release — a lightweight Windows screenshot tool, a Qt 6 fork of
Lightscreen slimmed to a fast capture-to-disk workflow. Global hotkeys, tray
icon, and whole-screen, area and window capture to PNG, JPG or BMP. No
installer and no Qt needed: extract and run `duskscreen.exe`.

GPL v2-or-later. Original © 2008–2021 Christian Kaiser; DuskScreen changes
© 2026 Myggiz.

[Unreleased]: https://github.com/myggiz/DuskScreen/compare/v1.0.14...master
[1.0.14]: https://github.com/myggiz/DuskScreen/releases/tag/v1.0.14
[1.0.13]: https://github.com/myggiz/DuskScreen/releases/tag/v1.0.13
[1.0.12]: https://github.com/myggiz/DuskScreen/releases/tag/v1.0.12
[1.0.11]: https://github.com/myggiz/DuskScreen/releases/tag/v1.0.11
[1.0.10]: https://github.com/myggiz/DuskScreen/releases/tag/v1.0.10
[1.0.9]: https://github.com/myggiz/DuskScreen/releases/tag/v1.0.9
[1.0.8]: https://github.com/myggiz/DuskScreen/releases/tag/v1.0.8
[1.0.7]: https://github.com/myggiz/DuskScreen/releases/tag/v1.0.7
[1.0.6]: https://github.com/myggiz/DuskScreen/releases/tag/v1.0.6
[1.0.5]: https://github.com/myggiz/DuskScreen/releases/tag/v1.0.5
[1.0.4]: https://github.com/myggiz/DuskScreen/releases/tag/v1.0.4
[1.0.3]: https://github.com/myggiz/DuskScreen/releases/tag/v1.0.3
[1.0.2]: https://github.com/myggiz/DuskScreen/releases/tag/v1.0.2
[1.0.1]: https://github.com/myggiz/DuskScreen/releases/tag/v1.0.1
[1.0.0]: https://github.com/myggiz/DuskScreen/releases/tag/v1.0.0

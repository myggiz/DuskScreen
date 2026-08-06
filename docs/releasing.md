# Releasing DuskScreen

DuskScreen has **no server component**. The update check reads the GitHub
Releases API for this repository, so publishing a release is the whole of the
release process — there is nothing to deploy or keep in sync.

## The update check

`updater/updater.cpp` issues one request per launch:

```
GET https://api.github.com/repos/myggiz/DuskScreen/releases/latest
```

From the JSON it reads `tag_name` and `html_url`. A leading `v` is stripped from
the tag, the remainder is parsed with `QVersionNumber`, and if it is greater
than the running version the user is offered the release page.

Consequences worth knowing:

- **The tag is the version.** `v1.0.6` advertises 1.0.6. A release whose tag
  does not parse as a version number is ignored rather than mishandled.
- **Draft and pre-release entries are skipped**, because `/releases/latest`
  excludes them. A draft release is therefore invisible to existing installs
  until it is published — which is what makes the draft-then-publish flow safe.
- **The check is unauthenticated**, so it is subject to GitHub's per-IP rate
  limit of 60 requests/hour. One request per launch is nowhere near it.
- Failures — no network, rate limiting, malformed JSON — are silent by design.
  A failed update check is not worth interrupting someone's work for.

Users can decline a specific version ("Skip This Version", stored as
`options/skippedVersion`) or turn the check off entirely
(`options/disableUpdater`).

## Publishing a release

1. Bump `VERSION` in `version.pri`, and the filename in the README's checksum
   example.
2. **Build clean** — `make clean` or a fresh build directory — then run
   `windeployqt`, prune, and zip as `DuskScreen-<version>-win64.zip`.

   The clean build is not optional. `VERSION` reaches the code as the
   `-DAPP_VERSION` compiler flag, and qmake does not treat a changed define as
   a reason to recompile: an incremental build after a version bump leaves
   `main.cpp` untouched, producing a binary that reports the *previous* version
   in About and compares against it when checking for updates.

   **Check `imageformats/qwebp.dll` is in the payload.** The WEBP save format
   depends on Qt Image Formats, an optional Qt component (see the README's
   build section). Without it in the kit there is nothing for `windeployqt` to
   deploy, the build succeeds, and choosing WEBP simply writes no file. With it
   the plugin count rises from 4 to 9 and the pruned payload grows by roughly
   five files — that growth is the signal it made it in.
3. Create a GitHub release tagged **`v<version>`** — the leading `v` matters,
   as does the tag parsing as a version number.
4. Attach the zip and publish the SHA-256 of both the zip and `duskscreen.exe`
   in the release notes.

Creating the release as a **draft** first is recommended: drafts are invisible
to `/releases/latest`, so nobody is prompted to update until the binary and
checksums are actually attached.

## Web pages (optional)

Three links in the UI point at a website and are **hardcoded in the Qt Designer
`.ui` files**, not derived from any build setting:

| URL | Where |
|-----|-------|
| `https://duskscreen.com/` | Options → About → "home page" |
| `https://github.com/Myggiz/DuskScreen/` | Options → About → "GitHub page" |
| `https://duskscreen.com/help#date` | the "?" beside the date-format field |

Nothing depends on those pages existing — they open in a browser when clicked
and 404 harmlessly if absent. Edit `dialogs/optionsdialog.ui` and
`dialogs/namingdialog.ui` to change them.

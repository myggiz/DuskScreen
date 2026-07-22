# DuskScreen server setup

DuskScreen can point at a small server for two optional things:

1. an **update check** (tells the app if a newer version exists), and
2. a couple of **informational web pages** (home page, "what's new", help).

None of this is required for the app to work — capture/save is fully offline. If the
server is absent, the update check just fails silently and the links 404. Set it up
whenever you like.

## 1. Configure the base URL

All server URLs derive from a single build define, `APP_URL`, in
[`version.pri`](../version.pri):

```
APP_URL = https://duskscreen.myggiz.net
```

Change it to your host and rebuild. Everything below is relative to that base.

> Note: three links live in the Qt Designer `.ui` files instead of the define
> (they can't read a compile define): the GitHub link and "home page" link on the
> Options → About tab (`dialogs/optionsdialog.ui`) and the date-format help link
> (`dialogs/namingdialog.ui`). Edit those directly if your URLs differ.

## 2. The update endpoint — `GET {APP_URL}/version`

This is the only endpoint the app actually calls. Behavior (`updater/updater.cpp`):

- Fired ~5 s after launch, but **at most once every 7 days**, and only if the user
  hasn't disabled updates (Options → "check for updates").
- Request:

  ```
  GET {APP_URL}/version?from=<currentVersion>&platform=Windows_<osVersion>
  ```

  e.g. `GET https://duskscreen.myggiz.net/version?from=2.5&platform=Windows_10`

- **Response: a plain-text version string and nothing else** — e.g. `2.6`.
  The app parses it with `QVersionNumber` and, if it is greater than the running
  version, shows an "update available" prompt. The `from`/`platform` query params are
  informational (handy for stats); you can ignore them server-side.

The simplest possible implementation is a static file served as `text/plain`:

```
# nginx
location = /version { return 200 "2.6"; default_type text/plain; }
```

or just drop a file named `version` containing `2.6` at the web root. Bump it when you
publish a new build.

## 3. Informational pages (plain HTML, optional)

Opened in the user's browser when they click through — serve whatever you like:

| URL | Opened from |
|-----|-------------|
| `{APP_URL}/` | Options → About → "home page" |
| `{APP_URL}/whatsnew?from=<version>` | the tray "new version" prompt |
| `{APP_URL}/whatsnew/<version>` | the updater dialog link |
| `{APP_URL}/help#date` | the "?" next to the date-format field |

A single static `whatsnew` page (ignoring the version suffix) is fine to start.

## 4. Publishing a new version — checklist

1. Bump `VERSION` in `version.pri`, rebuild, run `windeployqt`, prune, zip `dist/DuskScreen`.
2. Update the `version` endpoint to the new number.
3. Update the `whatsnew` page.

That's the whole contract — one plain-text endpoint plus a couple of static pages.

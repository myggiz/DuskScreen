# SingleApplication — vendored

- Upstream: https://github.com/ckaiser/SingleApplication.git (a fork of https://github.com/itay-grudev/SingleApplication)
- Commit / version: `ebc35bb842811b00b3b89260c89301da77f88a15` (recorded in DuskScreen commit `0982eca` "Record Qt6-compat submodule commits", 2026-07-22, as the submodule's Qt6-compat pointer; note per that commit this fix was local to the ckaiser fork and may not exist upstream). Copied into the tree as a plain directory (submodule removed) in commit `7139bc7`.
- Local patches: Qt 6 port (verify no `QtWinExtras`/removed APIs remain).
- Purpose: single-instance guard via QLocalServer + QSharedMemory.

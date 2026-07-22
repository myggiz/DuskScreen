VERSION = 1.0.1
QMAKE_TARGET_COMPANY = Myggiz
QMAKE_TARGET_PRODUCT = DuskScreen
QMAKE_TARGET_DESCRIPTION = DuskScreen Screenshot Tool
QMAKE_TARGET_COPYRIGHT = Copyright (C) 2008-2021 Christian Kaiser, (C) 2026 Myggiz

# DuskScreen server base URL: homepage + the updater's version/whatsnew endpoints.
# Point this at your own host once the server is live (see docs/server-setup.md).
APP_URL = https://duskscreen.com

DEFINES += APP_VERSION=\\\"$$VERSION\\\"
DEFINES += APP_URL=\\\"$$APP_URL\\\"

TEMPLATE = app
TARGET = tst_duskscreen

QT += core gui testlib
CONFIG += c++17 console testcase
CONFIG -= app_bundle

INCLUDEPATH += $$PWD/.. $$PWD/../tools/UGlobalHotkey

# UGlobalHotkey's headers default to dllimport; the test links the sources
# directly, exactly as duskscreen.pro does.
DEFINES += UGLOBALHOTKEY_NOEXPORT

# ukeysequence.h is listed so moc runs on it: UKeySequence is a QObject.
HEADERS += tst_ukeysequence.h \
    ../tools/UGlobalHotkey/ukeysequence.h

SOURCES += main.cpp \
    tst_ukeysequence.cpp \
    ../tools/UGlobalHotkey/ukeysequence.cpp

# QtKeyToWin only exists in the Windows branch of hotkeymap.h, and the Linux
# branch of that header needs xcb.
windows {
    HEADERS += tst_hotkeymap.h
    SOURCES += tst_hotkeymap.cpp
}

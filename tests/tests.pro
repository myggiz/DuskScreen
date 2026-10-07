TEMPLATE = app
TARGET = tst_duskscreen

QT += core gui widgets testlib
CONFIG += c++17 console testcase
CONFIG -= app_bundle

INCLUDEPATH += $$PWD/.. $$PWD/../tools/UGlobalHotkey

# UGlobalHotkey's headers default to dllimport; the test links the sources
# directly, exactly as duskscreen.pro does.
DEFINES += UGLOBALHOTKEY_NOEXPORT

# ukeysequence.h and uglobalhotkeys.h are listed so moc runs on them: both are
# QObjects.
HEADERS += tst_ukeysequence.h \
    tst_hotkeymap.h \
    tst_hotkeyregistration.h \
    ../tools/UGlobalHotkey/ukeysequence.h \
    ../tools/UGlobalHotkey/uglobalhotkeys.h

SOURCES += main.cpp \
    tst_ukeysequence.cpp \
    tst_hotkeymap.cpp \
    tst_hotkeyregistration.cpp \
    ../tools/UGlobalHotkey/ukeysequence.cpp \
    ../tools/UGlobalHotkey/uglobalhotkeys.cpp

LIBS += -luser32

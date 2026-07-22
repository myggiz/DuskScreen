TEMPLATE = app
TARGET = duskscreen
DEFINES += QT_DEPRECATED_WARNINGS

HEADERS += dialogs/areadialog.h \
    dialogs/namingdialog.h \
    dialogs/optionsdialog.h \
    dialogs/updaterdialog.h \
    lightscreenwindow.h \
    tools/os.h \
    tools/screenshot.h \
    tools/screenshotmanager.h \
    tools/windowpicker.h \
    updater/updater.h \
    widgets/hotkeywidget.h

SOURCES += dialogs/areadialog.cpp \
    dialogs/namingdialog.cpp \
    dialogs/optionsdialog.cpp \
    dialogs/updaterdialog.cpp \
    lightscreenwindow.cpp \
    main.cpp \
    tools/os.cpp \
    tools/screenshot.cpp \
    tools/screenshotmanager.cpp \
    tools/windowpicker.cpp \
    updater/updater.cpp \
    widgets/hotkeywidget.cpp

FORMS += dialogs/namingdialog.ui \
    dialogs/optionsdialog.ui \
    lightscreenwindow.ui

RESOURCES += lightscreen.qrc
CODECFORSRC = UTF-8
INCLUDEPATH += $$PWD
CONFIG += c++17

QT += core gui widgets network multimedia

include($$PWD/tools/SingleApplication/singleapplication.pri)
include($$PWD/tools/UGlobalHotkey/uglobalhotkey.pri)

windows {
    RC_ICONS += images/LS.ico

    # MinGW
    contains(QMAKE_CC, gcc){
        LIBS += libgdi32 libgcc libuser32 libole32 libshell32 libshlwapi libcomctl32
        QMAKE_CXXFLAGS = -Wextra -Wall -Wpointer-arith
    }

    # MSVC
    contains(QMAKE_CC, cl){
        LIBS += gdi32.lib user32.lib ole32.lib shell32.lib shlwapi.lib comctl32.lib
    }

    CONFIG += embed_manifest_exe

    #QMAKE_CXXFLAGS_DEBUG += /analyze /W3 /wd6326
}

unix:LIBS += -lX11
unix:QT += x11extras

include (version.pri)

OTHER_FILES += TODO.txt









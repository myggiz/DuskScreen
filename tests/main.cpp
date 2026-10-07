#include <QTest>

#include "tst_ukeysequence.h"

#ifdef Q_OS_WIN
    // UGlobalHotkeys is a QWidget, so the registration test needs a
    // QApplication. Only Windows builds it, and only Windows is guaranteed a
    // display here: elsewhere a QApplication would abort the whole binary
    // before any test ran.
    #include <QApplication>

    #include "tst_hotkeymap.h"
    #include "tst_hotkeyregistration.h"
#else
    #include <QCoreApplication>
#endif

// One binary for several test classes, so there is a single thing for the
// pre-merge check to build and run.
int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    QApplication app(argc, argv);
#else
    QCoreApplication app(argc, argv);
#endif

    int failures = 0;

    tst_UKeySequence ukeysequence;
    failures += QTest::qExec(&ukeysequence, argc, argv);

#ifdef Q_OS_WIN
    tst_HotkeyMap hotkeymap;
    failures += QTest::qExec(&hotkeymap, argc, argv);

    tst_HotkeyRegistration registration;
    failures += QTest::qExec(&registration, argc, argv);
#endif

    return failures;
}

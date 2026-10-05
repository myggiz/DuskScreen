#include <QCoreApplication>
#include <QTest>

#include "tst_ukeysequence.h"

#ifdef Q_OS_WIN
    #include "tst_hotkeymap.h"
#endif

// One binary for several test classes, so there is a single thing for the
// pre-merge check to build and run.
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    int failures = 0;

    tst_UKeySequence ukeysequence;
    failures += QTest::qExec(&ukeysequence, argc, argv);

#ifdef Q_OS_WIN
    tst_HotkeyMap hotkeymap;
    failures += QTest::qExec(&hotkeymap, argc, argv);
#endif

    return failures;
}

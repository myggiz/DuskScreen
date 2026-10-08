#include <QApplication>
#include <QTest>

#include "tst_hotkeymap.h"
#include "tst_hotkeyregistration.h"
#include "tst_ukeysequence.h"

// One binary for several test classes, so there is a single thing for the
// pre-merge check to build and run. QApplication rather than QCoreApplication:
// UGlobalHotkeys is a QWidget.
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    int failures = 0;

    tst_UKeySequence ukeysequence;
    failures += QTest::qExec(&ukeysequence, argc, argv);

    tst_HotkeyMap hotkeymap;
    failures += QTest::qExec(&hotkeymap, argc, argv);

    tst_HotkeyRegistration registration;
    failures += QTest::qExec(&registration, argc, argv);

    return failures;
}

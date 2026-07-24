#include <QApplication>
#include <QLocale>
#include <QTimer>
#include <gtest/gtest.h>

// Custom gtest entry point. UKeySequence parses QKeySequence, and the Qt 6 docs
// require a QApplication (not QGuiApplication/QCoreApplication) to exist before any
// QKeySequence is constructed. We run headless via QT_QPA_PLATFORM=offscreen (also
// set in the Meson test env). gtest is driven inside the event loop via a
// zero-delay timer to avoid the QApplication singleton double-exec()/exit() crash.
int main(int argc, char **argv)
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc, argv);
    // Pin the whole suite to the C locale so locale-formatted output (e.g. the
    // year in Screenshot::getName's Date naming) is deterministic ASCII digits,
    // independent of the host's default locale. Set once here rather than per-test
    // so it never leaks between tests as order-dependent global state.
    QLocale::setDefault(QLocale::c());
    ::testing::InitGoogleTest(&argc, argv);

    int rc = 0;
    QTimer::singleShot(0, &app, [&]() {
        rc = RUN_ALL_TESTS();
        app.exit(rc);
    });
    app.exec();
    return rc;
}

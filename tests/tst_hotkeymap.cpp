#include "tst_hotkeymap.h"

#include <QtCore>
#include <windows.h>

#include <QTest>

#include "hotkeymap.h"

void tst_HotkeyMap::namedKeysMapToTheirVirtualKey_data()
{
    QTest::addColumn<int>("key");
    QTest::addColumn<int>("virtualKey");

    QTest::newRow("Escape")   << int(Qt::Key_Escape)   << int(VK_ESCAPE);
    QTest::newRow("Tab")      << int(Qt::Key_Tab)      << int(VK_TAB);
    QTest::newRow("Backtab")  << int(Qt::Key_Backtab)  << int(VK_TAB);
    QTest::newRow("Return")   << int(Qt::Key_Return)   << int(VK_RETURN);
    QTest::newRow("Enter")    << int(Qt::Key_Enter)    << int(VK_RETURN);
    QTest::newRow("Print")    << int(Qt::Key_Print)    << int(VK_SNAPSHOT);
    QTest::newRow("PageUp")   << int(Qt::Key_PageUp)   << int(VK_PRIOR);
    QTest::newRow("PageDown") << int(Qt::Key_PageDown) << int(VK_NEXT);
    QTest::newRow("Space")    << int(Qt::Key_Space)    << int(VK_SPACE);
    QTest::newRow("F1")       << int(Qt::Key_F1)       << int(VK_F1);
    QTest::newRow("F12")      << int(Qt::Key_F12)      << int(VK_F12);
}

void tst_HotkeyMap::namedKeysMapToTheirVirtualKey()
{
    QFETCH(int, key);
    QFETCH(int, virtualKey);

    QCOMPARE(QtKeyToWin(Qt::Key(key)), size_t(virtualKey));
}

// Qt::Key and the Windows virtual-key codes agree on letters and digits, which
// is why passing those through works.
void tst_HotkeyMap::lettersAndDigitsFallThroughUnchanged_data()
{
    QTest::addColumn<int>("key");
    QTest::addColumn<int>("virtualKey");

    QTest::newRow("A") << int(Qt::Key_A) << 0x41;
    QTest::newRow("Z") << int(Qt::Key_Z) << 0x5A;
    QTest::newRow("0") << int(Qt::Key_0) << 0x30;
    QTest::newRow("9") << int(Qt::Key_9) << 0x39;
}

void tst_HotkeyMap::lettersAndDigitsFallThroughUnchanged()
{
    QFETCH(int, key);
    QFETCH(int, virtualKey);

    QCOMPARE(QtKeyToWin(Qt::Key(key)), size_t(virtualKey));
}

// Punctuation is resolved through the active keyboard layout, so the expected
// value has to come from the same place rather than from a fixed table: the
// virtual key behind ';' is VK_OEM_1 on a US layout and something else on most
// others. What is asserted is that the mapping agrees with the layout.
void tst_HotkeyMap::punctuationMatchesTheActiveLayout_data()
{
    QTest::addColumn<int>("key");

    for (const char c : {'.', ',', '-', '=', ';', '/', '\'', '[', ']', '\\', '`'}) {
        QTest::newRow(QByteArray(1, c).constData()) << int(c);
    }
}

void tst_HotkeyMap::punctuationMatchesTheActiveLayout()
{
    QFETCH(int, key);

    const SHORT scan = VkKeyScanW(static_cast<wchar_t>(key));

    if (scan == -1) {
        QSKIP("this character is not on the active keyboard layout");
    }

    QCOMPARE(QtKeyToWin(Qt::Key(key)), size_t(scan & 0xFF));
}

// '.' is on every layout, so this one is unconditional — and it is the key
// DUSK-3 was reported against.
void tst_HotkeyMap::periodResolvesToARealKey()
{
    const size_t period = QtKeyToWin(Qt::Key_Period);

    QVERIFY(period != 0);
    QCOMPARE(period, size_t(VkKeyScanW(L'.') & 0xFF));
}

// DUSK-3: these Qt::Key values collide with unrelated virtual keys, and used to
// be passed through onto them. No layout resolves punctuation to any of these,
// so this holds everywhere.
void tst_HotkeyMap::punctuationNoLongerGrabsUnrelatedKeys()
{
    QVERIFY(QtKeyToWin(Qt::Key_Period) != size_t(VK_DELETE));
    QVERIFY(QtKeyToWin(Qt::Key_Apostrophe) != size_t(VK_RIGHT));
    QVERIFY(QtKeyToWin(Qt::Key_BracketLeft) != size_t(VK_LWIN));
    QVERIFY(QtKeyToWin(Qt::Key_Backslash) != size_t(VK_RWIN));
    QVERIFY(QtKeyToWin(Qt::Key_BracketRight) != size_t(VK_APPS));
    QVERIFY(QtKeyToWin(Qt::Key_QuoteLeft) != size_t(VK_NUMPAD0));
}

// 0 is not a virtual key: it means "no mapping", and registerHotkey() fails the
// binding rather than registering something that can never fire.
void tst_HotkeyMap::unmappableKeysAreRefused_data()
{
    QTest::addColumn<int>("key");

    QTest::newRow("unknown")   << int(Qt::Key_unknown);
    QTest::newRow("Hangul")    << int(Qt::Key_Hangul);
    QTest::newRow("Kana_Lock") << int(Qt::Key_Kana_Lock);
    QTest::newRow("Massyo")    << int(Qt::Key_Massyo);
}

void tst_HotkeyMap::unmappableKeysAreRefused()
{
    QFETCH(int, key);

    QCOMPARE(QtKeyToWin(Qt::Key(key)), size_t(0));
}

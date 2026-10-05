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

// These four are main-row punctuation. Mapping them to the numeric-keypad
// codes (VK_ADD, VK_SEPARATOR, VK_SUBTRACT, VK_DIVIDE) registered a keypad key
// the user never pressed, and VK_SEPARATOR barely exists on real keyboards, so
// that binding could not fire at all.
void tst_HotkeyMap::mainRowPunctuationUsesOemCodes()
{
    QCOMPARE(QtKeyToWin(Qt::Key_Plus),  size_t(VK_OEM_PLUS));
    QCOMPARE(QtKeyToWin(Qt::Key_Comma), size_t(VK_OEM_COMMA));
    QCOMPARE(QtKeyToWin(Qt::Key_Minus), size_t(VK_OEM_MINUS));
    QCOMPARE(QtKeyToWin(Qt::Key_Slash), size_t(VK_OEM_2));

    QVERIFY(QtKeyToWin(Qt::Key_Plus)  != size_t(VK_ADD));
    QVERIFY(QtKeyToWin(Qt::Key_Comma) != size_t(VK_SEPARATOR));
    QVERIFY(QtKeyToWin(Qt::Key_Minus) != size_t(VK_SUBTRACT));
    QVERIFY(QtKeyToWin(Qt::Key_Slash) != size_t(VK_DIVIDE));
}

// Qt::Key and the Windows virtual-key codes agree on letters and digits, which
// is why the default branch returning the key unchanged works at all.
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

// DUSK-3. The remaining main-row punctuation is unmapped, so it reaches the
// default branch and is returned as-is — and those Qt::Key values collide with
// unrelated virtual keys. Each QEXPECT_FAIL states the mapping the key should
// have; fixing DUSK-3 turns these into XPASS, which fails the run until the
// markers are removed.
void tst_HotkeyMap::unmappedPunctuationCollidesWithUnrelatedKeys()
{
    QEXPECT_FAIL("", "DUSK-3: Qt::Key_Period is 0x2E, which is VK_DELETE", Continue);
    QCOMPARE(QtKeyToWin(Qt::Key_Period), size_t(VK_OEM_PERIOD));

    QEXPECT_FAIL("", "DUSK-3: Qt::Key_Apostrophe is 0x27, which is VK_RIGHT", Continue);
    QCOMPARE(QtKeyToWin(Qt::Key_Apostrophe), size_t(VK_OEM_7));

    QEXPECT_FAIL("", "DUSK-3: Qt::Key_BracketLeft is 0x5B, which is VK_LWIN", Continue);
    QCOMPARE(QtKeyToWin(Qt::Key_BracketLeft), size_t(VK_OEM_4));

    QEXPECT_FAIL("", "DUSK-3: Qt::Key_Backslash is 0x5C, which is VK_RWIN", Continue);
    QCOMPARE(QtKeyToWin(Qt::Key_Backslash), size_t(VK_OEM_5));

    QEXPECT_FAIL("", "DUSK-3: Qt::Key_BracketRight is 0x5D, which is VK_APPS", Continue);
    QCOMPARE(QtKeyToWin(Qt::Key_BracketRight), size_t(VK_OEM_6));

    QEXPECT_FAIL("", "DUSK-3: Qt::Key_QuoteLeft is 0x60, which is VK_NUMPAD0", Continue);
    QCOMPARE(QtKeyToWin(Qt::Key_QuoteLeft), size_t(VK_OEM_3));

    QEXPECT_FAIL("", "DUSK-3: Qt::Key_Semicolon is 0x3B, an unassigned virtual key", Continue);
    QCOMPARE(QtKeyToWin(Qt::Key_Semicolon), size_t(VK_OEM_1));

    QEXPECT_FAIL("", "DUSK-3: Qt::Key_Equal is 0x3D, an unassigned virtual key", Continue);
    QCOMPARE(QtKeyToWin(Qt::Key_Equal), size_t(VK_OEM_PLUS));
}

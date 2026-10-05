#include "tst_hotkeyregistration.h"

#include <QTest>

#include "uglobalhotkeys.h"
#include "ukeysequence.h"

// These cover the guard in registerHotkey rather than the key map: refusing a
// virtual key of 0 has to happen before RegisterHotKey, because RegisterHotKey
// accepts 0 and reports success, registering a hotkey nothing can trigger.
//
// The id is arbitrary and the binding is released immediately, but it is a real
// system-wide registration, so the combinations below are deliberately ones
// nothing is likely to hold.

void tst_HotkeyRegistration::unmappableKeyIsRefused()
{
    UGlobalHotkeys hotkeys;
    UKeySequence sequence;

    sequence.addKey(Qt::Key_Control);
    sequence.addKey(Qt::Key_Alt);
    sequence.addKey(Qt::Key_Shift);
    sequence.addKey(Qt::Key_Hangul);

    QVERIFY(!hotkeys.registerHotkey(sequence, 4001));
}

// The key used to default to VK_F2, so a sequence naming only modifiers
// registered Ctrl+F2 and reported success.
void tst_HotkeyRegistration::sequenceWithNoKeyIsRefused()
{
    UGlobalHotkeys hotkeys;
    UKeySequence sequence;

    sequence.addKey(Qt::Key_Control);
    sequence.addKey(Qt::Key_Alt);
    sequence.addKey(Qt::Key_Shift);

    QVERIFY(!hotkeys.registerHotkey(sequence, 4002));
}

void tst_HotkeyRegistration::mappableKeyRegisters()
{
    UGlobalHotkeys hotkeys;
    UKeySequence sequence;

    sequence.addKey(Qt::Key_Control);
    sequence.addKey(Qt::Key_Alt);
    sequence.addKey(Qt::Key_Shift);
    sequence.addKey(Qt::Key_Period);

    QVERIFY(hotkeys.registerHotkey(sequence, 4003));

    hotkeys.unregisterHotkey(4003);
}

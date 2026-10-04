#include "tst_ukeysequence.h"

#include <QTest>

#include "ukeysequence.h"

// The out-of-range guard in UKeySequence::operator[] has regressed twice
// (GitHub #28, #53): without it, a sequence shorter than the index read past
// the end of mKeys.
void tst_UKeySequence::indexPastTheEndIsUnknown()
{
    UKeySequence seq("Ctrl+A");

    QCOMPARE(seq.size(), size_t(2));
    QCOMPARE(seq[0], Qt::Key_Control);
    QCOMPARE(seq[1], Qt::Key_A);
    QCOMPARE(seq[2], Qt::Key_unknown);
    QCOMPARE(seq[99], Qt::Key_unknown);
}

void tst_UKeySequence::indexIntoEmptySequenceIsUnknown()
{
    UKeySequence seq;

    QCOMPARE(seq.size(), size_t(0));
    QCOMPARE(seq[0], Qt::Key_unknown);
}

void tst_UKeySequence::fromStringSplitsModifiersAndKeys()
{
    UKeySequence seq("Ctrl+Shift+A");

    QCOMPARE(seq.size(), size_t(3));
    QCOMPARE(seq.getModifiers(), QVector<Qt::Key>({Qt::Key_Control, Qt::Key_Shift}));
    QCOMPARE(seq.getSimpleKeys(), QVector<Qt::Key>({Qt::Key_A}));
}

void tst_UKeySequence::modifierAliasesAreAccepted()
{
    QCOMPARE(UKeySequence("ctrl+a")[0], Qt::Key_Control);
    QCOMPARE(UKeySequence("control+a")[0], Qt::Key_Control);
    QCOMPARE(UKeySequence("shft+a")[0], Qt::Key_Shift);
    QCOMPARE(UKeySequence("win+a")[0], Qt::Key_Meta);
    QCOMPARE(UKeySequence("meta+a")[0], Qt::Key_Meta);
}

void tst_UKeySequence::duplicateKeysAreIgnored()
{
    UKeySequence seq("Ctrl+A");
    seq.addKey(Qt::Key_Control);
    seq.addKey(Qt::Key_A);

    QCOMPARE(seq.size(), size_t(2));
}

void tst_UKeySequence::invalidKeyStringsAreRejected()
{
    UKeySequence seq;

    QTest::ignoreMessage(QtWarningMsg, "Wrong key");
    seq.addKey(QString("A+B"));
    QTest::ignoreMessage(QtWarningMsg, "Wrong key");
    seq.addKey(QString("A,B"));

    seq.addKey(Qt::Key(0));
    seq.addKey(Qt::Key(-1));

    QCOMPARE(seq.size(), size_t(0));
}

// DUSK-38. QKeySequence parses an unknown name as one key whose value is
// Qt::Key_unknown, so addKey's "count() != 1" check doesn't reject it, and
// Qt::Key_unknown is positive so the "key <= 0" guard doesn't either.
void tst_UKeySequence::unparseableKeyNameIsRejected()
{
    UKeySequence seq;

    seq.addKey(QString("nonsense"));

    QEXPECT_FAIL("", "DUSK-38: the name is stored as Qt::Key_unknown instead of being refused", Continue);
    QCOMPARE(seq.size(), size_t(0));

    // Holds either way: an empty sequence also reads back as unknown.
    QCOMPARE(seq[0], Qt::Key_unknown);
}

void tst_UKeySequence::toStringPutsModifiersFirst()
{
    UKeySequence seq("A+Ctrl");

    QCOMPARE(seq.toString(), QString("Ctrl+A"));
}

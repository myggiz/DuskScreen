#ifndef TST_UKEYSEQUENCE_H
#define TST_UKEYSEQUENCE_H

#include <QObject>

class tst_UKeySequence : public QObject
{
    Q_OBJECT

private slots:
    void indexPastTheEndIsUnknown();
    void indexIntoEmptySequenceIsUnknown();
    void fromStringSplitsModifiersAndKeys();
    void modifierAliasesAreAccepted();
    void duplicateKeysAreIgnored();
    void invalidKeyStringsAreRejected();
    void unparseableKeyNameIsRejected();
    void toStringPutsModifiersFirst();
};

#endif // TST_UKEYSEQUENCE_H

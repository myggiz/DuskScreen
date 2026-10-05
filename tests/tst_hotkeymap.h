#ifndef TST_HOTKEYMAP_H
#define TST_HOTKEYMAP_H

#include <QObject>

class tst_HotkeyMap : public QObject
{
    Q_OBJECT

private slots:
    void namedKeysMapToTheirVirtualKey_data();
    void namedKeysMapToTheirVirtualKey();
    void lettersAndDigitsFallThroughUnchanged_data();
    void lettersAndDigitsFallThroughUnchanged();
    void punctuationMatchesTheActiveLayout_data();
    void punctuationMatchesTheActiveLayout();
    void periodResolvesToARealKey();
    void punctuationNoLongerGrabsUnrelatedKeys();
    void unmappableKeysAreRefused_data();
    void unmappableKeysAreRefused();
};

#endif // TST_HOTKEYMAP_H

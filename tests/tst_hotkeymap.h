#ifndef TST_HOTKEYMAP_H
#define TST_HOTKEYMAP_H

#include <QObject>

class tst_HotkeyMap : public QObject
{
    Q_OBJECT

private slots:
    void namedKeysMapToTheirVirtualKey_data();
    void namedKeysMapToTheirVirtualKey();
    void mainRowPunctuationUsesOemCodes();
    void lettersAndDigitsFallThroughUnchanged_data();
    void lettersAndDigitsFallThroughUnchanged();
    void unmappedPunctuationCollidesWithUnrelatedKeys();
};

#endif // TST_HOTKEYMAP_H

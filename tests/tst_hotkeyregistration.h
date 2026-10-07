#ifndef TST_HOTKEYREGISTRATION_H
#define TST_HOTKEYREGISTRATION_H

#include <QObject>

class tst_HotkeyRegistration : public QObject
{
    Q_OBJECT

private slots:
    void unmappableKeyIsRefused();
    void sequenceWithNoKeyIsRefused();
    void mappableKeyRegisters();
};

#endif // TST_HOTKEYREGISTRATION_H

#ifndef TESTPROTOCOLSETTINGS_H
#define TESTPROTOCOLSETTINGS_H

#include <QObject>

//Тесты хранилища протокола и настроек связи
class TestProtocolSettings : public QObject
{
    Q_OBJECT

private slots:
    void protocolDefaults();
    void settingsDefaults();
    void storesProtocol();
    void storesSettings();
    void mutableSettingsWritesThrough();
    void referencesStayStable();
};

#endif // TESTPROTOCOLSETTINGS_H

#ifndef TESTPROFILEDATA_H
#define TESTPROFILEDATA_H

#include <QObject>

//Тесты разбора и записи профиля .eag
class TestProfileData : public QObject
{
    Q_OBJECT

private slots:
    void parsesProtocolFields();
    void parsesConnectionSettings();
    void missingSettingsIsReported();
    void emptyTextGivesProtocolDefaults();
    void ignoresGarbageLines();
    void parsesMaskLine();
    void parsesMaskLineWithoutTrailingTab();
    void skipsIncompleteMaskLine();
    void handlesCrlfLineEndings();
    void serializedHeaderParsesBack();
    void serializedMaskLineRoundTrip();
};

#endif // TESTPROFILEDATA_H

#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QVariantMap>
#include <QQueue>
#include <QDateTime>
#include "global.h"

class Logger : public QObject
{
    Q_OBJECT
public:
    Logger();

public slots:
    void startLog();
    void stopLog();
    void setBin(bool);
    void setTxt(bool);
    void setJson(bool);
    void setProfileName(QString);
    void incomingBinData(const QByteArray data);
    void incomingTxtData(const QString string);
    void incomingJsonData(const QVariantMap jsonMap);
    void binReadFromCsv(bool r);

private:
    QFile newBinFile;
    QFile newLogFile;
    QFile newJsonFile;
    QString binFileName;
    QString logFileName;
    QString jsonFileName;
    bool bin = false, txt = false, json = false;
    bool createNewBinFileNamePermission = false,
         createNewJsonFileNamePermission = false,
         createNewTxtFileNamePermission = false;
    QDateTime returnTimestamp();
    const QString timeFormat = "dd.MM.yy_hh:mm:ss::zzz";
    const QString timeFormatForFile = "dd.MM.yy_hh-mm-ss";
    QString sessionName;
    QString appHomeDir;
    QDir dir;
    QString currentProfileName;
    bool writeLogsPermission = false;
    QQueue<QString> txtLogQueue;
    s_Settings settings;
    QMap<QDateTime, QVector<uint8_t >> *rawDataWithTimeLog = nullptr;

signals:
    void showStatusMessage(QString);
    void toTextLog(QString text, bool redFlag);
    void setSettings(s_Settings);
    void readFromCsv(QMap<QDateTime, QVector<uint8_t >> );

};

#endif // LOGGER_H

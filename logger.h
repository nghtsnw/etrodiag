#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QVariantMap>
#include <QQueue>
#include <QDateTime>
#include "global.h"

class ProtocolSettings;

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
    void setModel(const ProtocolSettings *model); //настройки (путь к логу) читаем из модели

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
    const QString timeFormat = "dd.MM.yyyy_HH:mm:ss.zzz";
    const QString timeFormatForFile = "dd.MM.yy_HH-mm-ss";
    QString sessionName;
    QDir dir;
    QString currentProfileName;
    bool writeLogsPermission = false;
    QQueue<QString> txtLogQueue;
    const ProtocolSettings *m_model = nullptr;
    QMap<QDateTime, QVector<uint8_t >> *rawDataWithTimeLog = nullptr;

signals:
    void showStatusMessage(QString);
    void logLoadProgress(int percent);
    void toTextLog(QString text, bool redFlag);
    void readFromCsv(QMap<QDateTime, QVector<uint8_t >> );

};

#endif // LOGGER_H

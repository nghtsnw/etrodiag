#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QVariantMap>
#include <QQueue>

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
    QString sessionName;    
    QString appHomeDir;
    QDir dir;
    QString currentProfileName;
    bool writeLogsPermission = false;
    QQueue<QString> txtLogQueue;

signals:
    void showStatusMessage(QString);
    void toTextLog(QString text, bool redFlag);
};

#endif // LOGGER_H

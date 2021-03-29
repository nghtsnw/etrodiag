#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QVariantMap>

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
    void incomingBinData(QByteArray data);
    void incomingTxtData(QString string);
    void incomingJsonData(QVariantMap jsonMap);

private:
    QFile newBinFile;
    QFile newLogFile;
    QFile newJsonFile;
    QString binFileName;
    QString logFileName;
    QString jsonFileName;
    bool bin, txt, json;
    bool createNewBinFileNamePermission, createNewJsonFileNamePermission, createNewTxtFileNamePermission;
    QDateTime returnTimestamp();
    QString sessionName;    
    QString appHomeDir;
    QDir dir;
    QString currentProfileName;
    bool writeLogsPermission = false;

signals:
    void showStatusMessage(QString);
    void toTextLog(QString);
};

#endif // LOGGER_H

#include "logger.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QDataStream>
#include <QStandardPaths>
#include <QApplication>

Logger::Logger()
{
    #ifdef Q_OS_WIN32
       appHomeDir = qApp->applicationDirPath() + QDir::separator();
    #endif
    #ifdef Q_OS_ANDROID
        appHomeDir = QStandardPaths::standardLocations(QStandardPaths::DataLocation)[1] + QDir::separator();
    #endif
    dir.setPath(appHomeDir + "Logs");
    if (!dir.exists())
    QDir().mkdir(appHomeDir + "Logs");
}

void Logger::startLog()
{
    createNewBinFileNamePermission = true;
    createNewTxtFileNamePermission = true;
    createNewJsonFileNamePermission = true;
    writeLogsPermission = true;
}

void Logger::stopLog()
{
    newBinFile.close();
    newLogFile.close();
    newJsonFile.close();
    writeLogsPermission = false;
}

void Logger::incomingBinData(QByteArray data)
{    
    if (bin && writeLogsPermission)
    {        
        if (!newBinFile.isOpen())
        {
            if (createNewBinFileNamePermission)
            {
                binFileName = (dir.path() + "\\" + currentProfileName + '_' + returnTimestamp().toString("dd.MM.yy_hh-mm-ss") + ".bin");
                createNewBinFileNamePermission = false;
            }
            newBinFile.setFileName(binFileName);
            if (!newBinFile.exists())
            {
                newBinFile.open(QIODevice::WriteOnly);
                emit toTextLog(QString(tr("Start write log file ")) + newBinFile.fileName());
            }
            else newBinFile.open(QIODevice::Append);
        }
        if (newBinFile.isOpen())
        {
            QDataStream binStream(&newBinFile);
            binStream.writeRawData(data.constData(), data.size());
            newBinFile.close();
        }
        else emit showStatusMessage(tr("Error write bin"));
    }
    else if (!bin && !createNewBinFileNamePermission)
    {
            emit toTextLog(QString(tr("Stop write bin file")));
            newBinFile.close();
            createNewBinFileNamePermission = true;
    }
}

void Logger::incomingTxtData(QString string)
{
    if (txt && writeLogsPermission)
    {        
        if (!newLogFile.isOpen())
        {
            if (createNewTxtFileNamePermission)
            {
                logFileName = (dir.path() + "\\" + currentProfileName + '_' + returnTimestamp().toString("dd.MM.yy_hh-mm-ss") + ".log");
                createNewTxtFileNamePermission = false;
            }
            newLogFile.setFileName(logFileName);
            if (!newLogFile.exists())
            {
                newLogFile.open(QIODevice::WriteOnly|QIODevice::Text);
                emit toTextLog(QString(tr("Start write log file ")) + newLogFile.fileName());
            }
            else newLogFile.open(QIODevice::Append|QIODevice::Text);
        }
        if (newLogFile.isOpen())
        {
            QTextStream logStream(&newLogFile);
            logStream << string << '\n';
            newLogFile.close();
        }
        else emit showStatusMessage(tr("Error write log"));
    }
    else if (!txt && !createNewTxtFileNamePermission)
    {
            emit toTextLog(QString(tr("Stop write log file")));
            newLogFile.close();
            createNewTxtFileNamePermission = true;
    }
}

void Logger::incomingJsonData(QVariantMap jsonMap)
{    
    if (json && writeLogsPermission)
    {
        if (!newJsonFile.isOpen())
        {
            if (createNewJsonFileNamePermission)
            {
                jsonFileName = (dir.path() + "\\" + currentProfileName + '_' + returnTimestamp().toString("dd.MM.yy_hh-mm-ss") + ".json");
                createNewJsonFileNamePermission = false;
            }
            newJsonFile.setFileName(jsonFileName);
            if (!newJsonFile.exists())
            {
                newJsonFile.open(QIODevice::WriteOnly|QIODevice::Text);
                emit toTextLog(QString(tr("Start write json file ")) + newJsonFile.fileName());
            }
            else newJsonFile.open(QIODevice::Append|QIODevice::Text);
        }
        if (newJsonFile.isOpen())
        {
            QJsonObject jsonObj = QJsonObject::fromVariantMap(jsonMap);
            newJsonFile.write(QJsonDocument(jsonObj).toJson(QJsonDocument::Compact));
            QTextStream newLineStream(&newJsonFile);
            newLineStream << '\n';
            newJsonFile.close();
        }
        else emit showStatusMessage(tr("Error write json"));
    }
    else if (!json && !createNewJsonFileNamePermission)
    {
            emit toTextLog(QString(tr("Stop write json file")) + "</span></p>");
            newJsonFile.close();
            createNewJsonFileNamePermission = true;
    }
}

QDateTime Logger::returnTimestamp()
{
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    QDateTime dt3 = QDateTime::fromMSecsSinceEpoch(timestamp);
    return dt3;
}

void Logger::setBin(bool b)
{
    bin = b;
}

void Logger::setTxt(bool b)
{
    txt = b;
}

void Logger::setJson(bool b)
{
    json = b;
}

void Logger::setProfileName(QString name)
{
    currentProfileName = name;
}

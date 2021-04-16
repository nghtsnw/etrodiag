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
    writeLogsPermission = false;
    newBinFile.close();
    newLogFile.close();
    newJsonFile.close();    
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
                emit toTextLog(QString(tr("Start write bin file ")) + newBinFile.fileName());
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
            newBinFile.close();
            createNewBinFileNamePermission = true;
            emit toTextLog(QString(tr("Stop write bin file")));
    }
}

void Logger::incomingTxtData(QString string)
{
    if (txt && writeLogsPermission)//если стоит галка в настройках и есть разрешение на писание логов
    {        
        if (!newLogFile.isOpen())
        {
            if (createNewTxtFileNamePermission)//обновляем имя файла, если стоит флаг
            {
                logFileName = (dir.path() + "\\" + currentProfileName + '_' + returnTimestamp().toString("dd.MM.yy_hh-mm-ss") + ".log");
                newLogFile.setFileName(logFileName);
                createNewTxtFileNamePermission = false;
            }
            if (!newLogFile.exists() && !logFileName.isEmpty()) //если файла нет - создаём
            {
                newLogFile.open(QIODevice::WriteOnly|QIODevice::Text);
                if (newLogFile.isOpen())
                emit toTextLog(QString(tr("Start write log file ")) + newLogFile.fileName());
                else emit toTextLog(tr("Error open log file"));
            }
            else newLogFile.open(QIODevice::Append|QIODevice::Text); //если есть - открываем на дописывание
            if (newLogFile.isOpen())//если файл открыт - пишем
            {
                QTextStream logStream(&newLogFile);
                while (!txtLogQueue.isEmpty())
                    logStream << txtLogQueue.dequeue() << '\n';
                logStream << string << '\n';
                newLogFile.close();//закрываем после записи
            }
            else emit toTextLog(tr("Error open log file"));
        }
        else {
                txtLogQueue.enqueue(string);
        }
    }
    if (!txt && !createNewTxtFileNamePermission)//если сняли галку в настройках при активном соединении, закрываем файл
    {
        newLogFile.close();
        createNewTxtFileNamePermission = true;
        txtLogQueue.clear();
        emit toTextLog(QString(tr("Stop write log file")));
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
            newJsonFile.close();
            createNewJsonFileNamePermission = true;
            emit toTextLog(QString(tr("Stop write json file")));
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

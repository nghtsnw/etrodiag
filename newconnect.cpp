#include "newconnect.h"
#include "ui_newconnect.h"
#include <QSerialPort>
#include "settingsdialog.h"
#include "console.h"
#include <QtWidgets>
#include "mainwindow.h"
#include <QDebug>
#include "getstream.h"
#include "dataprofiler.h"
#include "txtmaskobj.h"
#include <QStandardPaths>

newconnect::newconnect(QWidget *parent) :
    QWidget(parent),
    m_settings (new SettingsDialog),
    m_serial (new QSerialPort(this)),
    m_console (new Console),
    m_status (new QLabel),
    gstream (new getStream),
    datapool (new dataprofiler),
    ui(new Ui::newconnect)
{
    ui->setupUi(this);
    m_console->setEnabled(false);
    m_console->setParent(ui->consoleFrame);
    m_console->show();
    #ifdef Q_OS_WIN32
        appHomeDir = qApp->applicationDirPath() + QDir::separator();
    #endif
    #ifdef Q_OS_ANDROID
        appHomeDir = QStandardPaths::standardLocations(QStandardPaths::DataLocation)[1] + QDir::separator();
    #endif
    qDebug() << appHomeDir;
    connect(m_serial, &QSerialPort::errorOccurred, this, &newconnect::handleError);
    connect(m_serial, &QSerialPort::readyRead, this, &newconnect::readData);
    connect(m_console, &Console::getData, this, &newconnect::writeData);
    connect(gstream, &getStream::giveMyByte, datapool, &dataprofiler::getByte);
    connect(datapool, &dataprofiler::deviceData, this, &newconnect::transData);
    connect(m_settings, &SettingsDialog::restoreConsoleAndButtons, this, &newconnect::restoreWindowAfterApplySettings);
    connect (m_settings, &SettingsDialog::writeTextLog, this, &newconnect::writeTextLog);
    connect (m_settings, &SettingsDialog::writeBinLog, this, &newconnect::writeBinLogSlot);
    connect (timer, &QTimer::timeout, this, &newconnect::readFromFile);
    on_settingsButton_clicked();

}

newconnect::~newconnect()
{
    delete m_serial;
    delete ui;
}

void newconnect::on_settingsButton_clicked()
{
    m_settings->setParent(this);
    m_console->hide();
    ui->connectButton->hide();
    ui->settingsButton->hide();
    ui->consoleFrame->hide();
    m_settings->show();
}

void newconnect::openSerialPort()
{    
    const SettingsDialog::Settings p = m_settings->settings();
    p_local = m_settings->settings();
    if (p.readFromFileFlag)
    {
        readProfile();        
        showStatusMessage(tr("Read file %1").arg(p.pathToBinFile));
        pos = 0;
        fileSplitted.clear();
        int freq = 1000/((p_local.baudRate/8)/bytesPerOneShot);
        QFile file(p_local.pathToBinFile);
        file.open(QIODevice::ReadOnly);
        QByteArray fileBuffer = file.readAll();
        for (int i = 0; i < fileBuffer.size();)
        {
            static QByteArray ch;
            while (ch.size() < bytesPerOneShot && i < fileBuffer.size())
            {
                ch.append(fileBuffer.at(i));
                i++;
            }
            fileSplitted.push_back(ch);
            ch.clear();
        }
        fileBuffer.clear();
        timer->start(freq);
    }
    else
    {
    qDebug() << "Open serial port " << p.name;
    m_serial->setPortName(p.name);
    m_serial->setBaudRate(p.baudRate);
    m_serial->setDataBits(p.dataBits);
    m_serial->setParity(p.parity);
    m_serial->setStopBits(p.stopBits);
    m_serial->setFlowControl(p.flowControl);
    if (m_serial->open(QIODevice::ReadWrite)) {
        readProfile();
        m_console->setEnabled(true);          
        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6, %7")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl).arg(p.profilePath));
    } else {
                QMessageBox::critical(this, tr("Error"), m_serial->errorString());
                showStatusMessage(tr("Open error"));
           }
    }
}

void newconnect::readFromFile()
{
    if (pos < fileSplitted.size())
    {
        fsba.clear();
        fsba.append(fileSplitted.at(pos));
        pos++;
        readData();
    }
    else
    {
        showStatusMessage(tr("End of file"));
        timer->stop();
        on_connectButton_clicked();
    }
}

void newconnect::closeSerialPort()
{
    if (m_serial->isOpen())
    {
        m_serial->close();
        showStatusMessage(tr("Disconnected"));
    }
    if (p_local.readFromFileFlag) p_local.readFromFileFlag = false;
}

void newconnect::writeData(const QByteArray &data)
{
    m_serial->write(data);
}

void newconnect::readData()
{
    static QByteArray data;
    if (p_local.readFromFileFlag) data = fsba;
    else data = m_serial->readAll();
    m_console->putData(data);
    gstream->getRawData(data);

    QFile newBinFile;
    if (writeBinLog)
    {
        QDir dir(appHomeDir + "Logs");
        if (!dir.exists())
        QDir().mkdir(appHomeDir + "Logs");

        if (!newBinFile.isOpen())
        {
            if (createNewFileNamePermission)
            {
                binFileName = (dir.path() + returnTimestamp().toString("\\dd.MM.yy_hh-mm-ss") + ".bin");
                createNewFileNamePermission = false;
            }
            newBinFile.setFileName(binFileName);
            if (!newBinFile.exists())
            {
                newBinFile.open(QIODevice::WriteOnly);
                emit directly2logArea("<p><span style=color:#ff0000>" + returnTimestamp().toString("hh:mm:ss:zzz") + " "
                                          + QString("Start write log file ") + newBinFile.fileName() + "</span></p>");
            }
            else newBinFile.open(QIODevice::Append);
        }
        if (newBinFile.isOpen())
        {
            QDataStream binStream(&newBinFile);
            binStream << data;
            newBinFile.close();
        }
        else showStatusMessage("Error write bin");
    }
    else if (!writeBinLog && !createNewFileNamePermission)
    {
            emit directly2logArea("<p><span style=color:#ff0000>" + returnTimestamp().toString("hh:mm:ss:zzz") + " "
                                  + QString("Stop write bin file") + "</span></p>");
            newBinFile.close();
            createNewFileNamePermission = true;
    }
    data.clear();
}

void newconnect::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), m_serial->errorString());
        closeSerialPort();
    }
}

void newconnect::showStatusMessage(QString message)
{
    m_status->setText(message);
    emit sendStatusStr(message);
}

void newconnect::on_connectButton_clicked()
{
    if (m_serial->isOpen() || p_local.readFromFileFlag)
        {
            this->closeSerialPort();
            p_local.readFromFileFlag = false;
            if (!(m_serial->isOpen()) && !p_local.readFromFileFlag)
            {
                ui->connectButton->setText("Connect");
                showStatusMessage("Connection closed");
            }
        }
    else if (!(m_serial->isOpen()) || !p_local.readFromFileFlag)
    {
        openSerialPort();
        if (m_serial->isOpen() || p_local.readFromFileFlag)
        {
            ui->connectButton->setText("Disconnect");
        }
    }
}

void newconnect::transData(QVector<int> snapshot)
{
    emit transmitData(snapshot);
}

void newconnect::prepareToSaveProfile()
{
    const SettingsDialog::Settings p = m_settings->settings();

    if (!p.readOnlyProfile)
    {//очищаем список, выставляем разрешение для дальнейших операций по сохранению, даём сигнал на запрос всех масок
        maskVectorsList = this->findChildren<txtmaskobj*>();
        QListIterator<txtmaskobj*> maskVectorsListIt(maskVectorsList);
        maskVectorsListIt.toFront();
        while (maskVectorsListIt.hasNext())
        maskVectorsListIt.next()->~txtmaskobj();
        permission2SaveMasks = true;
        qDebug() << "prepare to save profile";
        emit saveAllMasks();
    }
}

void newconnect::saveProfileSlot4Masks(int devNum, QString devName, int byteNum, QString byteName, int id, QString paramName, QString paramMask, int, double valueShift, double valueKoef, bool viewInLogFlag, int wordType, bool _drawGraphFlag, QString _drawGraphColor)
     {
               //перед сохранением все маски сигналом отправляются сюда, что-бы образовать перечень масок
               //проверяется что этой маски тут ещё нет, после этого создаётся список с текстовым перечнем всех параметров
               //создаются только описания масок, само сохранение будет в другой функции
                qDebug() << "create obj to save mask " << paramName;
               if (permission2SaveMasks)
               {
                maskVectorsList = this->findChildren<txtmaskobj*>();
                QListIterator<txtmaskobj*> maskVectorsListIt(maskVectorsList);
                bool thisMaskHere = false;
                maskVectorsListIt.toFront();
                while (maskVectorsListIt.hasNext())
               {
                   if ((QString::number(id,10) == maskVectorsListIt.peekNext()->lst.at(1))&&(QString::number(devNum,10) == maskVectorsListIt.peekNext()->lst.at(2))&&(QString::number(byteNum,10)==maskVectorsListIt.peekNext()->lst.at(3))&& (maskVectorsListIt.peekNext()->lst.at(0) == "thisIsMask"))
                      thisMaskHere = true;
                   maskVectorsListIt.next();
               }
               if (!thisMaskHere)
               {
                   QList<QString> maskList;
                   maskList.append("thisIsMask");//0
                   maskList.append(QString::number(id,10));//1
                   maskList.append(QString::number(devNum,10));//2
                   maskList.append(QString::number(byteNum,10));//3
                   maskList.append(devName);//4
                   maskList.append(byteName);//5
                   maskList.append(paramName);//6
                   maskList.append(paramMask);//7
                   maskList.append(QString::number(valueShift,'g',6));//8
                   maskList.append(QString::number(valueKoef,'g',6));//9
                   maskList.append((viewInLogFlag ?"true":"false"));//10
                   maskList.append(QString::number(wordType));//11
                   maskList.append(_drawGraphFlag ?"true":"false");//12
                   maskList.append(_drawGraphColor);//13
                   txtmaskobj *savingMask = new txtmaskobj(maskList);
                   qDebug() << "create obj to save mask " << maskList[6];
                   savingMask->setParent(this);
                   maskList.clear();
               }
           }
           }

void newconnect::saveProfile()
{
    if (permission2SaveMasks)
    {
    maskVectorsList = this->findChildren<txtmaskobj*>();
    QListIterator<txtmaskobj*> maskVectorsListIt(maskVectorsList);
    maskVectorsListIt.toFront();
    const SettingsDialog::Settings p = m_settings->settings();
    QFile profile(p.profilePath);
    QFileInfo info(profile);
    profile.open(QIODevice::WriteOnly|QIODevice::Text);
    QTextStream txtStream(&profile);
    txtStream.setCodec("UTF-8");
    txtStream << info.fileName() << "\n";
    while (maskVectorsListIt.hasNext())
    {
        qDebug() << "saving in progress...";
        QListIterator<QString> lstIt(maskVectorsListIt.peekNext()->lst);
        while (lstIt.hasNext())
        txtStream << lstIt.next() << "\t";
        txtStream << "\n";
        maskVectorsListIt.next();
    }
    permission2SaveMasks = false;
    emit sendStatusStr("Profile " + info.fileName() + " saved");
}
}

void newconnect::readProfile()
{
    emit cleanDevListSig();
    const SettingsDialog::Settings p = m_settings->settings();
    QFile profile(p.profilePath);
    profile.open(QIODevice::ReadOnly|QIODevice::Text);
    QTextStream txtStream(&profile);
    //txtStream.setCodec("UTF-8");
    while (!txtStream.atEnd())
    {
        QString str = txtStream.readLine();
        QStringList strLst = str.split('\t');
        if (strLst.at(0)=="thisIsMask")
            emit loadMask(strLst.at(2).toInt(0,10),strLst.at(4),strLst.at(3).toInt(0,10),strLst.at(5),strLst.at(1).toInt(0,10),strLst.at(6),strLst.at(7),0,strLst.at(8).toDouble(),strLst.at(9).toDouble(),((QString::compare(strLst.at(10), "true") == 0) ? true : false),strLst.at(11).toInt(0,10), ((QString::compare(strLst.at(12), "true") == 0) ? true : false), strLst.at(13));
        strLst.clear();
    }
}

void newconnect::resizeEvent(QResizeEvent *event)
{
    if (event)
    {
        m_console->resize(event->size());
        m_settings->resize(event->size());
    }
}

void newconnect::restoreWindowAfterApplySettings()
{
    m_console->show();
    ui->connectButton->show();
    ui->settingsButton->show();
    ui->consoleFrame->show();
}

void newconnect::writeBinLogSlot(bool arg)
{
    writeBinLog = arg;
}

QDateTime newconnect::returnTimestamp()
{
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    QDateTime dt3 = QDateTime::fromMSecsSinceEpoch(timestamp);
    return dt3;
}


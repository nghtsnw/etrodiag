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
#include <global.h>

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
    connect(m_serial, &QSerialPort::errorOccurred, this, &newconnect::handleError);
    connect(m_serial, &QSerialPort::readyRead, this, &newconnect::readData);
    connect(m_console, &Console::getData, this, &newconnect::writeData);
    connect(gstream, &getStream::giveMyByte, datapool, &dataprofiler::getByte);
    connect(datapool, &dataprofiler::deviceData, this, &newconnect::transmitData);
    connect(datapool, &dataprofiler::deviceData, this, [this]() {
        timerAboveTxCommand->start(1);
    });/*После успешного приёма задержка перед отправкой команды */
    connect(datapool, &dataprofiler::badCRC, this, &newconnect::badCRC);
    connect(datapool, &dataprofiler::ready4read, gstream, &getStream::readPermission);
    connect(datapool, &dataprofiler::readNext, gstream, &getStream::readIntByte);
    connect(m_settings, &SettingsDialog::restoreConsoleAndButtons, this, &newconnect::restoreWindowAfterApplySettings);
    connect (m_settings, &SettingsDialog::prepareToSaveProfile, this, &newconnect::prepareToSaveProfile);
    connect (m_settings, &SettingsDialog::saveProfile, this, &newconnect::saveProfile);
    connect (m_settings, &SettingsDialog::writeBinLog, this, &newconnect::writeBinLog);
    connect (m_settings, &SettingsDialog::writeJsonLog, this, &newconnect::writeJsonLog);
    /*----------------------------------------------------------------------------------------------------------------------------*/
    //При загрузке данных из профиля, они отправляются в окно настроек в UI
    connect (this, &newconnect::loadProtocol, m_settings, &SettingsDialog::loadProtocol);
    /*----------------------------------------------------------------------------------------------------------------------------*/
    /*----------------------------------------------------------------------------------------------------------------------------*/
    //По применению настроек в UI, они сразу применяются на датаразборке, копия в newconnect для сохранения профиля
    connect (m_settings, &SettingsDialog::setProtocol, datapool, &dataprofiler::setProtocol);
    connect (m_settings, &SettingsDialog::setProtocol, this, [this](s_protocolDescription p) {
        protocol = p;
    });
    /*----------------------------------------------------------------------------------------------------------------------------*/
    /*----------------------------------------------------------------------------------------------------------------------------*/
    //При загрузке данных из профиля, они сразу применяются на датаразборке
    connect (this, &newconnect::loadProtocol, datapool, &dataprofiler::setProtocol);
    /*----------------------------------------------------------------------------------------------------------------------------*/
    connect (m_settings, &SettingsDialog::loadSelectedProfile, this, &newconnect::readProfile);
    connect (timer, &QTimer::timeout, this, &newconnect::readFromFile);//читаем из файла по таймеру
    connect (timerAboveTxCommand, &QTimer::timeout, this, &newconnect::sendCommand);//отправляем команду после задержки
    connect (this, &newconnect::sendRawData, gstream, &getStream::getRawData);
    connect (this, &newconnect::sendRawData, m_console, &Console::putData);
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
        pos = 0;//задаём позицию для чтения FileSplitted в readFromFile()
        fileSplitted.clear();
        int freq = 1000 / ((p_local.baudRate / 8) / bytesPerOneShot);
        QFile file(p_local.pathToBinFile);
        file.open(QIODevice::ReadOnly);
        showStatusMessage(tr("Bufferisation..."));
        QByteArray fileBuffer = file.readAll();//читаем весь файл в память
        for (int i = 0; i < fileBuffer.size();)
        {
            static QByteArray ch;
            while (ch.size() < bytesPerOneShot && i < fileBuffer.size())//создаём список FileSplitted с кусками файла fileBuffer равными bytesPerOneShot
            {
                ch.append(fileBuffer.at(i));
                i++;
            }
            fileSplitted.push_back(ch);
            ch.clear();
        }
        fileBuffer.clear();
        showStatusMessage(tr("Read file %1").arg(p.pathToBinFile));
        timer->start(freq);//запускаем таймер, по нему читается по порядку FileSplitted функцией readFromFile()
    }
    else
    {
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
        }
        else {
            QMessageBox::critical(this, tr("Error"), m_serial->errorString());
            showStatusMessage(tr("Open error"));
        }
    }
}

void newconnect::readFromFile()
{
    if (pos < fileSplitted.size())
    { //если текущая позиция не в конце списка (костыль вместо итератора) то кусок по нужному номеру листа добавляем в fsba
        fsba.append(fileSplitted.at(pos));//добавляем кусок по указателю
        pos++;
        readData();//вызываем читалку данных
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
    if (p_local.readFromFileFlag)
    {
        p_local.readFromFileFlag = false;
        timer->stop();
    }
}

void newconnect::writeData(const QByteArray &data)
{
    m_serial->write(data);
}

void newconnect::readData()
{
    static QByteArray data;
    if (p_local.readFromFileFlag)
    {
        data = fsba;//если есть флаг чтения из файла, то читаем из fsba
        fsba.clear();
    }
    else
    {
        data = m_serial->readAll();//если нет то читаем всё что есть с порта
    }
    emit sendRawData(data);
    data.clear();
}

void newconnect::receiveCommandFromGui(QVector<quint8> command, bool newcommandflag)
{ // Тут принимаем команду сформированную в GUI
    toTransmit = command;
    newcommand = newcommandflag;
}

void newconnect::sendCommand()
{ // Отправляем команду контроллеру по сигналу после приёма
    if (m_serial->isOpen() && newcommand) {
        //if (!newcommand) toTransmit = {0xFF, 0xAB, 0, 0, 0};
        toTransmit.last() = calcCrc(toTransmit);
        QByteArray ba;
        for (auto i : qAsConst(toTransmit)) {
            ba.append(i);
        }
        writeData(ba);
        if (newcommand) {
            newcommand = false;
        }
    }
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
            ui->connectButton->setText(tr("Connect"));
            showStatusMessage(tr("Connection closed"));
        }
        emit stopLog();
    }
    else if (!(m_serial->isOpen()) || !p_local.readFromFileFlag)
    {
        openSerialPort();
        if (m_serial->isOpen() || p_local.readFromFileFlag)
        {
            emit startLog();
            emit cleanGraph();
            createNewFileNamePermission = true;
            ui->connectButton->setText(tr("Disconnect"));
        }
    }
}

void newconnect::prepareToSaveProfile()
{
    const SettingsDialog::Settings p = m_settings->settings();
    if (!p.readOnlyProfile)
    { //очищаем список, выставляем разрешение для дальнейших операций по сохранению, даём сигнал на запрос всех масок
        maskVectorsList = this->findChildren<txtmaskobj*>();
        QListIterator<txtmaskobj*> maskVectorsListIt(maskVectorsList);
        maskVectorsListIt.toFront();
        while (maskVectorsListIt.hasNext()) {
            maskVectorsListIt.next()->~txtmaskobj();
        }
        permission2SaveMasks = true;
        emit saveAllMasks();
    }
}

void newconnect::saveProfileSlot4Masks(int devNum, QString devName, int byteNum, QString byteName, int id, QString paramName, QString paramMask, int, double valueShift, double valueKoef, bool viewInLogFlag, int wordType, bool _drawGraphFlag, QString _drawGraphColor)
{
    //перед сохранением все маски сигналом отправляются сюда, что-бы образовать перечень масок
    //проверяется что этой маски тут ещё нет, после этого создаётся список с текстовым перечнем всех параметров
    //создаются только описания масок, само сохранение будет в другой функции
    if (permission2SaveMasks)
    {
        maskVectorsList = this->findChildren<txtmaskobj*>();
        QListIterator<txtmaskobj*> maskVectorsListIt(maskVectorsList);
        bool thisMaskHere = false;
        maskVectorsListIt.toFront();
        while (maskVectorsListIt.hasNext())
        {
            if ((QString::number(id, 10) == maskVectorsListIt.peekNext()->lst.at(1)) && (QString::number(devNum, 10) == maskVectorsListIt.peekNext()->lst.at(2)) && (QString::number(byteNum, 10) == maskVectorsListIt.peekNext()->lst.at(3)) && (maskVectorsListIt.peekNext()->lst.at(0) == "thisIsMask")) {
                thisMaskHere = true;
            }
            maskVectorsListIt.next();
        }
        if (!thisMaskHere)
        {
            QList<QString> maskList;
            maskList.append("thisIsMask");//0
            maskList.append(QString::number(id, 10)); //1
            maskList.append(QString::number(devNum, 10)); //2
            maskList.append(QString::number(byteNum, 10)); //3
            maskList.append(devName);//4
            maskList.append(byteName);//5
            maskList.append(paramName);//6
            maskList.append(paramMask);//7
            maskList.append(QString::number(valueShift, 'g', 6)); //8
            maskList.append(QString::number(valueKoef, 'g', 6)); //9
            maskList.append((viewInLogFlag ? "true" : "false")); //10
            maskList.append(QString::number(wordType));//11
            maskList.append(_drawGraphFlag ? "true" : "false"); //12
            maskList.append(_drawGraphColor);//13
            txtmaskobj *savingMask = new txtmaskobj(maskList);
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
        profile.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream txtStream(&profile);
        txtStream << info.fileName() << "\n";
        txtStream << "packetSize" << "\t" << QString::number(protocol.packetSize, 10) << "\n";
        txtStream << "blockIdentifycatorPosition" << "\t" << QString::number(protocol.blockIdentifycatorPosition, 10) << "\n";
        txtStream << "calcCRCFromPosition" << "\t" << QString::number(protocol.calcCRCFromPosition, 10) << "\n";
        txtStream << "markerPacketBeginSize" << "\t" << QString::number(protocol.markerPacketBeginSize, 10) << "\n";
        txtStream << "markerPacketBeginTextB1" << "\t" << QString::number(protocol.markerPacketBeginByte1, 16) << "\n";
        txtStream << "markerPacketBeginTextB2" << "\t" << QString::number(protocol.markerPacketBeginByte2, 16) << "\n";
        txtStream << "timeoutAfterLastByte" << "\t" << QString::number(protocol.timeoutAfterLastByte, 10) << "\n";
        txtStream << "description" << "\t" << protocol.description << "\n";
        txtStream << "varControl" << "\t" << (protocol.varControl ? "true" : "false") << "\n";
        while (maskVectorsListIt.hasNext())
        {
            QListIterator<QString> lstIt(maskVectorsListIt.peekNext()->lst);
            while (lstIt.hasNext()) {
                txtStream << lstIt.next() << "\t";
            }
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
    s_protocolDescription pt;
    QFile profile(p.profilePath);
    QFileInfo info(profile);
    currentProfileName = getProfileNameFromInfo(info);
    emit profileName2log(currentProfileName);
    profile.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream txtStream(&profile);
    while (!txtStream.atEnd())
    {
        QString str = txtStream.readLine();
        QStringList strLst = str.split('\t');
        if (strLst.at(0) == "thisIsMask") {
            emit loadMask(strLst.at(2).toInt(0, 10), strLst.at(4), strLst.at(3).toInt(0, 10), strLst.at(5), strLst.at(1).toInt(0, 10), strLst.at(6), strLst.at(7), 0, strLst.at(8).toDouble(), strLst.at(9).toDouble(), ((QString::compare(strLst.at(10), "true") == 0) ? true : false), strLst.at(11).toInt(0, 10), ((QString::compare(strLst.at(12), "true") == 0) ? true : false), strLst.at(13));
        }
        if (strLst.at(0) == "packetSize") {
            pt.packetSize = (strLst.at(1).toInt(0, 10));
        }
        if (strLst.at(0) == "blockIdentifycatorPosition") {
            pt.blockIdentifycatorPosition = (strLst.at(1).toInt(0, 10));
        }
        if (strLst.at(0) == "calcCRCFromPosition") {
            pt.calcCRCFromPosition = (strLst.at(1).toInt(0, 10));
        }
        if (strLst.at(0) == "markerPacketBeginSize") {
            pt.markerPacketBeginSize = (strLst.at(1).toInt(0, 10));
        }
        if (strLst.at(0) == "markerPacketBeginTextB1") {
            pt.markerPacketBeginByte1 = (strLst.at(1).toInt(0, 16));
        }
        if (strLst.at(0) == "markerPacketBeginTextB2") {
            pt.markerPacketBeginByte2 = (strLst.at(1).toInt(0, 16));
        }
        if (strLst.at(0) == "timeoutAfterLastByte") {
            pt.timeoutAfterLastByte = (strLst.at(1).toInt(0, 10));
        }
        if (strLst.at(0) == "description") {
            pt.description = str;
        }
        if (strLst.at(0) == "varControl") {
            pt.varControl = (strLst.at(1) == "true") ? true : false;
        }
        strLst.clear();
    }
    emit loadProtocol(pt);
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

QDateTime newconnect::returnTimestamp()
{
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    QDateTime dt3 = QDateTime::fromMSecsSinceEpoch(timestamp);
    return dt3;
}

QString newconnect::getProfileNameFromInfo(QFileInfo& info)
{
    return (info.fileName());
}

quint8 newconnect::calcCrc(const QVector<quint8> &arr)
{
    quint8 crc = 0;
    for (int i = 2; i < arr.size() - 1; i++) {
        crc += arr[i];
    }
    return crc;
}

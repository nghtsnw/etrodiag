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
    connect (this, &newconnect::setPacketSize, m_settings, &SettingsDialog::setPacketSizeSpinBox);
    connect (this, &newconnect::setBlockIdentifycatorPosition, m_settings, &SettingsDialog::setBlockIdentifycatorPositionSpinBox);
    connect (this, &newconnect::setCalcCRCFromPosition, m_settings, &SettingsDialog::setCalcCRCFromPositionSpinBox);
    connect (this, &newconnect::setMarkerPacketBeginSize, m_settings, &SettingsDialog::setMarkerPacketBeginSizeSpinBox);
    connect (this, &newconnect::setMarkerPacketBeginText, m_settings, &SettingsDialog::setMarkerPacketBeginText);
    connect (this, &newconnect::setTimeoutAfterLastByte, m_settings, &SettingsDialog::setTimeoutAfterLastByteSpinBox);
    /*----------------------------------------------------------------------------------------------------------------------------*/
    /*----------------------------------------------------------------------------------------------------------------------------*/
    //По изменению настроек в UI, они сразу применяются на датаразборке
    connect (m_settings, &SettingsDialog::packetSizeSpinBox_valueChanged, datapool, &dataprofiler::setPacketSize);
    connect (m_settings, &SettingsDialog::blockIdentifycatorPositionSpinBox_valueChanged, datapool, &dataprofiler::setBlockIdentifycatorPosition);
    connect (m_settings, &SettingsDialog::calcCRCFromSpinBox_valueChanged, datapool, &dataprofiler::setCalcCRCFromPosition);
    connect (m_settings, &SettingsDialog::markerSizeSpinBox_valueChanged, datapool, &dataprofiler::setMarkerPacketBeginSize);
    connect (m_settings, &SettingsDialog::varConrolCheckBox_valueChanged, this, &newconnect::setVisibleControlWindow);
    connect (m_settings, &SettingsDialog::markerBeginText_valueChanged, datapool, &dataprofiler::setMarkerPacketBeginText);
    /*----------------------------------------------------------------------------------------------------------------------------*/
    /*----------------------------------------------------------------------------------------------------------------------------*/
    //При загрузке данных из профиля, они сразу применяются на датаразборке (возможно лишние связи)
    connect (this, &newconnect::setPacketSize, datapool, &dataprofiler::setPacketSize);
    connect (this, &newconnect::setBlockIdentifycatorPosition, datapool, &dataprofiler::setBlockIdentifycatorPosition);
    connect (this, &newconnect::setCalcCRCFromPosition, datapool, &dataprofiler::setCalcCRCFromPosition);
    connect (this, &newconnect::setMarkerPacketBeginSize, datapool, &dataprofiler::setMarkerPacketBeginSize);
    connect (this, &newconnect::setMarkerPacketBeginText, datapool, &dataprofiler::setMarkerPacketBeginText);
    connect (this, &newconnect::setTimeoutAfterLastByte, datapool, &dataprofiler::setTimeoutAfterLastByte);
    /*----------------------------------------------------------------------------------------------------------------------------*/
    /*----------------------------------------------------------------------------------------------------------------------------*/
    //Запрос текущих параметров протокола из датаразборки для сохранения в файле профиля
    connect (this, &newconnect::getPacketSize, datapool, &dataprofiler::s_returnPacketSize);
    connect (this, &newconnect::getBlockIdentifycatorPosition, datapool, &dataprofiler::s_returnBlockIdentifycatorPosition);
    connect (this, &newconnect::getCalcCRCFromPosition, datapool, &dataprofiler::s_returnCalcCRCFromPosition);
    connect (this, &newconnect::getMarkerPacketBeginSize, datapool, &dataprofiler::s_returnMarkerPacketBeginSize);
    connect (this, &newconnect::getMarkerPacketBeginText, datapool, &dataprofiler::s_returnMarkerPacketBeginText);
    connect (this, &newconnect::getTimeoutAfterLastByte, datapool, &dataprofiler::s_returnTimeoutAfterLastByte);
    /*----------------------------------------------------------------------------------------------------------------------------*/
    connect (m_settings, &SettingsDialog::loadSelectedProfile, this, &newconnect::readProfile);
    connect (timer, &QTimer::timeout, this, &newconnect::readFromFile);//читаем из файла по таймеру
    connect (timerAboveTxCommand, &QTimer::timeout, this, &newconnect::sendCommand);//отправляем команду после задержки
    connect (this, &newconnect::sendRawData, gstream, &getStream::getRawData);
    connect (this, &newconnect::sendRawData, m_console, &Console::putData);
    /*----------------------------------------------------------------------------------------------------------------------------*/
    //Возврат текущих параметров из датаразборки на сохранение в файл профиля
    connect (datapool, &dataprofiler::returnPacketSize, this, [ = ](int size) {
        toSavePacketSize = size;
    });
    connect (datapool, &dataprofiler::returnBlockIdentifycatorPosition, this, [ = ](int pos) {
        toSaveBlockIdentifycatorPosition = pos;
    });
    connect (datapool, &dataprofiler::returnCalcCRCFromPosition, this, [ = ](int pos) {
        toSaveCalcCRCFromPosition = pos;
    });
    connect (datapool, &dataprofiler::returnMarkerPacketBeginSize, this, [ = ](int size) {
        toSaveMarkerPacketBeginSize = size;
    });
    connect (datapool, &dataprofiler::returnMarkerPacketBeginText, this, [ = ](QString text) {
        toSaveMarkerPacketBeginText = text;
    });
    connect (datapool, &dataprofiler::returnTimeoutAfterLastByte, this, [ = ](int timeout_ms) {
        toSaveTimeoutAfterLastByte = timeout_ms;
    });
    /*----------------------------------------------------------------------------------------------------------------------------*/
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
        emit getPacketSize();
        emit getBlockIdentifycatorPosition();
        emit getCalcCRCFromPosition();
        emit getMarkerPacketBeginSize();
        emit getMarkerPacketBeginText();
        emit getTimeoutAfterLastByte();
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
        txtStream << "packetSize" << "\t" << QString::number(toSavePacketSize, 10) << "\n";
        txtStream << "blockIdentifycatorPosition" << "\t" << QString::number(toSaveBlockIdentifycatorPosition, 10) << "\n";
        txtStream << "calcCRCFromPosition" << "\t" << QString::number(toSaveCalcCRCFromPosition, 10) << "\n";
        txtStream << "markerPacketBeginSize" << "\t" << QString::number(toSaveMarkerPacketBeginSize, 10) << "\n";
        txtStream << "markerPacketBeginText" << "\t" << toSaveMarkerPacketBeginText << "\n";
        txtStream << "timeoutAfterLastByte" << "\t" << QString::number(toSaveTimeoutAfterLastByte, 10) << "\n";
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
            emit setPacketSize(strLst.at(1).toInt(0, 10));
        }
        if (strLst.at(0) == "blockIdentifycatorPosition") {
            emit setBlockIdentifycatorPosition(strLst.at(1).toInt(0, 10));
        }
        if (strLst.at(0) == "calcCRCFromPosition") {
            emit setCalcCRCFromPosition(strLst.at(1).toInt(0, 10));
        }
        if (strLst.at(0) == "markerPacketBeginSize") {
            emit setMarkerPacketBeginSize(strLst.at(1).toInt(0, 10));
        }
        if (strLst.at(0) == "markerPacketBeginText") {
            emit setMarkerPacketBeginText(strLst.at(1));
        }
        if (strLst.at(0) == "timeoutAfterLastByte") {
            emit setTimeoutAfterLastByte(strLst.at(1).toInt(0, 10));
        }
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

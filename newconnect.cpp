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

    m_console->setParent(ui->label);

    m_console->show();

    connect(m_serial, &QSerialPort::errorOccurred, this, &newconnect::handleError);
    connect(m_serial, &QSerialPort::readyRead, this, &newconnect::readData);
    connect(m_console, &Console::getData, this, &newconnect::writeData);
    connect(gstream, &getStream::giveMyByte, datapool, &dataprofiler::getByte);
    connect(datapool, SIGNAL(readyGetByte()), gstream, SLOT(profilerReadyToReceive()));
    connect(datapool, &dataprofiler::deviceData, this, &newconnect::transData);
}

newconnect::~newconnect()
{
    delete m_serial;
    delete ui;
}

void newconnect::on_pushButton_clicked()
{
    m_settings->show();
}

void newconnect::openSerialPort()
{
    readProfile();
    const SettingsDialog::Settings p = m_settings->settings();
    m_serial->setPortName(p.name);
    m_serial->setBaudRate(p.baudRate);
    m_serial->setDataBits(p.dataBits);
    m_serial->setParity(p.parity);
    m_serial->setStopBits(p.stopBits);
    m_serial->setFlowControl(p.flowControl);
    if (m_serial->open(QIODevice::ReadWrite)) {
        m_console->setEnabled(true);
        m_console->setLocalEchoEnabled(p.localEchoEnabled);        
        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
    } else {
        QMessageBox::critical(this, tr("Error"), m_serial->errorString());
        showStatusMessage(tr("Open error"));

  }
}


void newconnect::closeSerialPort()
{
    if (m_serial->isOpen())
        m_serial->close();

    showStatusMessage(tr("Disconnected"));
}

void newconnect::writeData(const QByteArray &data)
{
    m_serial->write(data);
}

void newconnect::readData()
{
    const QByteArray data = m_serial->readAll();
    m_console->putData(data);
    gstream->getRawData(data);


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

void newconnect::on_pushButton_2_clicked()
{
    if (!(m_serial->isOpen()))
    {
        qDebug() << "mserial is open: " << (m_serial->isOpen());
        openSerialPort();
        if (m_serial->isOpen())
        {
            ui->pushButton_2->setText("Disconnect");
            qDebug() << "port open";
        }
    }
    else if (m_serial->isOpen())
    {
        this->closeSerialPort();
        if (!(m_serial->isOpen()))
        ui->pushButton_2->setText("Connect");
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
        emit saveAllMasks();
    }
}

void newconnect::saveProfileSlot4Masks(int devNum, QString devName, int byteNum, QString byteName, int id, QString paramName, QString paramMask, int paramType, double valueShift, double valueKoef, bool viewInLogFlag, int wordType)
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
    profile.open(QIODevice::WriteOnly|QIODevice::Text);
    while (maskVectorsListIt.hasNext())
    {
        QTextStream txtStream(&profile);
        txtStream << info.fileName() << "\n";

        QListIterator<QString> lstIt(maskVectorsListIt.peekNext()->lst);
        while (lstIt.hasNext())
        txtStream << lstIt.next() << "\t";
        txtStream << "\n";
        maskVectorsListIt.next();
    }
    permission2SaveMasks = false;
}
}

void newconnect::readProfile()
{
    const SettingsDialog::Settings p = m_settings->settings();
    QFile profile(p.profilePath);
    profile.open(QIODevice::ReadOnly|QIODevice::Text);
    QTextStream txtStream(&profile);
    while (!txtStream.atEnd())
    {
        QString str = txtStream.readLine();
        QStringList strLst = str.split('\t');
        if (strLst.at(0)=="thisIsMask")
            emit loadMask(strLst.at(2).toInt(0,10),strLst.at(4),strLst.at(3).toInt(0,10),strLst.at(5),strLst.at(1).toInt(0,10),strLst.at(6),strLst.at(7),0,strLst.at(8).toDouble(),strLst.at(9).toDouble(),((QString::compare(strLst.at(10), "true") == 0) ? true : false),strLst.at(11).toInt(0,10));
        strLst.clear();
    }
}

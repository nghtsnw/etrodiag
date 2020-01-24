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
        //getStream *gstream = new getStream(this); //объект обработчика входящих данных
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

//void newconnect::sendstatus()
//{
//    emit sendStatusStr(*m_status);
//}

void newconnect::showStatusMessage(QString message)
{
    m_status->setText(message);
    emit sendStatusStr(message);
    //w.showStatusMessage(*m_status);
    //connect (this, SIGNAL(sendStatusStr()), MainWindow::showStatusMessage(message))
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
    maskVectorsList = this->findChildren<txtmaskobj*>();
    QListIterator<txtmaskobj*> maskVectorsListIt(maskVectorsList);
    maskVectorsListIt.toFront();
    while (maskVectorsListIt.hasNext())
           maskVectorsListIt.next()->~txtmaskobj();
    permission2SaveMasks = true;
    emit saveAllMasks();
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
                   //maskList.clear();
               }
           }
           }

void newconnect::saveProfile()
{
    maskVectorsList = this->findChildren<txtmaskobj*>();
    QListIterator<txtmaskobj*> maskVectorsListIt(maskVectorsList);
    maskVectorsListIt.toFront();
    const SettingsDialog::Settings p = m_settings->settings();
    QFile profile(p.profilePath);
    QFileInfo info(profile);
    profile.open(QIODevice::WriteOnly);
    QXmlStreamWriter profileWriter(&profile);
    profileWriter.setAutoFormatting(true);
    profileWriter.writeStartDocument();
    profileWriter.writeStartElement("profile");
    profileWriter.writeAttribute("profilename", info.fileName());

    while (maskVectorsListIt.hasNext())
    {
        profileWriter.writeStartElement("devices");
        profileWriter.writeStartElement("devnumber", maskVectorsListIt.peekNext()->lst.at(2));
        profileWriter.writeTextElement("devname", maskVectorsListIt.peekNext()->lst.at(4));
        profileWriter.writeStartElement("bytes");
        profileWriter.writeStartElement("bytenum", maskVectorsListIt.peekNext()->lst.at(3));
        profileWriter.writeTextElement("bytename", maskVectorsListIt.peekNext()->lst.at(5));
        profileWriter.writeTextElement("wordlenght", maskVectorsListIt.peekNext()->lst.at(11));
        profileWriter.writeStartElement("masks");
        profileWriter.writeTextElement("mask", maskVectorsListIt.peekNext()->lst.at(7));
        profileWriter.writeTextElement("maskname", maskVectorsListIt.peekNext()->lst.at(6));
        profileWriter.writeTextElement("valueshift", maskVectorsListIt.peekNext()->lst.at(8));
        profileWriter.writeTextElement("valuekoef", maskVectorsListIt.peekNext()->lst.at(9));
        profileWriter.writeTextElement("viewinlog", maskVectorsListIt.peekNext()->lst.at(10));
        profileWriter.writeEndElement();
        profileWriter.writeEndElement();
        profileWriter.writeEndElement();
        profileWriter.writeEndElement();
        profileWriter.writeEndElement();
        profileWriter.writeEndElement();
        maskVectorsListIt.next();
    }

    profileWriter.writeEndElement();
    profileWriter.writeEndDocument();
    profile.close();
    permission2SaveMasks = false;
    //emit saveAllMasks();
}



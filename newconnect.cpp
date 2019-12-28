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

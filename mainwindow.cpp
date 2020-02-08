/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "console.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include "newconnect.h"
#include <QtWidgets>
#include <QDebug>
#include <QPushButton>
#include "dataprofiler.h"
#include <QList>
#include "device.h"
#include "devsettingsform.h"
#include "bytesettingsform.h"
#include "bytebutton.h"
#include "treemodel.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), statuslbl (new QLabel), m_ui (new Ui::MainWindow)

{
    m_ui->setupUi(this);

    //QLabel *statuslbl = new QLabel(this);
    statusBar()->addWidget(statuslbl);
    statuslbl->setText("Make Electroagregat great again!");
    addConnection();
    connect (connection, &newconnect::loadMask, this, &MainWindow::loadProfile);
    connect (&btsf, &ByteSettingsForm::editMask, &masksd, &maskSettingsDialog::requestDataOnId);
//    connect (this, &MainWindow::resizeEvent, this, &MainWindow::resizeEventSlot);
}


MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::addConnection()
{
    connection = new newconnect(m_ui->tab_connections);
    connect (connection, SIGNAL(sendStatusStr(QString)), SLOT(showStatusMessage(QString)));
    connect (connection, &newconnect::transmitData, this, &MainWindow::addDeviceToList);
    connection->show();
}

void MainWindow::showStatusMessage(QString message)
{
    statuslbl->setText(message);
}


void MainWindow::addDeviceToList(QVector<int> ddata)
{
    //static QVBoxLayout *vlay = new QVBoxLayout;
    //m_ui->devArea->setLayout(vlay); //больше некуда воткнуть это, пусть пока побудет здесь
    int devNum = ddata.at(2);//узнаём номер устройства в посылке
    QString devNumHex = QString("%1").arg(devNum,0,16).toUpper();//конвертируем его в хекс-вид
    bool thisDeviceHere = false; //обнуляем флаг

    QList<Device*> vlayChildList = m_ui->devArea->findChildren<Device*>();
    QListIterator<Device*> vlayChildListIt(vlayChildList); //смотрим сколько в гуе отображается устройств, создаём перечислитель
    //qDebug() << "Children of devArea = " << vlayChildList.size();

    while (vlayChildListIt.hasNext())
    {
        if (devNum == vlayChildListIt.next()->devNum) //смотрим, есть ли наше устройство в текущем листе
        {
           thisDeviceHere = true; //если есть, ставим флаг что оно тут
           emit devUpdate(devNum, ddata); //если есть то пихаем ему обновление через сигнал
           dvsf.updByteButtons(devNum, ddata); //обновление кнопок в форме настройки
        }
    }
    if (!thisDeviceHere) //если устройства нет, то создаём его
    {
        createDevice(devNum);
        emit devUpdate(devNum, ddata);
        dvsf.updByteButtons(devNum, ddata);
    }

    vlayChildListIt.toFront();
    m_ui->devArea->update();
}

void MainWindow::createDevice(int devNum)
{
    Device *dev = new Device(devNum);
    dev->setParent(m_ui->devArea);
    m_ui->devAreaLay->addWidget(dev);
    dev->setText(QString::number(devNum,16));
    dev->show();
    connect (this, &MainWindow::devUpdate, dev, &Device::updateData);
    connect (dev, &Device::txtToGui, this, &MainWindow::txtToGuiFunc);
    connect (dev, &Device::openDevSettSig, this, &MainWindow::openDevSett);
    connect (dev, &Device::clicked, dev, &Device::clickedF);
    connect (this, &MainWindow::getDevName, dev, &Device::getDeviceName);
    connect (&dvsf, &devSettingsForm::returnDevNameAfterEdit, dev, &Device::setDeviceName);
    connect (dev, &Device::returnDeviceName, &dvsf, &devSettingsForm::setDevName);
    connect (&dvsf, &devSettingsForm::openByteSettingsFormTX, this, &MainWindow::openByteSett);
    connect (&btsf, &ByteSettingsForm::setWordBit, dev, &Device::setWordTypeInByteProfile);
    connect (&btsf, &ByteSettingsForm::setWordBit, &dvsf, &devSettingsForm::wordTypeChangeRX);//изменить
    connect (&btsf, &ByteSettingsForm::getWordType, dev, &Device::getWordTypeFromProfileRetranslator);
    connect (&dvsf, &devSettingsForm::initByteButtonsWordLeight, dev, &Device::getWordTypeFromProfileRetranslator);
    connect (dev, &Device::returnWordTypeTX, &btsf, &ByteSettingsForm::returnWordType);
    connect (dev, &Device::returnWordTypeTX, &dvsf, &devSettingsForm::wordTypeChangeRX);
    connect (&btsf, &ByteSettingsForm::createMask, dev, &Device::createNewMaskRX);
    connect (dev, &Device::mask2FormTX, &masksd, &maskSettingsDialog::requestDataOnId);
    connect (&masksd, &maskSettingsDialog::requestMaskData, dev, &Device::requestMaskDataRX);
    connect (&btsf, &ByteSettingsForm::requestAllMaskToList, dev, &Device::requestMaskDataRX);
    connect (dev, &Device::maskData2FormTX, &masksd, &maskSettingsDialog::getDataOnId);
    connect (dev, &Device::allMasksToListTX, &btsf, &ByteSettingsForm::addMaskItem);
    connect (&masksd, &maskSettingsDialog::sendMaskData, &btsf, &ByteSettingsForm::addMaskItem);
    connect (&masksd, &maskSettingsDialog::requestMaskData, this, &MainWindow::openMaskSettingsDialog);
    connect (&masksd, &maskSettingsDialog::sendMaskData, dev, &Device::sendDataToProfileRX);
    connect (&btsf, &ByteSettingsForm::deleteMaskObj, dev, &Device::deleteMaskObjTX);
    connect (&dvsf, &devSettingsForm::wordDataFullHex, &btsf, &ByteSettingsForm::updateHexWordData);
    connect (dev, &Device::param2FrontEndTX, this, &MainWindow::frontendDataSort);
    connect (dev, &Device::param2FrontEndTX, &masksd, &maskSettingsDialog::liveDataSlot);
    connect (dev, &Device::param2FrontEndTX, &btsf, &ByteSettingsForm::updateMasksList);
    connect (connection, &newconnect::saveAllMasks, dev, &Device::requestMasks4Saving);
    connect (dev, &Device::allMasksToListTX, connection, &newconnect::saveProfileSlot4Masks);
    connect (this, &MainWindow::sendMaskData, dev, &Device::loadMaskRX);
    connect (this, &MainWindow::getByteName, dev, &Device::getByteNameRX);
    connect (dev, &Device::returnByteNameTX, &btsf, &ByteSettingsForm::setWordName);
    connect (&btsf, &ByteSettingsForm::saveByteName, dev, &Device::saveByteNameRX);

}

void MainWindow::txtToGuiFunc(QString txtToGui)
{
    m_ui->logArea->insertPlainText(txtToGui);
}

void MainWindow::openDevSett(int devNum, QVector<int> data)
{//все реакции на нажатие кнопки устройства в зависимости от состояния окна
    if (masksd.isVisible())
    {            
            masksd.sendMask2Profile();
            masksd.hide();
            masksd.killChildren();
            btsf.show();
            btsf.resize(m_ui->rightFrame->size());
    }
    else {
    if (btsf.isVisible())
    {
        btsf.hide();
        btsf.cleanMaskList();
        dvsf.show();
        dvsf.resize(m_ui->rightFrame->size());
    }
    else
    {
    dvsf.setParent(m_ui->rightFrame);
    if (m_ui->logArea->isHidden())
    {
        m_ui->logArea->show();
        m_ui->valueArea->show();
        m_ui->writeLogCheckBox->show();
        connection->prepareToSaveProfile();
        connection->saveProfile();
        dvsf.hide();
        dvsf.killChildren();
    }
    else
    {
        m_ui->logArea->hide();
        m_ui->valueArea->hide();
        m_ui->writeLogCheckBox->hide();
        dvsf.initByteButtons(devNum,data);
        dvsf.show();
        dvsf.resize(m_ui->rightFrame->size());
        emit getDevName(devNum);
    }
    }
    }
}

void MainWindow::openByteSett(int devNum, int byteNum)
{
    if (dvsf.isVisible())
    {
        btsf.setParent(m_ui->rightFrame);
        dvsf.hide();
        btsf.open(devNum, byteNum);
        btsf.show();
        emit getByteName(devNum, byteNum);
    }
}

void MainWindow::openMaskSettingsDialog()
{
    if (btsf.isVisible())
    {
        btsf.hide();
        masksd.setParent(m_ui->rightFrame);
        masksd.show();
        masksd.resize(m_ui->rightFrame->size());
    }
}

void MainWindow::frontendDataSort(int devNum, QString devName, int byteNum, QString byteName, int wordData, int id, QString parameterName, int binRawValue, double endValue, bool viewInLogFlag)
{    
    if (viewInLogFlag)
    {
        quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
        QDateTime dt3 = QDateTime::fromMSecsSinceEpoch(timestamp);
        QString formString(dt3.toString("hh:mm:ss:zzz") + " " + parameterName + "@" + devName + ": " + QString::number(endValue,'g',6));
        m_ui->logArea->appendPlainText(formString);


    QDir dir("Logs");
    if (!dir.exists())
        QDir().mkdir("Logs");
    QFile newLogFile;

    if (m_ui->writeLogCheckBox->isChecked())
    {
        if (!newLogFile.isOpen())
        {
            if (createNewFileNamePermission)
            {
                logFileName = (dir.path() + dt3.toString("\\dd.MM.yy_hh-mm-ss") + ".log");
                createNewFileNamePermission = false;
            }
            newLogFile.setFileName(logFileName);
            if (!newLogFile.exists())
            newLogFile.open(QIODevice::WriteOnly|QIODevice::Text);
            newLogFile.open(QIODevice::Append|QIODevice::Text);
            showStatusMessage("Start write log file " + newLogFile.fileName());
        }
        if (newLogFile.isOpen())
        {
            QTextStream logStream(&newLogFile);
            logStream << formString << '\n';            
            newLogFile.close();
        }
        else showStatusMessage("Error write log");
    }
    else if (!m_ui->writeLogCheckBox->isChecked() && !createNewFileNamePermission)
    {
            newLogFile.close();
            createNewFileNamePermission = true;
            showStatusMessage("Stop write log file " + newLogFile.fileName());
    }
}
}

void MainWindow::loadProfile(int devNum, QString devName, int byteNum, QString byteName, int id, QString paramName, QString paramMask, int paramType, double valueShift, double valueKoef, bool viewInLogFlag, int wordType)
{//если устройства нет, то создаём, потом посылаем маску
    bool thisDeviceHere = false;
    QList<Device*> vlayChildList = m_ui->devArea->findChildren<Device*>();
    QListIterator<Device*> vlayChildListIt(vlayChildList);
    QString dev2mod;
    while (vlayChildListIt.hasNext())
        if (devNum == vlayChildListIt.next()->devNum)
            thisDeviceHere = true;
    if (thisDeviceHere)
        emit sendMaskData(devNum, devName, byteNum, byteName, id, paramName, paramMask, paramType, valueShift, valueKoef, viewInLogFlag, wordType);
    else if (!thisDeviceHere)
    {//создаём устройство и инициализируем пустым пакетом в 40 байт, с номером устройства на позиции 2
        createDevice(devNum);
        QVector<int> devInitArray(40,0);
        devInitArray.replace(2, devNum);
        emit devUpdate(devNum, devInitArray);
        dvsf.updByteButtons(devNum, devInitArray);
        emit sendMaskData(devNum, devName, byteNum, byteName, id, paramName, paramMask, paramType, valueShift, valueKoef, viewInLogFlag, wordType);
    }

}

void MainWindow::setLogFlag(bool _logFlag)
{
    logFlag = _logFlag;
}


void MainWindow::resizeEvent(QResizeEvent* e)
{
    dvsf.resize(m_ui->rightFrame->size());
    btsf.resize(m_ui->rightFrame->size());
    masksd.resize(m_ui->rightFrame->size());
}

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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), statuslbl (new QLabel), m_ui (new Ui::MainWindow)

{
    m_ui->setupUi(this);
    statusBar()->addWidget(statuslbl);
    statuslbl->setText("Make Electroagregat great again!");
    addConnection();    
    connect (&btsf, &ByteSettingsForm::editMask, &masksd, &maskSettingsDialog::requestDataOnId);
    connect (this, &MainWindow::dvsfAfterCloseClear, &dvsf, &devSettingsForm::afterCloseClearing);
    m_ui->tab_connections->show();
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::addConnection()
{
    connection = new newconnect;
    m_ui->horizontalLayout_3->addWidget(connection);

    connect (connection, &newconnect::loadMask, this, &MainWindow::loadProfile);
    connect (connection, &newconnect::sendStatusStr, this, &MainWindow::showStatusMessage);
    connect (connection, &newconnect::transmitData, this, &MainWindow::addDeviceToList);
    connect (connection, &newconnect::cleanDevListSig, this, &MainWindow::cleanDevList);
    connect (this, &MainWindow::prepareToSaveProfile, connection, &newconnect::prepareToSaveProfile);
    connect (this, &MainWindow::saveProfile, connection, &newconnect::saveProfile);
    connection->show();

}

void MainWindow::showStatusMessage(QString message)
{
    statuslbl->setText(message);
    logFileCreator(message, true);
}

void MainWindow::addDeviceToList(QVector<int> ddata)
{
    int devNum = ddata.at(2);//узнаём номер устройства в посылке
    QString devNumHex = QString("%1").arg(devNum,0,16).toUpper();//конвертируем его в хекс-вид
    bool thisDeviceHere = false; //обнуляем флаг

    QList<Device*> vlayChildList = m_ui->devArea->findChildren<Device*>();
    QListIterator<Device*> vlayChildListIt(vlayChildList); //смотрим сколько в гуе отображается устройств, создаём перечислитель

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
    connect (this, &MainWindow::devUpdate, dev, &Device::updateData);
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
    connect (dev, &Device::param2FrontEndTX, &dvsf, &devSettingsForm::liveDataSlot);    
    connect (this, &MainWindow::sendMaskData, dev, &Device::loadMaskRX);
    connect (this, &MainWindow::getByteName, dev, &Device::getByteNameRX);
    connect (dev, &Device::returnByteNameTX, &btsf, &ByteSettingsForm::setWordName);
    connect (&btsf, &ByteSettingsForm::saveByteName, dev, &Device::saveByteNameRX);
    connect (this, &MainWindow::hideOtherDevButtons, dev, &Device::hideDevButton);
    connect (dev, &Device::devStatusMessage, this, &MainWindow::devStatusMsg);
    connect (connection, &newconnect::saveAllMasks, dev, &Device::requestMasks4Saving);
    connect (dev, &Device::allMasksToListTX, connection, &newconnect::saveProfileSlot4Masks);

    dev->show();
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
        btsf.cleanForm();
        dvsf.show();
        dvsf.resize(m_ui->rightFrame->size());
    }
    else
    {
    dvsf.setParent(m_ui->rightFrame);
    if (m_ui->logArea->isHidden())
    {
        dvsf.hide();
        emit dvsfAfterCloseClear();
        m_ui->logArea->show();
        m_ui->valueArea->clear();
        m_ui->valueArea->setRowCount(0);
        m_ui->valueArea->show();
        m_ui->writeLogCheckBox->show();
        emit hideOtherDevButtons(false, devNum);
        emit prepareToSaveProfile();
        emit saveProfile();
    }
    else
    {
        m_ui->logArea->hide();
        m_ui->valueArea->hide();
        m_ui->writeLogCheckBox->hide();        
        emit hideOtherDevButtons(true, devNum);        
        dvsf.initByteButtons(devNum,data);
        emit getDevName(devNum);
        dvsf.show();        
        dvsf.resize(m_ui->rightFrame->size());        
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
        btsf.cleanForm();
        btsf.open(devNum, byteNum);
        btsf.resize(m_ui->rightFrame->size());
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

QDateTime MainWindow::returnTimestamp()
{
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    QDateTime dt3 = QDateTime::fromMSecsSinceEpoch(timestamp);
    return dt3;
}

void MainWindow::updValueArea(QString parameterName, QString devName, double endValue, bool isNewData)
{
    {
        bool findRow = false;
        QString value2str;
        QString namesUnited = (parameterName+'@'+devName);
        value2str.setNum(endValue, 'g', 6);
        if (m_ui->valueArea->rowCount() > 0)
        {
            for (int i = 0; i < m_ui->valueArea->rowCount(); i++)
            {
                if ((namesUnited) == m_ui->valueArea->item(i,0)->text())
                {
                    findRow = true;
                    if (value2str != m_ui->valueArea->item(i,1)->text())
                    {
                        m_ui->valueArea->item(i,1)->setText(value2str);
                        m_ui->valueArea->item(i,1)->setBackgroundColor(Qt::green);
                       //метод setbackgroundcolor устаревший, по возможности переписать на делегаты
                    }
                    else if (value2str == m_ui->valueArea->item(i,1)->text())
                        m_ui->valueArea->item(i,1)->setBackgroundColor(Qt::white);
                }
            }
        }
        if (!findRow)
        {
            m_ui->valueArea->setRowCount(m_ui->valueArea->rowCount()+1); //добавляем новую строку
            int row = m_ui->valueArea->rowCount()-1;//определяем индекс строки
            QTableWidgetItem *nameItem = new QTableWidgetItem;
            nameItem->setText(parameterName+'@'+devName);
            m_ui->valueArea->setItem(row, 0, nameItem);
            QTableWidgetItem *valueItem = new QTableWidgetItem;
            valueItem->setText(value2str);
            m_ui->valueArea->setItem(row, 1, valueItem);
        }
    }
}

void MainWindow::frontendDataSort(int devNum, QString devName, int byteNum, QString byteName, int wordData, int id, QString parameterName, int binRawValue, double endValue, bool viewInLogFlag, bool isNewData)
{    
    if (dvsf.isVisible() && devNum == dvsf.devNum)
        dvsf.setDevName(devNum, devName);

    if (viewInLogFlag && isNewData)
    {        
        QString formString(parameterName + "@" + devName + ": " + QString::number(endValue,'g',6));
        logFileCreator(formString, false);
    }
    updValueArea(parameterName, devName, endValue, isNewData);
}

void MainWindow::logFileCreator(QString string, bool redFlag)
{
    QString stringWithTime = (returnTimestamp().toString("hh:mm:ss:zzz") + " " + string);
    if (!redFlag)
    m_ui->logArea->appendPlainText(stringWithTime);
    else m_ui->logArea->appendHtml("<p><span style=color:#ff0000>" + stringWithTime + "</span></p>");

    QFile newLogFile;
    if (m_ui->writeLogCheckBox->isChecked())
    {
        QDir dir("Logs");
        if (!dir.exists())
        QDir().mkdir("Logs");

        if (!newLogFile.isOpen())
        {
            if (createNewFileNamePermission)
            {
                logFileName = (dir.path() + returnTimestamp().toString("\\dd.MM.yy_hh-mm-ss") + ".log");
                createNewFileNamePermission = false;
            }
            newLogFile.setFileName(logFileName);
            if (!newLogFile.exists())
            {
                newLogFile.open(QIODevice::WriteOnly|QIODevice::Text);                
                m_ui->logArea->appendHtml("<p><span style=color:#ff0000>" + returnTimestamp().toString("hh:mm:ss:zzz") + " "
                                          + QString("Start write log file ") + newLogFile.fileName() + "</span></p>");
            }
            else newLogFile.open(QIODevice::Append|QIODevice::Text);
        }
        if (newLogFile.isOpen())
        {
            QTextStream logStream(&newLogFile);
            logStream << stringWithTime << '\n';
            newLogFile.close();
        }
        else showStatusMessage("Error write log");
    }
    else if (!m_ui->writeLogCheckBox->isChecked() && !createNewFileNamePermission)
    {
            m_ui->logArea->appendHtml("<p><span style=color:#ff0000>" + returnTimestamp().toString("hh:mm:ss:zzz") + " "
                                  + QString("Stop write log file") + "</span></p>");
            newLogFile.close();
            createNewFileNamePermission = true;            
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

void MainWindow::devStatusMsg(QString _devName, QString status)
{
    logFileCreator("Device " + _devName + " is " + status, true);
}

void MainWindow::resizeEvent(QResizeEvent* e)
{
    dvsf.resize(m_ui->rightFrame->size());
    btsf.resize(m_ui->rightFrame->size());
    masksd.resize(m_ui->rightFrame->size());
}

void MainWindow::cleanDevList()
{
    QList<Device*> vlayChildList = m_ui->devArea->findChildren<Device*>();
    QListIterator<Device*> vlayChildListIt(vlayChildList);
    while(vlayChildListIt.hasNext())
        vlayChildListIt.next()->~Device();
}

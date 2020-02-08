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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QHBoxLayout>
#include <QPushButton>
#include "devsettingsform.h"
#include "bytesettingsform.h"
#include "device.h"
#include "masksettingsdialog.h"
#include "treemodel.h"
//#include "newconnect.h"

QT_BEGIN_NAMESPACE

class QLabel;


namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

class Console;
class SettingsDialog;
class newconnect;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = nullptr);
    void addConnection();
    newconnect *connection = nullptr;
    //devSettingsForm *dvsf = nullptr;
    QLabel *statuslbl = nullptr;
    //Device& getLinkOnDev(int devNum);
    void openMaskSettingsDialog();
    void createDevice(int devNum);
    void loadProfile(int devNum, QString devName, int byteNum, QString byteName, int id, QString paramName, QString paramMask, int paramType, double valueShift, double valueKoef, bool viewInLogFlag, int wordType);
    bool logFlag = false;
    ~MainWindow();


signals:
    void devUpdate(int devNum, QVector<int> ddata);
    void getDevName(int devNum);
    void setDevName(int devNum, QString name);
    void returnDevNameAfterClose(int devNum, QString text);
    void sendMaskData(int devNum, QString devName, int byteNum, QString byteName, int id, QString paramName, QString paramMask, int paramType, double valueShift, double valueKoef, bool viewInLogFlag, int wordType);
    void getByteName(int devNum, int byteNum);

public slots:

    void showStatusMessage(QString message);
    void addDeviceToList(QVector<int> ddata);
    void txtToGuiFunc(QString txtToGui);
    void openDevSett(int devNum, QVector<int> data);
    void openByteSett(int devNum, int byteNum);
    void frontendDataSort(int devNum, QString devName, int byteNum, QString byteName, int wordData, int id, QString parameterName, int binRawValue, double endValue, bool viewInLogFlag);
    void setLogFlag(bool _logFlag);

private slots:
    //void handleError(QSerialPort::SerialPortError error);


private:
    void initActionsConnections();

private:

    QLabel *m_status = nullptr;
protected:
    Ui::MainWindow *m_ui = nullptr;

public:
    devSettingsForm dvsf;//(m_ui->rightFrame);
    ByteSettingsForm btsf;
    maskSettingsDialog masksd;

};

#endif // MAINWINDOW_H

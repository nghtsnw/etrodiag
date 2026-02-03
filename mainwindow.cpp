#include "mainwindow.h"
#include "aboutdialog.h"
#include "controlboard.h"
#include "ui_mainwindow.h"
#include "newconnect.h"
#include <QtWidgets>
#include <QDebug>
#include <QPushButton>
#include <QList>
#include "device.h"
#include "devsettingsform.h"
#include "bytesettingsform.h"
#include <QGestureEvent>
#include <QSwipeGesture>
#include <QMap>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), statuslbl (new QLabel), crcerrorlbl (new QLabel), aboutButton (new QPushButton), m_ui (new Ui::MainWindow)

{
// QVector<Qt::GestureType> gestures;
// gestures << Qt::SwipeGesture << Qt::TapGesture;
// for (Qt::GestureType gesture : gestures)
// grabGesture(gesture);
// Надеюсь, что когда в qt починят qswipegesture, я раскомментирую это и удалю тот ужас что сейчас заменяет свайп.
    m_ui->setupUi(this);
    statusBar()->addWidget(statuslbl, 1);
    statusBar()->addWidget(crcerrorlbl);
    statusBar()->addWidget(aboutButton);
    crcerrorlbl->setText(tr("CRC Errors: ") + QString::number(CRCErrorCount));
    statuslbl->setText(tr("Etrodiag"));
    aboutButton->setText(tr("About"));
    logger = new Logger;
    addConnection();
    connect (&byteSettForm, &ByteSettingsForm::editMask, &maskSettForm, &maskSettingsDialog::requestDataOnId);
    connect (this, &MainWindow::dvsfAfterCloseClear, &devSettForm, &devSettingsForm::afterCloseClearing);
    connect (m_ui->valueArea, &QTabWidget::currentChanged, this, &MainWindow::setCurrentOpenTab);
    connect (logger, &Logger::showStatusMessage, this, &MainWindow::showStatusMessage);
    connect (logger, &Logger::toTextLog, this, &MainWindow::textLogWindow);
    //connect (logger, &Logger::, connection, &newconnect::sendRawData); //TODO: Доделать загрузку лога - переработку через профиль в графики
    connect (logger, &Logger::, connection, &newconnect::sendRawData);
    connect (this, &MainWindow::toTxtLogger, logger, &Logger::incomingTxtData);
    connect (aboutButton, &QPushButton::clicked, this, &MainWindow::onAboutButtonClicked);
    m_ui->logArea->viewport()->installEventFilter(this);
    graphiq.setParent(m_ui->graphLabel);
    m_ui->graphLayout->addWidget(&cBoard);
    connect (&cBoard, &ControlBoard::controlCommand, this, &MainWindow::guiCommandHandler);
    connect (connection, &newconnect::setVisibleControlWindow, &cBoard, &ControlBoard::setVisible);
    connect (connection, &newconnect::s_sendSettings, logger, &Logger::setSettings);
    connect (this, &MainWindow::emitCommand, connection, &newconnect::receiveCommandFromGui);
    connect (m_ui->timeNavigationSlider, &QSlider::sliderReleased, this, [=](){
        emit timeNavigationSliderPositionChanged(m_ui->timeNavigationSlider->value());
    });
    connect (this, &MainWindow::timeNavigationSliderPositionChanged, graphiq, &liveGraph::timeNavigationSliderPositionChanged);
    m_ui->tabWidget->setCurrentIndex(0);
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
    connect (connection, &newconnect::writeTextLog, logger, &Logger::setTxt);
    connect (connection, &newconnect::writeJsonLog, logger, &Logger::setJson);
    connect (connection, &newconnect::writeBinLog, logger, &Logger::setBin);
    connect (connection, &newconnect::cleanGraph, &graphiq, &liveGraph::cleanGraph);
    connect (this, &MainWindow::prepareToSaveProfile, connection, &newconnect::prepareToSaveProfile);
    connect (this, &MainWindow::saveProfile, connection, &newconnect::saveProfile);
    connect (connection, &newconnect::sendRawData, logger, &Logger::incomingBinData);
    connect (connection, &newconnect::startLog, logger, &Logger::startLog);
    connect (connection, &newconnect::stopLog, logger, &Logger::stopLog);
    connect (connection, &newconnect::profileName2log, logger, &Logger::setProfileName);
    connect (connection, &newconnect::badCRC, this, &MainWindow::badCRCEvent);
    connect (connection, &newconnect::corruptedData, this, &MainWindow::corruptedDataEvent);
    connect(this, &MainWindow::emitCommand, connection, &newconnect::receiveCommandFromGui);
    connection->show();
}

void MainWindow::showStatusMessage(QString message)
{
    statuslbl->setText(message);
    textLogWindow(message, true);
}

void MainWindow::addDeviceToList(QDateTime currentTime, QVector<int> ddata)
{
    devNum = ddata.at(2);//узнаём номер устройства в посылке
    thisDeviceHere = false; //обнуляем флаг
    vlayChildList = m_ui->devArea->findChildren<Device*>();
    QListIterator<Device*> vlayChildListIt(vlayChildList); //смотрим сколько в гуе отображается устройств, создаём перечислитель
    while (vlayChildListIt.hasNext())
    {
        if (devNum == vlayChildListIt.next()->devNum) //смотрим, есть ли наше устройство в текущем листе
        {
            thisDeviceHere = true; //если есть, ставим флаг что оно тут
            emit devUpdate(currentTime, devNum, ddata); //если есть то пихаем ему обновление через сигнал
            devSettForm.updByteButtons(devNum, ddata); //обновление кнопок в форме настройки
        }
    }
    if (!thisDeviceHere) //если устройства нет, то создаём его
    {
        createDevice(devNum);
        emit devUpdate(currentTime, devNum, ddata);
        devSettForm.updByteButtons(devNum, ddata);
    }
    vlayChildListIt.toFront();
    m_ui->devArea->update();
}

void MainWindow::createDevice(int devNum)
{
    Device *dev = new Device(devNum);
    dev->setParent(m_ui->devArea);
    m_ui->devAreaLay->addWidget(dev);
    dev->setText(QString::number(devNum, 16));
    connect (this, &MainWindow::devUpdate, dev, &Device::updateData);
    connect (dev, &Device::openDevSettSig, this, &MainWindow::openDevSett);
    connect (dev, &Device::clicked, dev, &Device::clickedF);
    connect (this, &MainWindow::getDevName, dev, &Device::getDeviceName);
    connect (&devSettForm, &devSettingsForm::returnDevNameAfterEdit, dev, &Device::setDeviceName);
    connect (dev, &Device::returnDeviceName, &devSettForm, &devSettingsForm::setDevName);
    connect (&devSettForm, &devSettingsForm::openByteSettingsFormTX, this, &MainWindow::openByteSett);
    connect (&byteSettForm, &ByteSettingsForm::setWordBit, dev, &Device::setWordBitTX);
    connect (&byteSettForm, &ByteSettingsForm::setWordBit, &devSettForm, &devSettingsForm::wordTypeChangeRX);//изменить
    connect (&byteSettForm, &ByteSettingsForm::getWordType, dev, &Device::getWordTypeTX);
    connect (&devSettForm, &devSettingsForm::initByteButtonsWordLeight, dev, &Device::getWordTypeTX);
    connect (dev, &Device::returnWordTypeTX, &byteSettForm, &ByteSettingsForm::returnWordType);
    connect (dev, &Device::returnWordTypeTX, &devSettForm, &devSettingsForm::wordTypeChangeRX);
    connect (&byteSettForm, &ByteSettingsForm::createMask, dev, &Device::createNewMaskTX);
    connect (dev, &Device::mask2FormTX, &maskSettForm, &maskSettingsDialog::requestDataOnId);
    connect (&maskSettForm, &maskSettingsDialog::requestMaskData, dev, &Device::requestMaskDataTX);
    connect (&byteSettForm, &ByteSettingsForm::requestAllMaskToList, dev, &Device::requestMaskDataTX);
    connect (dev, &Device::maskData2FormTX, &maskSettForm, &maskSettingsDialog::getDataOnId);
    connect (dev, &Device::allMasksToListTX, &byteSettForm, &ByteSettingsForm::addMaskItem);
    connect (&maskSettForm, &maskSettingsDialog::sendMaskData, &byteSettForm, &ByteSettingsForm::addMaskItem);
    connect (&maskSettForm, &maskSettingsDialog::requestMaskData, this, &MainWindow::openMaskSettingsDialog);
    connect (&maskSettForm, &maskSettingsDialog::sendMaskData, dev, &Device::sendDataToProfileTX);
    connect (&byteSettForm, &ByteSettingsForm::deleteMaskObj, dev, &Device::deleteMaskObjTX);
    connect (&devSettForm, &devSettingsForm::wordDataFullHex, &byteSettForm, &ByteSettingsForm::updateHexWordData);
    connect (dev, &Device::param2FrontEndTX, this, &MainWindow::frontendDataSort);
    connect (dev, &Device::param2FrontEndTX, &maskSettForm, &maskSettingsDialog::liveDataSlot);
    connect (dev, &Device::param2FrontEndTX, &byteSettForm, &ByteSettingsForm::updateMasksList);
    connect (dev, &Device::param2FrontEndTX, &devSettForm, &devSettingsForm::liveDataSlot);
    connect (dev, &Device::param2FrontEndTX, &graphiq, &liveGraph::incomingDataSlot);
    connect (this, &MainWindow::sendMaskData, dev, &Device::loadMaskRX);
    connect (this, &MainWindow::hideOtherDevButtons, dev, &Device::hideDevButton);
    connect (dev, &Device::devStatusMessage, this, &MainWindow::devStatusMsg);
    connect (connection, &newconnect::saveAllMasks, dev, &Device::requestMasks4Saving);
    connect (dev, &Device::allMasksToListTX, connection, &newconnect::saveProfileSlot4Masks);
    connect (this, &MainWindow::toJsonMap, dev, &Device::jsonMap);
    connect (dev, &Device::devParamsToJson, logger, &Logger::incomingJsonData);
    dev->show();
}

void MainWindow::openDevSett(int devNum, QVector<int> data)
{ //все реакции на нажатие кнопки устройства в зависимости от состояния окна
    if (maskSettForm.isVisible())
    {
        maskSettForm.sendMask2Profile();
        maskSettForm.hide();
        maskSettForm.killChildren();
        if (maskSettForm.openDirectly)
        {
            emit hideOtherDevButtons(false, devNum);
            emit prepareToSaveProfile();
            emit saveProfile();
            maskSettForm.openDirectly = false;
            m_ui->valueArea->clear();
            graphiq.graphAnnotation.clear();
            m_ui->valueArea->show();
        }
        else {
            byteSettForm.show();
            byteSettForm.resize(m_ui->rightFrame->size());
        }
    }
    else {
        if (byteSettForm.isVisible())
        {
            byteSettForm.hide();
            byteSettForm.cleanForm();
            devSettForm.show();
            devSettForm.resize(m_ui->rightFrame->size());
        }
        else
        {
            devSettForm.setParent(m_ui->rightFrame);
            if (m_ui->valueArea->isHidden())
            {
                devSettForm.hide();
                emit dvsfAfterCloseClear();
                m_ui->valueArea->clear();
                graphiq.graphAnnotation.clear();
                m_ui->valueArea->show();
                emit hideOtherDevButtons(false, devNum);
                emit prepareToSaveProfile();
                emit saveProfile();
            }
            else
            {
                m_ui->valueArea->hide();
                emit hideOtherDevButtons(true, devNum);
                devSettForm.initByteButtons(devNum, data);
                emit getDevName(devNum);
                devSettForm.show();
                devSettForm.resize(m_ui->rightFrame->size());
            }
        }
    }
}

void MainWindow::openByteSett(int devNum, int byteNum)
{
    if (devSettForm.isVisible())
    {
        byteSettForm.setParent(m_ui->rightFrame);
        devSettForm.hide();
        byteSettForm.cleanForm();
        byteSettForm.open(devNum, byteNum);
        byteSettForm.resize(m_ui->rightFrame->size());
        byteSettForm.show();
        emit getByteName(devNum, byteNum);
    }
}

void MainWindow::openMaskSettingsDialog()
{
    if (byteSettForm.isVisible())
    {
        byteSettForm.hide();
        maskSettForm.setParent(m_ui->rightFrame);
        maskSettForm.show();
        maskSettForm.resize(m_ui->rightFrame->size());
    }
}

/*QDateTime MainWindow::returnTimestamp()
{
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    QDateTime dt3 = QDateTime::fromMSecsSinceEpoch(timestamp);
    return dt3;
}*/

void MainWindow::updValueArea(s_parameterMask mask)
{ //сначала проверяем есть ли уже вкладка с этим устройством по имени
    static int thisDeviceIndex = -1;
    for (int var = m_ui->valueArea->count(); var >= 0; --var) {
        if (m_ui->valueArea->tabText(var) == mask.devName)
        { //если есть то сохраняем индекс вкладки и покидаем цикл
            thisDeviceIndex = var;
            break;
        }
        else {
            thisDeviceIndex = -1;
        }
        //если нет то ставим индекс -1 чтоб триггернуться для последующей обработки
    }
    if (thisDeviceIndex == -1)
    { //создаём и инициализируем таблицу, добавляем виджет таблицы в новую вкладку имени девайса пришедшего в этой посылке
        QTableWidget *valueTableNew = new QTableWidget(m_ui->valueArea);
        connect(valueTableNew, &QTableWidget::cellClicked, this, &MainWindow::ValueArea_CellClicked);
        valueTableNew->insertColumn(0);//name
        valueTableNew->insertColumn(1);//value
        valueTableNew->insertColumn(2);//devnum
        valueTableNew->insertColumn(3);//bytenum
        valueTableNew->insertColumn(4);//maskid
        valueTableNew->hideColumn(2);//скрываем колонки с виду, данные в них нужны только для открытия настроек нужной маски
        valueTableNew->hideColumn(3);
        valueTableNew->hideColumn(4);
        valueTableNew->horizontalHeader()->hide();
        m_ui->valueArea->addTab(valueTableNew, mask.devName);
        //узнаём индекс только что созданной вкладки. Может быть стоит выделить это в отдельную функцию, но пока и так сойдёт
        for (int var = m_ui->valueArea->count(); var >= 0; --var) {
            if (m_ui->valueArea->tabText(var) == mask.devName) {
                thisDeviceIndex = var;
                break;
            }
            else {
                thisDeviceIndex = -1;
            }
        }
    }
    tmp = (m_ui->valueArea->widget(thisDeviceIndex)->metaObject()->className());
    //ищем виджет таблицы на вкладке и ссылаем на него статичный указатель
    if (tmp == "QTableWidget") {
        valueTable = (QTableWidget*)m_ui->valueArea->widget(thisDeviceIndex);
    }
    //далее работаем со строками таблицы по указателю
    findRow = false;
    namesUnited = (mask.parameterName + '@' + mask.devName);
    value2str.setNum(mask.endValue, 'g', 6);
    if (valueTable->rowCount() > 0)
    { //если строки есть то ищем нужную
        for (int i = 0; i < valueTable->rowCount(); i++)
        {
            if ((namesUnited) == valueTable->item(i, 0)->text())
            { //если найдена строка с именем и значение обновилось, подсвечиваем
                findRow = true;
                if (value2str != valueTable->item(i, 1)->text())
                {
                    valueTable->item(i, 1)->setText(value2str);
                    valueTable->item(i, 1)->setBackground(Qt::green);
                }
                else if (value2str == valueTable->item(i, 1)->text()) {
                    valueTable->item(i, 1)->setBackground(Qt::white);
                }
            }
        }
    }
    if (!findRow)
    { //если строка не найдена - создаём
        valueTable->setRowCount(valueTable->rowCount() + 1); //добавляем новую строку
        int row = valueTable->rowCount() - 1; //определяем индекс строки
        QTableWidgetItem *nameItem = new QTableWidgetItem;
        nameItem->setText(mask.parameterName + '@' + mask.devName);
        valueTable->setItem(row, 0, nameItem);
        QTableWidgetItem *valueItem = new QTableWidgetItem;
        valueItem->setText(value2str);
        valueTable->setItem(row, 1, valueItem);
        QTableWidgetItem *devNumItem = new QTableWidgetItem;
        devNumItem->setText(QString::number(mask.devNum));
        valueTable->setItem(row, 2, devNumItem);
        QTableWidgetItem *byteNumItem = new QTableWidgetItem;
        byteNumItem->setText(QString::number(mask.byteNum));
        valueTable->setItem(row, 3, byteNumItem);
        QTableWidgetItem *maskIdItem = new QTableWidgetItem;
        maskIdItem->setText(QString::number(mask.id));
        valueTable->setItem(row, 4, maskIdItem);
        valueTable->resizeColumnsToContents();
        valueTable->resizeRowsToContents();
    }
}

void MainWindow::setCurrentOpenTab(int index)
{
    currentOpenTab = index;
}

void MainWindow::ValueArea_CellClicked(int row, int)
{
    maskSettForm.openDirectly = true;
    static QTableWidget *table = nullptr;
    table = (QTableWidget*)m_ui->valueArea->widget(currentOpenTab);
    grabDevNum = table->item(row, 2)->text().toInt();
    grabByteNum = table->item(row, 3)->text().toInt();
    grabMaskId = table->item(row, 4)->text().toInt();
    maskSettForm.requestDataOnId(grabDevNum, grabByteNum, grabMaskId);
    maskSettForm.setParent(m_ui->rightFrame);
    m_ui->valueArea->hide();
    maskSettForm.show();
    maskSettForm.resize(m_ui->rightFrame->size());
    emit hideOtherDevButtons(true, grabDevNum);
}

void MainWindow::frontendDataSort(QDateTime currentTime, s_parameterMask mask)
{
    if (devSettForm.isVisible() && mask.devNum == devSettForm.devNum) {
        devSettForm.setDevName(mask.devNum, mask.devName);
    }
    if (mask.viewInLogFlag && mask.isNewData)
    {
        QString formString(mask.parameterName + "@" + mask.devName + ": " + QString::number(mask.endValue, 'g', 6));
        textLogWindow(currentTime, formString, false);
    }
    emit toJsonMap(mask);
    updValueArea(mask);
}

void MainWindow::textLogWindow(QDateTime currentTime, QString string, bool redFlag)
{
    QString stringWithTime = (currentTime.toString("hh:mm:ss:zzz") + " " + string);
    emit toTxtLogger(stringWithTime);
    if (!redFlag) {
        m_ui->logArea->appendHtml("<p><span style=color:#000000>" + stringWithTime + "</span></p>");
    }
    else {
        m_ui->logArea->appendHtml("<p><span style=color:#ff0000>" + stringWithTime + "</span></p>");
    }
}

void MainWindow::loadProfile(s_parameterMask mask)
{ //если устройства нет, то создаём, потом посылаем маску
    bool thisDeviceHere = false;
    QList<Device*> vlayChildList = m_ui->devArea->findChildren<Device*>();
    QListIterator<Device*> vlayChildListIt(vlayChildList);
    while (vlayChildListIt.hasNext())
        if (mask.devNum == vlayChildListIt.next()->devNum) {
            thisDeviceHere = true;
        }
    if (thisDeviceHere) {
        emit sendMaskData(mask);
    }
    else if (!thisDeviceHere)
    { //создаём устройство и инициализируем пустым пакетом в oneMsgLeight байт, с номером устройства на позиции 2
        createDevice(mask.devNum);
        QVector<int> devInitArray(oneMsgLeight, 0);
        devInitArray.replace(2, mask.devNum);
        emit devUpdate(QDateTime::currentDateTime(), mask.devNum, devInitArray);
        devSettForm.updByteButtons(mask.devNum, devInitArray);
        emit sendMaskData(mask);
    }
}

void MainWindow::devStatusMsg(QString _devName, QString status)
{
    textLogWindow(QDateTime::currentDateTime(), tr("Device %1 is %2").arg(_devName).arg(status), true);
}

void MainWindow::resizeEvent(QResizeEvent*)
{
    devSettForm.resize(m_ui->rightFrame->size());
    byteSettForm.resize(m_ui->rightFrame->size());
    maskSettForm.resize(m_ui->rightFrame->size());
    graphiq.resize(m_ui->graphLabel->size());
}

void MainWindow::cleanDevList()
{
    QList<Device*> vlayChildList = m_ui->devArea->findChildren<Device*>();
    QListIterator<Device*> vlayChildListIt(vlayChildList);
    while(vlayChildListIt.hasNext()) {
        vlayChildListIt.next()->~Device();
    }
    CRCErrorCount = 0;
}

void MainWindow::on_tabWidget_currentChanged(int)
{ //так как сразу после пуска программы ресайз виджета не срабатывает, вешаю его на событие смены таба
    graphiq.resize(m_ui->graphLabel->size());
}

void MainWindow::onAboutButtonClicked(bool)
{
    aboutDialog.show();
}

void MainWindow::badCRCEvent(uint8_t calculatedCRC, QVector<int> dataFrame)
{
    QString str, chr, crcchr;
    for (int i = 0; i < dataFrame.size(); ++i)
    {
        if (i > 0) {
            str += ":";
        }
        chr = QString::number(dataFrame[i], 16).toUpper();
        if (chr.size() == 1) {
            chr = '0' + chr;
        }
        str += chr;
    }
    crcchr = QString::number(calculatedCRC, 16).toUpper();
    if (crcchr.size() == 1) {
        crcchr = '0' + crcchr;
    }
    textLogWindow(QDateTime::currentDateTime(), tr("CRC Calc: ") + crcchr + ", " + tr("Frame: ") + str, true);
    CRCErrorCount++;
    crcerrorlbl->setText(tr("CRC Errors: ") + QString::number(CRCErrorCount));
}

void MainWindow::corruptedDataEvent(QVector<int> data)
{
    QString str, chr;
    for (int i = 0; i < data.size(); ++i)
    {
        if (i > 0) {
            str += ":";
        }
        chr = QString::number(data[i], 16).toUpper();
        if (chr.size() == 1) {
            chr = '0' + chr;
        }
        str += chr;
    }
    textLogWindow(QDateTime::currentDateTime(), tr("Corrupted data: ") + str, true);
}

void MainWindow::guiCommandHandler(int varNumber, bool action)
{
    uint8_t actionChr = action ? 1 : 0;
    uint8_t varNumberChr = static_cast<unsigned char>(varNumber);
    QVector<quint8> command = {0xFF, 0xAB, 0x01, varNumberChr, actionChr, 0};
    /*
    1 - (FF AB) начало пакета
    2 - Тип команды (1 - изменение переменной)
    3 - Условный номер переменной
    4 - Воздействие на переменную (0 -, 1 +)
    5 - контрольная сумма, считается уже при передаче
    */
    emit emitCommand(command, true);
}
//так как не получилось заставить работать SwipeGesture, я напишу свой свайп. Для пролистывания табов его хватит.
/*bool MainWindow::eventFilter(QObject *obj, QEvent *event)//взято из документации к QObject::eventFilter
{//ещё немножко костылей ради того что-бы свайп работал

    if (obj == m_ui->logArea || m_ui->valueArea || m_ui->aboutText)
    {
        if ((event->type() == QEvent::MouseButtonPress) || (event->type() == QEvent::MouseButtonRelease))
        {
            QMouseEvent mouseev(*static_cast<QMouseEvent*>(event));
            swipeCalc(mouseev);
            return true;
        }
    else return false;
    }
    else return QMainWindow::eventFilter(obj, event);
}
*/
bool MainWindow::event(QEvent *event)
{
    /*if ((event->type() == QEvent::MouseButtonPress) || (event->type() == QEvent::MouseButtonRelease))
    {
          QMouseEvent mouseEvent = *static_cast<QMouseEvent*>(event);
          #ifdef Q_OS_ANDROID
          swipeCalc(mouseEvent);
          #endif
    }*/
    if ((event->type() == QEvent::MouseButtonDblClick) && graphiq.isVisible())
    {
        graphiq.chngMinMaxVisible();
    }
    return QMainWindow::event(event);
}
/*
void MainWindow::swipeCalc(QMouseEvent mouseev)
{
    if (mouseev.type() == QMouseEvent::MouseButtonPress)
        {
            mouseStartX = mouseev.x();
            mouseStartY = mouseev.y();
        }
    if (mouseev.type() == QMouseEvent::MouseButtonRelease)
        {
            touchTrigger = false;
            mouseStopX = mouseev.x();
            mouseStopY = mouseev.y();
            int calcx = mouseStartX - mouseStopX;
            int calcy = mouseStartY - mouseStopY;
            bool xpositive;
            bool ypositive;
            static QString direction;
            int pixelsToSwipe = 200; //граница после которой действие будет однозначно восприниматься как свайп, в пикселях
            if (calcx > pixelsToSwipe) xpositive = true;
            else if (calcx < 0)
            {
                calcx = calcx * -1;
                if (calcx > pixelsToSwipe) xpositive = false;
            }
            if (calcy > pixelsToSwipe) ypositive = true;
            else if (calcy < 0)
            {
                calcy = calcy * -1;
                if (calcy > pixelsToSwipe) ypositive = false;
            }
            if ((calcx > calcy) && (calcx > pixelsToSwipe))
            {
                if (xpositive) direction = "Right";
                else if (!xpositive) direction = "Left";
            }
            else if ((calcx < calcy) && (calcy > pixelsToSwipe))
            {
                if (ypositive) direction = "Up";
                else if (!ypositive) direction = "Down";
            }
            if (!direction.isEmpty())
            swipeTriggered(direction);
        }
}

void MainWindow::swipeTriggered(QString gesture)
{
    if (gesture == "Left")
        {
            m_ui->tabWidget->setCurrentIndex(m_ui->tabWidget->currentIndex()-1);
        } else if (gesture == "Right")
        {
            m_ui->tabWidget->setCurrentIndex(m_ui->tabWidget->currentIndex()+1);
        }
        update();
}*/

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
#include <QGestureEvent>
#include <QSwipeGesture>
#include <QMap>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), statuslbl (new QLabel), m_ui (new Ui::MainWindow)

{
//    QVector<Qt::GestureType> gestures;
//    gestures << Qt::SwipeGesture << Qt::TapGesture;
//    for (Qt::GestureType gesture : gestures)
//    grabGesture(gesture);
//  Надеюсь, что когда в qt починят qswipegesture, я раскомментирую это и удалю тот ужас что сейчас заменяет свайп.
    m_ui->setupUi(this);
    statusBar()->addWidget(statuslbl);
    statuslbl->setText("Etrodiag beta");
    addConnection();    
    connect (&byteSettForm, &ByteSettingsForm::editMask, &maskSettForm, &maskSettingsDialog::requestDataOnId);
    connect (this, &MainWindow::dvsfAfterCloseClear, &devSettForm, &devSettingsForm::afterCloseClearing);
    connect (m_ui->valueArea, &QTabWidget::currentChanged, this, &MainWindow::setCurrentOpenTab);
    connect (&logger, &Logger::showStatusMessage, this, &MainWindow::showStatusMessage);
    connect (&logger, &Logger::toTextLog, this, &MainWindow::logAreaAppendHtml);
    connect (this, &MainWindow::toTxtLogger, &logger, &Logger::incomingTxtData);

    m_ui->logArea->viewport()->installEventFilter(this);

    //m_ui->textBrowser->viewport()->installEventFilter(this);

    m_ui->aboutimg->setPixmap(pixmap->scaledToWidth(m_ui->tab_about->size().width(), Qt::FastTransformation));
    m_ui->aboutimg->setScaledContents(true);
    m_ui->aboutimg->show();
    graphiq.setParent(m_ui->graphLabel);
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
    connect (connection, &newconnect::writeTextLog, &logger, &Logger::setTxt);
    connect (connection, &newconnect::writeJsonLog, &logger, &Logger::setJson);
    connect (connection, &newconnect::writeBinLog, &logger, &Logger::setBin);
    connect (connection, &newconnect::cleanGraph, &graphiq, &liveGraph::cleanGraph);
    connect (this, &MainWindow::prepareToSaveProfile, connection, &newconnect::prepareToSaveProfile);
    connect (this, &MainWindow::saveProfile, connection, &newconnect::saveProfile);
    connect (connection, &newconnect::sendRawData, &logger, &Logger::incomingBinData);
    connect (connection, &newconnect::startLog, &logger, &Logger::startLog);
    connect (connection, &newconnect::stopLog, &logger, &Logger::stopLog);
    connect (connection, &newconnect::profileName2log, &logger, &Logger::setProfileName);
    connection->show();
}

void MainWindow::showStatusMessage(QString message)
{
    statuslbl->setText(message);
    textLogWindow(message, true);
}

void MainWindow::addDeviceToList(QVector<int> ddata)
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
           emit devUpdate(devNum, ddata); //если есть то пихаем ему обновление через сигнал
           devSettForm.updByteButtons(devNum, ddata); //обновление кнопок в форме настройки
        }
    }
    if (!thisDeviceHere) //если устройства нет, то создаём его
    {
        createDevice(devNum);
        emit devUpdate(devNum, ddata);
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
    dev->setText(QString::number(devNum,16));    
    connect (this, &MainWindow::devUpdate, dev, &Device::updateData);
    connect (dev, &Device::openDevSettSig, this, &MainWindow::openDevSett);
    connect (dev, &Device::clicked, dev, &Device::clickedF);
    connect (this, &MainWindow::getDevName, dev, &Device::getDeviceName);
    connect (&devSettForm, &devSettingsForm::returnDevNameAfterEdit, dev, &Device::setDeviceName);
    connect (dev, &Device::returnDeviceName, &devSettForm, &devSettingsForm::setDevName);
    connect (&devSettForm, &devSettingsForm::openByteSettingsFormTX, this, &MainWindow::openByteSett);
    connect (&byteSettForm, &ByteSettingsForm::setWordBit, dev, &Device::setWordTypeInByteProfile);
    connect (&byteSettForm, &ByteSettingsForm::setWordBit, &devSettForm, &devSettingsForm::wordTypeChangeRX);//изменить
    connect (&byteSettForm, &ByteSettingsForm::getWordType, dev, &Device::getWordTypeFromProfileRetranslator);
    connect (&devSettForm, &devSettingsForm::initByteButtonsWordLeight, dev, &Device::getWordTypeFromProfileRetranslator);
    connect (dev, &Device::returnWordTypeTX, &byteSettForm, &ByteSettingsForm::returnWordType);
    connect (dev, &Device::returnWordTypeTX, &devSettForm, &devSettingsForm::wordTypeChangeRX);
    connect (&byteSettForm, &ByteSettingsForm::createMask, dev, &Device::createNewMaskRX);
    connect (dev, &Device::mask2FormTX, &maskSettForm, &maskSettingsDialog::requestDataOnId);
    connect (&maskSettForm, &maskSettingsDialog::requestMaskData, dev, &Device::requestMaskDataRX);
    connect (&byteSettForm, &ByteSettingsForm::requestAllMaskToList, dev, &Device::requestMaskDataRX);
    connect (dev, &Device::maskData2FormTX, &maskSettForm, &maskSettingsDialog::getDataOnId);
    connect (dev, &Device::allMasksToListTX, &byteSettForm, &ByteSettingsForm::addMaskItem);
    connect (&maskSettForm, &maskSettingsDialog::sendMaskData, &byteSettForm, &ByteSettingsForm::addMaskItem);
    connect (&maskSettForm, &maskSettingsDialog::requestMaskData, this, &MainWindow::openMaskSettingsDialog);
    connect (&maskSettForm, &maskSettingsDialog::sendMaskData, dev, &Device::sendDataToProfileRX);
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
    connect (dev, &Device::devParamsToJson, &logger, &Logger::incomingJsonData);
    dev->show();
}

void MainWindow::openDevSett(int devNum, QVector<int> data)
{//все реакции на нажатие кнопки устройства в зависимости от состояния окна
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
        devSettForm.initByteButtons(devNum,data);
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

QDateTime MainWindow::returnTimestamp()
{
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    QDateTime dt3 = QDateTime::fromMSecsSinceEpoch(timestamp);
    return dt3;
}

void MainWindow::updValueArea(QString parameterName, int devNum, QString devName, double endValue, int byteNum, int maskId, bool)
{//сначала проверяем есть ли уже вкладка с этим устройством по имени
    static int thisDeviceIndex = -1;
    for (int var = m_ui->valueArea->count(); var >= 0; --var) {
        if (m_ui->valueArea->tabText(var) == devName)
            {//если есть то сохраняем индекс вкладки и покидаем цикл
                thisDeviceIndex = var;
                break;
            }
        else thisDeviceIndex = -1;
            //если нет то ставим индекс -1 чтоб триггернуться для последующей обработки
    }

    if (thisDeviceIndex == -1)
    {//создаём и инициализируем таблицу, добавляем виджет таблицы в новую вкладку имени девайса пришедшего в этой посылке
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
        m_ui->valueArea->addTab(valueTableNew, devName);

        //узнаём индекс только что созданной вкладки. Может быть стоит выделить это в отдельную функцию, но пока и так сойдёт
        for (int var = m_ui->valueArea->count(); var >= 0; --var) {
            if (m_ui->valueArea->tabText(var) == devName){
                    thisDeviceIndex = var;
                    break;
                }
            else thisDeviceIndex = -1;
        }
    }

    tmp = (m_ui->valueArea->widget(thisDeviceIndex)->metaObject()->className());
    //ищем виджет таблицы на вкладке и ссылаем на него статичный указатель
    if (tmp == "QTableWidget") valueTable = (QTableWidget*)m_ui->valueArea->widget(thisDeviceIndex);
    //далее работаем со строками таблицы по указателю
        findRow = false;
        namesUnited = (parameterName+'@'+devName);
        value2str.setNum(endValue, 'g', 6);
        if (valueTable->rowCount() > 0)
        {//если строки есть то ищем нужную
            for (int i = 0; i < valueTable->rowCount(); i++)
            {
                if ((namesUnited) == valueTable->item(i,0)->text())
                {//если найдена строка с именем и значение обновилось, подсвечиваем
                    findRow = true;
                    if (value2str != valueTable->item(i,1)->text())
                    {
                        valueTable->item(i,1)->setText(value2str);
                        valueTable->item(i,1)->setBackgroundColor(Qt::green);
                       //метод setbackgroundcolor устаревший, по возможности переписать на делегаты
                    }
                    else if (value2str == valueTable->item(i,1)->text())
                        valueTable->item(i,1)->setBackgroundColor(Qt::white);
                }
            }
        }
        if (!findRow)
        {//если строка не найдена - создаём
            valueTable->setRowCount(valueTable->rowCount()+1); //добавляем новую строку
            int row = valueTable->rowCount()-1;//определяем индекс строки
            QTableWidgetItem *nameItem = new QTableWidgetItem;
            nameItem->setText(parameterName+'@'+devName);
            valueTable->setItem(row, 0, nameItem);
            QTableWidgetItem *valueItem = new QTableWidgetItem;
            valueItem->setText(value2str);
            valueTable->setItem(row, 1, valueItem);
            QTableWidgetItem *devNumItem = new QTableWidgetItem;
            devNumItem->setText(QString::number(devNum));
            valueTable->setItem(row, 2, devNumItem);
            QTableWidgetItem *byteNumItem = new QTableWidgetItem;
            byteNumItem->setText(QString::number(byteNum));
            valueTable->setItem(row, 3, byteNumItem);
            QTableWidgetItem *maskIdItem = new QTableWidgetItem;
            maskIdItem->setText(QString::number(maskId));
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

void MainWindow::frontendDataSort(int devNum, QString devName, int byteNum, QString, int, int maskId, QString parameterName, int, double endValue, bool viewInLogFlag, bool isNewData, bool _drawGraphFlag, QString _drawGraphColor)
{    
    if (devSettForm.isVisible() && devNum == devSettForm.devNum)
        devSettForm.setDevName(devNum, devName);

    if (viewInLogFlag && isNewData)
    {        
        QString formString(parameterName + "@" + devName + ": " + QString::number(endValue,'g',6));
        textLogWindow(formString, false);
    }
    emit toJsonMap(devNum, devName, parameterName, endValue, maskId);
    updValueArea(parameterName, devNum, devName, endValue, byteNum, maskId, isNewData);
}

void MainWindow::textLogWindow(QString string, bool redFlag)
{
    QString stringWithTime = (returnTimestamp().toString("hh:mm:ss:zzz") + " " + string);
    emit toTxtLogger(stringWithTime);
    if (!redFlag) m_ui->logArea->appendHtml("<p><span style=color:#000000>" + stringWithTime + "</span></p>");
    else m_ui->logArea->appendHtml("<p><span style=color:#ff0000>" + stringWithTime + "</span></p>");
}

void MainWindow::loadProfile(int devNum, QString devName, int byteNum, QString byteName, int id, QString paramName, QString paramMask, int paramType, double valueShift, double valueKoef, bool viewInLogFlag, int wordType, bool drawGraphFlag, QString drawGraphColor)
{//если устройства нет, то создаём, потом посылаем маску
    bool thisDeviceHere = false;
    QList<Device*> vlayChildList = m_ui->devArea->findChildren<Device*>();
    QListIterator<Device*> vlayChildListIt(vlayChildList);
    while (vlayChildListIt.hasNext())
        if (devNum == vlayChildListIt.next()->devNum)
            thisDeviceHere = true;
    if (thisDeviceHere)
        emit sendMaskData(devNum, devName, byteNum, byteName, id, paramName, paramMask, paramType, valueShift, valueKoef, viewInLogFlag, wordType, drawGraphFlag, drawGraphColor);
    else if (!thisDeviceHere)
    {//создаём устройство и инициализируем пустым пакетом в 40 байт, с номером устройства на позиции 2
        createDevice(devNum);
        QVector<int> devInitArray(40,0);
        devInitArray.replace(2, devNum);
        emit devUpdate(devNum, devInitArray);
        devSettForm.updByteButtons(devNum, devInitArray);
        emit sendMaskData(devNum, devName, byteNum, byteName, id, paramName, paramMask, paramType, valueShift, valueKoef, viewInLogFlag, wordType, drawGraphFlag, drawGraphColor);
    }
}

void MainWindow::devStatusMsg(QString _devName, QString status)
{
    textLogWindow(tr("Device %1 is %2").arg(_devName).arg(status), true);
}

void MainWindow::resizeEvent(QResizeEvent*)
{
    devSettForm.resize(m_ui->rightFrame->size());
    byteSettForm.resize(m_ui->rightFrame->size());
    maskSettForm.resize(m_ui->rightFrame->size());
    graphiq.resize(m_ui->graphLabel->size());

    m_ui->helpText->viewport()->setMinimumWidth(m_ui->tab_about->width()*0.9);
    m_ui->helpText->document()->setTextWidth(m_ui->helpText->viewport()->size().width());
    auto docSize = m_ui->helpText->document()->size().toSize();
    m_ui->helpText->setMinimumHeight(docSize.height() + 10);
}

void MainWindow::cleanDevList()
{
    QList<Device*> vlayChildList = m_ui->devArea->findChildren<Device*>();
    QListIterator<Device*> vlayChildListIt(vlayChildList);
    while(vlayChildListIt.hasNext())
        vlayChildListIt.next()->~Device();
}

void MainWindow::on_tabWidget_currentChanged(int)
{//так как сразу после пуска программы ресайз виджета не срабатывает, вешаю его на событие смены таба
    graphiq.resize(m_ui->graphLabel->size());
}

void MainWindow::logAreaAppendHtml(QString str)
{
    textLogWindow(str, true);
}

//так как не получилось заставить работать SwipeGesture, я напишу свой свайп. Для пролистывания табов его хватит.
bool MainWindow::eventFilter(QObject *obj, QEvent *event)//взято из документации к QObject::eventFilter
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

bool MainWindow::event(QEvent *event)
{
    if ((event->type() == QEvent::MouseButtonPress) || (event->type() == QEvent::MouseButtonRelease))
    {
          QMouseEvent mouseEvent = *static_cast<QMouseEvent*>(event);
          #ifdef Q_OS_ANDROID
          swipeCalc(mouseEvent);
          #endif
    }
    if ((event->type() == QEvent::MouseButtonDblClick) && graphiq.isVisible())
    {
        graphiq.chngMinMaxVisible();
    }

    return QMainWindow::event(event);
}

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
}

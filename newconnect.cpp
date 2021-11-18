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
#include <QtXml/QtXml>
#include <QtXml/QDomDocument>
#include <QFile>

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
    connect(datapool, &dataprofiler::deviceData, this, &newconnect::transData);
    connect(datapool, &dataprofiler::ready4read, gstream, &getStream::readPermission);
    connect(datapool, &dataprofiler::readNext, gstream, &getStream::readIntByte);
    connect(m_settings, &SettingsDialog::restoreConsoleAndButtons, this, &newconnect::restoreWindowAfterApplySettings);
    connect (m_settings, &SettingsDialog::writeTextLog, this, &newconnect::writeTextLog);
    connect (m_settings, &SettingsDialog::writeBinLog, this, &newconnect::writeBinLog);
    connect (m_settings, &SettingsDialog::writeJsonLog, this, &newconnect::writeJsonLog);
    connect (timer, &QTimer::timeout, this, &newconnect::readFromFile);//читаем из файла по таймеру
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
        int freq = 1000/((p_local.baudRate/8)/bytesPerOneShot);
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
    } else {
                QMessageBox::critical(this, tr("Error"), m_serial->errorString());
                showStatusMessage(tr("Open error"));
           }
    }
}

void newconnect::readFromFile()
{
    if (pos < fileSplitted.size())
    {//если текущая позиция не в конце списка (костыль вместо итератора) то кусок по нужному номеру листа добавляем в fsba
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

        //тут вкорячить очистку xml от всех масок

        permission2SaveMasks = true;
        emit saveAllMasks();
    }
}

void newconnect::saveProfileSlot4Masks(bitMaskDataStruct &bitMask)//Переделать сохранение на xml
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
                   if ((QString::number(bitMask.id,10) == maskVectorsListIt.peekNext()->lst.at(1))&&
                           (QString::number(bitMask.devNum,10) == maskVectorsListIt.peekNext()->lst.at(2))&&
                           (QString::number(bitMask.byteNum,10)==maskVectorsListIt.peekNext()->lst.at(3))&&
                           (maskVectorsListIt.peekNext()->lst.at(0) == "thisIsMask"))
                      thisMaskHere = true;
                   maskVectorsListIt.next();
               }
               if (!thisMaskHere)
               {
                   //(int devNum, QString devName, int byteNum, QString byteName, int id, QString paramName,
                   //QString paramMask, int, double valueShift, double valueKoef, bool viewInLogFlag, int wordType,
                   //bool _drawGraphFlag, QString _drawGraphColor);

                   const SettingsDialog::Settings p = m_settings->settings();
                   QFile profile(p.profilePath);
                   QFileInfo info(profile);
                   profile.open(QIODevice::ReadWrite);

                   QDomDocument doc;
                   doc.setContent(&profile);

                   QDomProcessingInstruction instruction;
                   QDomElement root;
                   QDomElement strName;
                   QDomText strNameNodeText;
                   QDomElement strDevices;
                   QDomElement strDev;
                   QDomAttr attrDev;

                   if (doc.documentElement().nodeName() != "profile"){

                   instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
                   doc.appendChild(instruction);

                   root = doc.createElement("profile");// Создать корневой узел
                   doc.appendChild(root);// Добавить корневой узел

                   strName = doc.createElement("name");// Создать узел элемента
                   root.appendChild(strName);// Добавить узел элемента в корневой узел
                   strNameNodeText = doc.createTextNode(info.fileName());// Создать текст элемента
                   strName.appendChild(strNameNodeText);// Добавить текст элемента к элементу узла

                   strDevices = doc.createElement("devices");
                   root.appendChild(strName);
                   }

                   //перед созданием элемента устройства, проверяем что его нет в профиле по id
                   QVector<QString> devList;
                   int devIndex;
                   for (int i = 0; i < strDevices.childNodes().count(); i++){ //формируем список того что есть в файле
                       devList.append(strDevices.childNodes().at(i).attributes().namedItem("id").nodeValue());
                       if (strDevices.childNodes().at(i).attributes().namedItem("id").nodeValue() == QString::number(bitMask.id, 10))
                           devIndex = i; //при нахождении запоминаем индекс
                   }
                   if (!devList.contains(QString::number(bitMask.id, 10))){
                   strDev = doc.createElement("dev");
                   strDevices.appendChild(strName);
                   attrDev = doc.createAttribute("id");
                   attrDev.setValue(QString::number(bitMask.id, 10));
                   strDev.appendChild(attrDev);
                   }
                   else strDev = strDevices.childNodes().at(devIndex).toElement();

                   QDomElement strMasks = doc.createElement("masks");
                   strDev.appendChild(strMasks);//интересно, в какой dev будет писать



                   QDomElement strMask = doc.createElement("mask");
                   strMasks.appendChild(strMask);
                   QDomText strMaskNodeText = doc.createTextNode(bitMask.paramName);
                   strMask.appendChild(strMaskNodeText);
                   QDomAttr attrMask = doc.createAttribute("id");
                   attrMask.setValue(QString::number(bitMask.id, 10));
                   strMask.appendChild(attrMask);

                   QDomElement strParamMask = doc.createElement("paramMask");
                   strMask.appendChild(strParamMask);
                   QDomText strParamMaskNodeText = doc.createTextNode(bitMask.paramMask);
                   strMask.appendChild(strParamMaskNodeText);

                   QDomElement strValueShift = doc.createElement("valueShift");
                   strMask.appendChild(strValueShift);
                   QDomText strValueShiftNodeText = doc.createTextNode(QString::number(bitMask.valueShift, '.', 6));
                   strMask.appendChild(strValueShiftNodeText);

                   QDomElement strValueKoef = doc.createElement("valueKoef");
                   strMask.appendChild(strValueKoef);
                   QDomText strValueKoefNodeText = doc.createTextNode(QString::number(bitMask.valueKoef, '.', 6));
                   strMask.appendChild(strValueKoefNodeText);

                   QDomElement strViewInLogFlag = doc.createElement("viewInLogFlag");
                   strMask.appendChild(strViewInLogFlag);
                   QDomText strViewInLogFlagNodeText = doc.createTextNode(bitMask.viewInLogFlag ?"true":"false");
                   strMask.appendChild(strViewInLogFlagNodeText);

                   QDomElement strWordType = doc.createElement("wordType");
                   strMask.appendChild(strWordType);
                   QDomText strWordTypeNodeText = doc.createTextNode(QString::number(bitMask.wordType, 10));
                   strMask.appendChild(strWordTypeNodeText);

                   QDomElement strDrawGraph = doc.createElement("drawGraph");
                   strMask.appendChild(strDrawGraph);
                   QDomAttr attrDrawGraph = doc.createAttribute("color");
                   attrDrawGraph.setValue(bitMask.drawGraphColor);
                   strDrawGraph.appendChild(attrDrawGraph);
                   QDomText strDrawGraphNodeText = doc.createTextNode(bitMask.drawGraphFlag ?"true":"false");
                   strDrawGraph.appendChild(strDrawGraphNodeText);

//                   const SettingsDialog::Settings p = m_settings->settings();
//                   QFile profile(p.profilePath);
//                   QFileInfo info(profile);
//                   profile.open(QIODevice::WriteOnly);
//                   QXmlStreamWriter xmlWriter(&profile);
//                       xmlWriter.setAutoFormatting(true);  // Устанавливаем автоформатирование текста
//                       xmlWriter.writeStartDocument();     // Запускаем запись в документ
//                       xmlWriter.writeStartElement("profile");   // Записываем первый элемент с его именем

//                       xmlWriter.writeStartElement("name");
//                       xmlWriter.writeCharacters(info.fileName());
//                       xmlWriter.writeEndElement();

//                       xmlWriter.writeStartElement("devices");
//                       xmlWriter.writeStartElement("dev");
//                       xmlWriter.writeAttribute("devNum", QString::number(bitMask.devNum, 10));
//                       xmlWriter.writeCharacters(bitMask.devName);
//                       xmlWriter.writeStartElement("masks");

//                       xmlWriter.writeStartElement("mask");
//                       xmlWriter.writeAttribute("id", QString::number(bitMask.id, 10));
//                       xmlWriter.writeCharacters(bitMask.paramName);

//                       xmlWriter.writeStartElement("paramMask");
//                       xmlWriter.writeCharacters(bitMask.paramMask);
//                       xmlWriter.writeEndElement();

//                       xmlWriter.writeStartElement("valueShift");
//                       xmlWriter.writeCharacters(QString::number(bitMask.valueShift, '.', 6));
//                       xmlWriter.writeEndElement();

//                       xmlWriter.writeStartElement("valueKoef");
//                       xmlWriter.writeCharacters(QString::number(bitMask.valueKoef, '.', 6));
//                       xmlWriter.writeEndElement();

//                       xmlWriter.writeStartElement("viewInLogFlag");
//                       xmlWriter.writeCharacters(bitMask.viewInLogFlag ?"true":"false");
//                       xmlWriter.writeEndElement();

//                       xmlWriter.writeStartElement("wordType");
//                       xmlWriter.writeCharacters(QString::number(bitMask.wordType, 10));
//                       xmlWriter.writeEndElement();

//                       xmlWriter.writeStartElement("drawGraph");
//                       xmlWriter.writeAttribute("color", bitMask.drawGraphColor);
//                       xmlWriter.writeCharacters(bitMask.drawGraphFlag ?"true":"false");
//                       xmlWriter.writeEndElement();

//                       xmlWriter.writeEndElement(); //mask
//                       xmlWriter.writeEndElement(); //masks
//                       xmlWriter.writeEndElement(); //dev
//                       xmlWriter.writeEndElement(); //devices
//                       xmlWriter.writeEndElement(); //profile
//                       xmlWriter.writeEndDocument();

//                       profile.close();




//                   QList<QString> maskList;
//                   maskList.append("thisIsMask");//0
//                   maskList.append(QString::number(bitMask.id,10));//1
//                   maskList.append(QString::number(bitMask.devNum,10));//2
//                   maskList.append(QString::number(bitMask.byteNum,10));//3
//                   maskList.append(bitMask.devName);//4
//                   maskList.append(bitMask.byteName);//5
//                   maskList.append(bitMask.paramName);//6
//                   maskList.append(bitMask.paramMask);//7
//                   maskList.append(QString::number(bitMask.valueShift,'g',6));//8
//                   maskList.append(QString::number(bitMask.valueKoef,'g',6));//9
//                   maskList.append((bitMask.viewInLogFlag ?"true":"false"));//10
//                   maskList.append(QString::number(bitMask.wordType));//11
//                   maskList.append(bitMask.drawGraphFlag ?"true":"false");//12
//                   maskList.append(bitMask.drawGraphColor);//13
//                   txtmaskobj *savingMask = new txtmaskobj(maskList);
//                   savingMask->setParent(this);
//                   maskList.clear();
               }
           }
           }

void newconnect::saveProfile()
{
    //тут переписать на запись xml в файл уже после формирования в памяти
    if (permission2SaveMasks)
    {
        maskVectorsList = this->findChildren<txtmaskobj*>();
        QListIterator<txtmaskobj*> maskVectorsListIt(maskVectorsList);
        maskVectorsListIt.toFront();
        const SettingsDialog::Settings p = m_settings->settings();
        QFile profile(p.profilePath);
        QFileInfo info(profile);        
        profile.open(QIODevice::WriteOnly|QIODevice::Text);
        QTextStream txtStream(&profile);
        txtStream << info.fileName() << "\n";
        while (maskVectorsListIt.hasNext())
        {
            QListIterator<QString> lstIt(maskVectorsListIt.peekNext()->lst);
            while (lstIt.hasNext())
            txtStream << lstIt.next() << "\t";
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
    profile.open(QIODevice::ReadOnly|QIODevice::Text);
    QTextStream txtStream(&profile);
    while (!txtStream.atEnd())
    {
        QString str = txtStream.readLine();
        QStringList strLst = str.split('\t');
        if (strLst.at(0)=="thisIsMask")
            emit loadMask(strLst.at(2).toInt(0,10),strLst.at(4),strLst.at(3).toInt(0,10),strLst.at(5),
                          strLst.at(1).toInt(0,10),strLst.at(6),strLst.at(7),0,strLst.at(8).toDouble(),
                          strLst.at(9).toDouble(),((QString::compare(strLst.at(10), "true") == 0) ? true : false),
                          strLst.at(11).toInt(0,10), ((QString::compare(strLst.at(12), "true") == 0) ? true : false),
                          strLst.at(13));
        //(int devNum, QString devName, int byteNum, QString byteName, int id, QString paramName, QString paramMask,
        //int paramType, double valueShift, double valueKoef, bool viewInLogFlag, int wordType, bool _drawGraphFlag,
        //QString _drawGraphColor);
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

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
#include <QStandardPaths>
#include <QtXml/QtXml>
#include <QtXml/QDomNode>
#include <QFile>
#include <QDomNodeList>

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
        } else {
            QMessageBox::critical(this, tr("Error"), m_serial->errorString());
            showStatusMessage(tr("Open error"));
        }
    }
}

void newconnect::readFromFile()
{
    if (pos < fileSplitted.size())
    {   //если текущая позиция не в конце списка (костыль вместо итератора) то кусок по нужному номеру листа добавляем в fsba
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
    p = m_settings->settings();
    if (!p.readOnlyProfile)
    {   //очищаем список, выставляем разрешение для дальнейших операций по сохранению, даём сигнал на запрос всех масок
//        maskVectorsList = this->findChildren<bitMaskDataStruct*>();
//        QListIterator<bitMaskDataStruct*> maskVectorsListIt(maskVectorsList);
//        maskVectorsListIt.toFront();
//        while (maskVectorsListIt.hasNext()) {
//            maskVectorsListIt.next()->~bitMaskDataStruct();
//        }
        profile.setFileName(p.profilePath);
        QFileInfo info(profile);
        //profile.open(QIODevice::ReadWrite);
        QDomDocument doc;
        //doc.setContent(&profile);
        QDomProcessingInstruction instruction;
        instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
        doc.appendChild(instruction);
        QDomElement root = doc.createElement("profile");// Создать корневой узел
        doc.appendChild(root);// Добавить корневой узел
        root.setAttribute("name", info.fileName());

        QDomElement strDevices = doc.createElement("devices");
        root.appendChild(strDevices);

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

        QDomNodeList devList = strDevices.elementsByTagName("device"); //Список устройств
        QDomElement *strDev = new (QDomElement);
        //new (QDomElement);// = doc.createElementNS("device", QString::number(bitMask.devNum));

        if (devList.size() < 1) { //Если нет устройств то создаём
            *strDev = doc.createElementNS("device", QString::number(bitMask.devNum));
            strDevices.appendChild(*strDev);
            strDev->setAttribute("name", bitMask.devName);
            strDev->setAttribute("num", bitMask.devNum);
        }
        else { //Если устройство есть, то нужно посмотреть его номер и писать потом в него
            *strDev = strDevices.firstChildElement("device");
            for (int i = 0; i < devList.size(); i++)
            {
                if (strDevices.attributeNS("parameter", "devnum") == bitMask.devNum) {
                    break;
                }
                *strDev = strDevices.nextSiblingElement();
            }
        }

        QDomElement strParameter = doc.createElementNS("parameter", QString::number(bitMask.id));
        strDevices.elementsByTagNameNS("device", QString::number(bitMask.devNum)).item(0).toElement().appendChild(strParameter);
        strDev->appendChild(strParameter);
        strParameter.setAttribute("devnum", bitMask.devNum);
        strParameter.setAttribute("startbyte", bitMask.byteNum);
        strParameter.setAttribute("name", bitMask.paramName);
        strParameter.setAttribute("id", bitMask.id);
        strParameter.setAttribute("binarymask", bitMask.paramMask);
        strParameter.setAttribute("valueshift", bitMask.valueShift);
        strParameter.setAttribute("valuekoef", bitMask.valueKoef);
        strParameter.setAttribute("name", bitMask.paramName);
        strParameter.setAttribute("viewinlog", bitMask.viewInLogFlag ? "true" : "false");
        strParameter.setAttribute("wordlenght", bitMask.wordType);
        strParameter.setAttribute("color", bitMask.drawGraphColor);
        strParameter.setAttribute("drawongraph", bitMask.drawGraphFlag ? "true" : "false");


        if(!profile.open(QIODevice::Truncate | QIODevice::WriteOnly))
        {
            QMessageBox :: about (NULL, tr ("Ошибка"), tr ("Не удалось сгенерировать файл конфигурации, повторите попытку!"));
            return;
        }
        QTextStream out(&profile);
        doc.save(out, 4, QDomNode::EncodingFromDocument);
        profile.close();
    }
}
void newconnect::saveProfile()
{
    //тут переписать на запись xml в файл уже после формирования в памяти
    if (permission2SaveMasks)
    {
//        maskVectorsList = this->findChildren<bitMaskDataStruct*>();
//        QListIterator<bitMaskDataStruct*> maskVectorsListIt(maskVectorsList);
//        maskVectorsListIt.toFront();
//        const SettingsDialog::Settings p = m_settings->settings();
//        QFile profile(p.profilePath);
//        QFileInfo info(profile);
//        profile.open(QIODevice::WriteOnly | QIODevice::Text);
//        QTextStream txtStream(&profile);
//        txtStream << info.fileName() << "\n";
//        while (maskVectorsListIt.hasNext())
//        {
//            QListIterator<QString> lstIt(maskVectorsListIt.peekNext()->lst);
//            while (lstIt.hasNext()) {
//                txtStream << lstIt.next() << "\t";
//            }
//            txtStream << "\n";
//            maskVectorsListIt.next();
//        }
//        permission2SaveMasks = false;
//        emit sendStatusStr("Profile " + info.fileName() + " saved");
    }
}
/*
<profile>
    <name></name>
    <description></description>
    <devices>
        <device>
            <parameter>
                <devNum></devNum>
                <devName></devName>
                <byteNum></byteNum>
                <byteName></byteName>
                <id></id>
                <wordType></wordType>
                <wordData></wordData>
                <paramName></paramName>
                <paramMask></paramMask>
                <paramType></paramType>
                <valueShift></valueShift>
                <valueKoef></valueKoef>
                <viewInLogFlag></viewInLogFlag>
                <drawGraphFlag></drawGraphFlag>
                <drawGraphColor></drawGraphColor>
                <isNewData></isNewData>
                <binRawValue></binRawValue>
                <endValue></endValue>
            </parameter>
        </device>
    </devices>
</profile>
*/
void newconnect::readProfile()
{
    emit cleanDevListSig();
    const SettingsDialog::Settings p = m_settings->settings();
    QFile profile(p.profilePath);
    if (!profile.exists() || !profile.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "Check your file";
        return;
    }
    QFileInfo info(profile);
    currentProfileName = getProfileNameFromInfo(info);
    emit profileName2log(currentProfileName);
    QDomDocument domDocument;
    domDocument.setContent(&profile);
    QDomElement topElement = domDocument.documentElement();
    QDomNode domNode = topElement.firstChild();
    while (!domNode.isNull()) {
        QDomElement domElement = domNode.toElement();
        if (!domElement.isNull()) {
            if (domElement.tagName() == "profile") {
                QDomNode node = domElement.firstChild();
                while (!node.isNull()) {
                    QDomElement element = node.toElement();
                    if (!element.isNull()) {
                        const QString tagName(element.tagName());
                        if (tagName == "devices") {
                            QDomNode devNode = element.firstChild();
                            while (!devNode.isNull()) {
                                QDomElement devElement = devNode.toElement();
                                if (!devElement.isNull()) {
                                    if (devElement.tagName() == "device") {
                                        bitMaskDataStruct loadMaskFromFile;
                                        QDomNode parameterNode = element.firstChild();
                                        while (!parameterNode.isNull())
                                        {
                                            QDomElement parameterElement = parameterNode.toElement();
                                            if (!parameterElement.isNull() && parameterElement.tagName() == "parameter") {

                                                loadMaskFromFile.devNum = devElement.attributeNode("num").value().toInt();
                                                loadMaskFromFile.devName = devElement.attributeNode("name").value();
                                                loadMaskFromFile.byteNum = parameterElement.attributeNode("startbyte").value().toInt();
                                                //loadMaskFromFile.byteName = parameterElement.attributeNode("name").value();
                                                loadMaskFromFile.id = parameterElement.attributeNode("id").value().toInt();
                                                loadMaskFromFile.wordType = parameterElement.attributeNode("wordlenght").value().toInt();
                                                //loadMaskFromFile.wordData = parameterElement.attributeNode("name").value().toInt();
                                                loadMaskFromFile.paramName = parameterElement.attributeNode("name").value();
                                                loadMaskFromFile.paramMask = parameterElement.attributeNode("binarymask").value();
                                                //loadMaskFromFile.paramType = parameterElement.attributeNode("name").value().toInt();
                                                loadMaskFromFile.valueShift = parameterElement.attributeNode("valueshift").value().toDouble();
                                                loadMaskFromFile.valueKoef = parameterElement.attributeNode("valuekoef").value().toDouble();
                                                loadMaskFromFile.viewInLogFlag = parameterElement.attributeNode("viewinlog").value().compare("true") ? true : false;
                                                loadMaskFromFile.drawGraphFlag = parameterElement.attributeNode("drawongraph").value().compare("true") ? true : false;
                                                loadMaskFromFile.drawGraphColor = parameterElement.attributeNode("color").value();
                                                //loadMaskFromFile.isNewData = parameterElement.attributeNode("name").value().compare("true") ? true : false;
                                                //loadMaskFromFile.binRawValue = parameterElement.attributeNode("name").value().toInt();
                                                //loadMaskFromFile.endValue = parameterElement.attributeNode("name").value().toDouble();
                                            }
                                        }
                                        emit loadMask(loadMaskFromFile);
                                    }
                                }
                            }
                            devNode = devNode.nextSibling();
                        }
                    }
                }
                node = node.nextSibling();
            }
        }
    }
    domNode = domNode.nextSibling();
}
void newconnect::resizeEvent(QResizeEvent * event)
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
QString newconnect::getProfileNameFromInfo(QFileInfo & info)
{
    return (info.fileName());
}

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
#include "profiledata.h"
#include <QCoreApplication>
#include <QEventLoop>
#include <global.h>

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
    //Кнопки подключения/настроек перенесены в строку состояния главного окна
    ui->connectButton->hide();
    ui->settingsButton->hide();
    m_console->setEnabled(false);
    m_console->setParent(ui->consoleFrame);
    m_console->setPlainText(tr("Waiting for connection")); //надпись в терминале до прихода первых данных
    m_console->show();
    datapool->setModel(&m_settings->model()); //разбор читает протокол и настройки из модели профиля
    wireConnection();
    wireSettings();
    wireProfile();
}

void newconnect::wireConnection()
{ //порт, консоль, поток байт и разбор кадров
    connect(m_serial, &QSerialPort::errorOccurred, this, &newconnect::handleError);
    connect(m_serial, &QSerialPort::readyRead, this, &newconnect::readData);
    connect(m_console, &Console::getData, this, &newconnect::writeData);
    connect(gstream, &getStream::giveMyByte, datapool, &dataprofiler::getByte);
    connect(this, &newconnect::pushByteToProfiler, datapool, &dataprofiler::getByte);
    connect(this, &newconnect::setTime, datapool, &dataprofiler::setTime);
    connect(datapool, &dataprofiler::deviceData, this, &newconnect::transmitData);
    connect(datapool, &dataprofiler::deviceData, this, [this]() {
        timerAboveTxCommand->start(1);
    });/*После успешного приёма задержка перед отправкой команды */
    connect(datapool, &dataprofiler::badCRC, this, &newconnect::badCRC);
    connect(datapool, &dataprofiler::ready4read, gstream, &getStream::readPermission);
    connect(datapool, &dataprofiler::readNext, gstream, &getStream::readIntByte);
    connect(this, &newconnect::sendRawData, gstream, &getStream::getRawData);
    connect(this, &newconnect::sendRawData, m_console, &Console::putData);
    connect(this, &newconnect::putIntDataToConsole, m_console, &Console::putIntData);
}

void newconnect::wireSettings()
{ //обмен протоколом и настройками между редактором профиля, разбором и сохранением
    connect(m_settings, &SettingsDialog::restoreConsoleAndButtons, this, &newconnect::restoreWindowAfterApplySettings);
    connect(m_settings, &SettingsDialog::prepareToSaveProfile, this, &newconnect::prepareToSaveProfile);
    connect(m_settings, &SettingsDialog::saveProfile, this, &newconnect::saveProfile);
    connect(m_settings, &SettingsDialog::writeTextLog, this, &newconnect::writeTextLog);
    connect(m_settings, &SettingsDialog::writeBinLog, this, &newconnect::writeBinLog);
    connect(m_settings, &SettingsDialog::writeJsonLog, this, &newconnect::writeJsonLog);
    //При загрузке данных из профиля они попадают в модель (SettingsDialog::loadProtocol),
    //из которой протокол и настройки читают разбор, устройства и логгер
    connect(this, &newconnect::loadProtocol, m_settings, &SettingsDialog::loadProtocol);
    connect(this, &newconnect::loadProtocol, this, &newconnect::setProtocol);
    connect(this, &newconnect::loadProtocol, this, [this](s_protocolDescription p) {
        emit setVisibleControlWindow(p.varControl); //видимость окна управления берём из профиля
    });
    connect(this, &newconnect::loadSettings, m_settings, &SettingsDialog::applyConnectionSettings);
    //По применению настроек в редакторе они оказываются в модели, поэтому разбору и
    //логгеру пересылать ничего не нужно - они читают ту же модель
    connect(m_settings, &SettingsDialog::setProtocol, this, &newconnect::setProtocol);
    connect(m_settings, &SettingsDialog::setProtocol, this, [this](s_protocolDescription p) {
        emit setVisibleControlWindow(p.varControl); //окно управления переменными видно только при включённом контроле
        emit connectButtonTextChanged(m_settings->settings().readFromFileFlag ? tr("Read log") : tr("Connect"));
    });
}

void newconnect::wireProfile()
{ //загрузка профиля и отправка команд устройству
    connect(m_settings, &SettingsDialog::loadSelectedProfile, this, &newconnect::readProfile);
    connect(timerAboveTxCommand, &QTimer::timeout, this, &newconnect::sendCommand);//отправляем команду после задержки
    connect(this, &newconnect::sendRawDataWithTime, this, &newconnect::readFromFile);
}

newconnect::~newconnect()
{
    delete m_serial;
    delete ui;
}

void newconnect::on_settingsButton_clicked()
{
    editProfile();
}

void newconnect::editProfile()
{ //заполняем редактор профиля данными текущего профиля; показом управляет главное окно
    readProfile();
}

void newconnect::openSerialPort()
{
    const s_Settings p_local = m_settings->currentSettings(); //снимок настроек на момент открытия порта
    if (!p_local.readFromFileFlag)
    {
        m_serial->setPortName(p_local.name);
        m_serial->setBaudRate(p_local.baudRate);
        m_serial->setDataBits(p_local.dataBits);
        m_serial->setParity(p_local.parity);
        m_serial->setStopBits(p_local.stopBits);
        m_serial->setFlowControl(p_local.flowControl);
        if (m_serial->open(QIODevice::ReadWrite)) {
            m_console->setEnabled(true);
            showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6, %7")
                              .arg(p_local.name).arg(p_local.stringBaudRate).arg(p_local.stringDataBits)
                              .arg(p_local.stringParity).arg(p_local.stringStopBits).arg(p_local.stringFlowControl)
                              .arg(QFileInfo(p_local.profilePath).fileName())); //в статусе только имя профиля, без пути
        }
        else {
            QMessageBox::critical(this, tr("Error"), m_serial->errorString());
            showStatusMessage(tr("Open error"));
        }
    }
}

void newconnect::readFromFile(QMap<QDateTime, QVector<uint8_t> > dataWithTime)
/*
* Функция принимает весь лог с временными метками, держит его в памяти и разом
* прогоняет через разбор, без порционной выдачи по времени.
* Перед разбором каждого блока ему выставляется время из лога, поэтому метки кадров
* и навигация по графику используют время лога, а не системные часы.
*/
{ // Весь лог кладём в память
    p_dataWithTime = dataWithTime;
    const QList<QDateTime> logTimes = p_dataWithTime.keys();
    const int blocksCount = logTimes.size();
    const int step = qMax(1, blocksCount / 100); //обновляем прогресс примерно 100 раз
    for (int block = 0; block < blocksCount; ++block) {
        const QDateTime logTime = logTimes.at(block);
        const QVector<uint8_t> data = p_dataWithTime.value(logTime);
        emit setTime(logTime); //время из лога
        emit putIntDataToConsole(data);
        for (const uint8_t byte : data) {
            emit pushByteToProfiler(byte);
        }
        if (block % step == 0) { //вторая половина прогресса - разбор лога
            emit logLoadProgress(50 + (blocksCount ? (50 * (block + 1)) / blocksCount : 50));
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        }
    }
    emit logLoadProgress(100);
    showStatusMessage(tr("End of file"));
}

void newconnect::closeSerialPort()
{
    if (m_serial->isOpen())
    {
        m_serial->close();
        showStatusMessage(tr("Disconnected"));
    }
}

void newconnect::writeData(const QByteArray &data)
{
    m_serial->write(data);
}

void newconnect::readData()
{
    static QByteArray data;
    data = m_serial->readAll();
    emit sendRawData(data);
    data.clear();
}

void newconnect::receiveCommandFromGui(QVector<quint8> command, bool newcommandflag)
{ // Тут принимаем команду сформированную в GUI
    toTransmit = command;
    newcommand = newcommandflag;
}

void newconnect::sendCommand()
{ // Отправляем команду контроллеру по сигналу после приёма
    if (m_serial->isOpen() && newcommand) {
        //if (!newcommand) toTransmit = {0xFF, 0xAB, 0, 0, 0};
        toTransmit.last() = calcCrc(toTransmit);
        QByteArray ba;
        for (auto i : std::as_const(toTransmit)) {
            ba.append(i);
        }
        writeData(ba);
        if (newcommand) {
            newcommand = false;
        }
    }
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
    toggleConnection();
}

void newconnect::toggleConnection()
{
    const s_Settings p_local = m_settings->currentSettings();
    if (!p_local.readFromFileFlag)
    {
        if (m_serial->isOpen())
        {
            this->closeSerialPort();
            if (!(m_serial->isOpen()))
            {
                offerToSaveProfile(); //по завершении соединения предлагаем сохранить изменения профиля
                emit connectButtonTextChanged(tr("Connect"));
                showStatusMessage(tr("Connection closed"));
                emit disconnected();
            }
        }
        else if (!(m_serial->isOpen()))
        {
            openSerialPort();
            if (m_serial->isOpen())
            {
                emit readFromFileSignal(false); //живое соединение - график идёт по реальному времени
                emit connected();
                emit connectButtonTextChanged(tr("Disconnect"));
            }
        }
    }
    else {
        if (!readerBusy) {
            showStatusMessage(tr("Read data log from file..."));
            readerBusy = true; //весь лог читается и разбирается разом внутри emit ниже
            emit connected();
            emit readFromFileSignal(p_local.readFromFileFlag);
            readerBusy = false; //чтение синхронное - сразу завершаем сессию
            emit connectButtonTextChanged(tr("Read log"));
            emit disconnected(); //соединение закрывается автоматически после чтения лога
            offerToSaveProfile(); //если профиль менялся - предложим сохранить
        }
    }
}

void newconnect::prepareToSaveProfile()
{ //очищаем список масок, разрешаем сохранение и запрашиваем у устройств все маски
    maskVectorsList = this->findChildren<txtmaskobj*>();
    QListIterator<txtmaskobj*> maskVectorsListIt(maskVectorsList);
    maskVectorsListIt.toFront();
    while (maskVectorsListIt.hasNext()) {
        delete maskVectorsListIt.next();
    }
    permission2SaveMasks = true;
    emit saveAllMasks();
}

void newconnect::saveProfileSlot4Masks(s_parameterMask mask)
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
            if ((QString::number(mask.id, 10) == maskVectorsListIt.peekNext()->lst.at(1)) &&
                    (QString::number(mask.devNum, 10) == maskVectorsListIt.peekNext()->lst.at(2)) &&
                    (QString::number(mask.byteNum, 10) == maskVectorsListIt.peekNext()->lst.at(3)) &&
                    (maskVectorsListIt.peekNext()->lst.at(0) == "thisIsMask")) {
                thisMaskHere = true;
            }
            maskVectorsListIt.next();
        }
        if (!thisMaskHere)
        {
            QList<QString> maskList;
            maskList.append("thisIsMask");//0
            maskList.append(QString::number(mask.id, 10)); //1
            maskList.append(QString::number(mask.devNum, 10)); //2
            maskList.append(QString::number(mask.byteNum, 10)); //3
            maskList.append(mask.devName);//4
            maskList.append(mask.byteName);//5
            maskList.append(mask.parameterName);//6
            maskList.append(mask.parameterMask);//7
            maskList.append(QString::number(mask.valueShift, 'g', 6)); //8
            maskList.append(QString::number(mask.valueKoef, 'g', 6)); //9
            maskList.append((mask.viewInLogFlag ? "true" : "false")); //10
            maskList.append(QString::number(mask.wordType));//11
            maskList.append(mask.drawGraphFlag ? "true" : "false"); //12
            maskList.append(mask.drawGraphColor);//13
            txtmaskobj *savingMask = new txtmaskobj(maskList);
            savingMask->setParent(this);
            maskList.clear();
        }
    }
}

void newconnect::saveProfile()
{ //Изменения профиля пишем в файл <профиль>.eag.tmp; сам профиль заменяем только
  //по подтверждению при завершении соединения или закрытии программы.
    if (!permission2SaveMasks) {
        return;
    }
    maskVectorsList = this->findChildren<txtmaskobj*>();
    QListIterator<txtmaskobj*> maskVectorsListIt(maskVectorsList);
    maskVectorsListIt.toFront();
    const s_Settings p = m_settings->settings();
    if (p.profilePath.isEmpty()) {
        permission2SaveMasks = false;
        return;
    }
    QString content = profiledata::serializeHeader(QFileInfo(p.profilePath).fileName(), m_settings->model().protocol(), p);
    while (maskVectorsListIt.hasNext())
    {
        content += profiledata::serializeMaskLine(maskVectorsListIt.peekNext()->lst);
        maskVectorsListIt.next();
    }
    permission2SaveMasks = false;

    //Сравниваем с уже сохранённым (tmp или самим профилем), чтобы не помечать профиль изменённым зря
    const QString tmpPath = p.profilePath + ".tmp";
    QString existing;
    QFile existingFile(tmpPath);
    if (!existingFile.exists()) {
        existingFile.setFileName(p.profilePath);
    }
    if (existingFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        existing = QString::fromUtf8(existingFile.readAll());
        existingFile.close();
    }
    if (existing == content) {
        return; //никаких изменений
    }
    QFile out(tmpPath);
    if (out.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        out.write(content.toUtf8());
        out.close();
        profileModified = true;
        emit sendStatusStr(tr("Profile changes saved to ") + QFileInfo(tmpPath).fileName());
    }
}

void newconnect::commitProfileChanges()
{ //подтверждено: старый профиль -> .bak, временный -> профиль
    const QString path = m_settings->settings().profilePath;
    if (path.isEmpty()) {
        return;
    }
    const QString tmpPath = path + ".tmp";
    if (!QFile::exists(tmpPath)) {
        profileModified = false;
        return;
    }
    const QString bakPath = path + ".bak";
    if (QFile::exists(bakPath)) {
        QFile::remove(bakPath);
    }
    if (QFile::exists(path)) {
        QFile::rename(path, bakPath);
    }
    QFile::rename(tmpPath, path);
    profileModified = false;
    emit sendStatusStr(tr("Profile saved: ") + QFileInfo(path).fileName());
}

void newconnect::discardProfileChanges()
{ //отказ от сохранения - удаляем временный файл
    const QString path = m_settings->settings().profilePath;
    if (!path.isEmpty()) {
        QFile::remove(path + ".tmp");
    }
    profileModified = false;
}

void newconnect::offerToSaveProfile()
{ //предлагаем сохранить изменённый профиль (при разрыве соединения или выходе)
    if (!profileModified) {
        return;
    }
    const QString name = QFileInfo(m_settings->settings().profilePath).fileName();
    if (name.isEmpty()) {
        profileModified = false;
        return;
    }
    const QMessageBox::StandardButton answer = QMessageBox::question(this, tr("Profile changed"),
            tr("Profile %1 has been changed.\nSave the changes?").arg(name),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (answer == QMessageBox::Yes) {
        commitProfileChanges();
    }
    else {
        discardProfileChanges();
    }
}

void newconnect::readProfile()
{
    emit cleanDevListSig();
    const s_Settings p = m_settings->settings();
    QFile profile(p.profilePath);
    QFileInfo info(profile);
    currentProfileName = getProfileNameFromInfo(info);
    emit profileName2log(currentProfileName);
    QString text;
    if (profile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream txtStream(&profile);
        text = txtStream.readAll();
        profile.close();
    }
    //Файл разбирается целиком: пустые, неполные и неизвестные строки пропускаются внутри parse
    const profiledata::ProfileText parsed = profiledata::parse(text);
    if (parsed.hasSettings) {
        emit loadSettings(parsed.settings); //применяем настройки связи из профиля
    }
    emit loadProtocol(parsed.protocol);
    //Протокол применяется раньше масок, поэтому маски разбираем после его загрузки
    for (const s_parameterMask &mask : std::as_const(parsed.masks)) {
        emit loadMask(mask);
    }
    emit profileLoaded();
}

void newconnect::resizeEvent(QResizeEvent *event)
{
    if (event)
    {
        m_console->resize(event->size());
    }
}

void newconnect::restoreWindowAfterApplySettings()
{ //кнопки подключения/настроек теперь живут в строке состояния, поэтому не показываем их здесь
    m_console->show();
    ui->consoleFrame->show();
}

QString newconnect::getProfileNameFromInfo(QFileInfo& info)
{
    return (info.fileName());
}

quint8 newconnect::calcCrc(const QVector<quint8> &arr)
{
    quint8 crc = 0;
    for (int i = 2; i < arr.size() - 1; i++) {
        crc += arr[i];
    }
    return crc;
}

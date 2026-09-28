#include "settingsdialog.h"
#include "apppaths.h"
#include "ui_settingsdialog.h"
#include <QLineEdit>
#include <QSerialPortInfo>
#include <QDir>
#include <QFileDialog>
#include <QDebug>
#include <QInputDialog>
#include <QFileInfo>
#include <QMessageBox>
#include "global.h"

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

SettingsDialog::SettingsDialog(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::SettingsDialog)
{
    m_ui->setupUi(this);
    connect(m_ui->applyButton, &QPushButton::clicked, this, &SettingsDialog::apply);
    connect(this, &SettingsDialog::loadProtocol, this, [this](s_protocolDescription p) { //Заполнение полей выбранным протоколом
        m_ui->packetSizeSpinBox->setValue(p.packetSize);
        m_ui->BlockIdentifycatorPositionSpinBox->setValue(p.blockIdentifycatorPosition);
        m_ui->calcCRCFromSpinBox->setValue(p.calcCRCFromPosition);
        m_ui->markerSizeSpinBox->setValue(p.markerPacketBeginSize);
        m_ui->b1MarkerLineEdit->setText(QString::number(p.markerPacketBeginByte1, 16).toUpper());
        m_ui->b2MarkerLineEdit->setText(QString::number(p.markerPacketBeginByte2, 16).toUpper());
        m_ui->descriptionTextEdit->setText(p.description);
        m_ui->varConrolCheckBox->setChecked(p.varControl);
        //В модель кладём именно протокол из профиля, а не значения из полей:
        //спинбоксы ограничивают диапазоны и могли бы испортить загруженное значение
        m_model.setProtocol(p);
    } );
    connect(m_ui->b1MarkerLineEdit, &QLineEdit::textEdited, this, [this](QString text) {
        markerTextNormalisation(1, text);
    } );
    connect(m_ui->b2MarkerLineEdit, &QLineEdit::textEdited, this, [this](QString text) {
        markerTextNormalisation(2, text);
    });
    selectFirstProfile();
}

SettingsDialog::~SettingsDialog()
{
    delete m_ui;
}

s_Settings SettingsDialog::settings() const
{
    //Путь к профилю и режим чтения из файла - состояние окна, а не модели:
    //подмешиваем их к сохранённым настройкам связи
    s_Settings s = m_model.settings();
    s.profilePath = selectedProfile;
    s.readFromFileFlag = m_readFromFileMode;
    return s;
}

s_Settings SettingsDialog::currentSettings()
{
    return settings();
}

QString SettingsDialog::connectionSummary() const
{
    const s_Settings &s = m_model.settings();
    QString parityLetter = QStringLiteral("N");
    if (s.parity == QSerialPort::EvenParity) {
        parityLetter = QStringLiteral("E");
    }
    else if (s.parity == QSerialPort::OddParity) {
        parityLetter = QStringLiteral("O");
    }
    else if (s.parity == QSerialPort::MarkParity) {
        parityLetter = QStringLiteral("M");
    }
    else if (s.parity == QSerialPort::SpaceParity) {
        parityLetter = QStringLiteral("S");
    }
    return s.stringBaudRate + " " + s.stringDataBits
           + parityLetter + s.stringStopBits;
}

bool SettingsDialog::isReadFromFile() const
{
    return m_readFromFileMode;
}

QString SettingsDialog::selectedPortName() const
{
    return m_model.settings().name;
}

QStringList SettingsDialog::availablePortNames() const
{
    QStringList ports;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        ports << info.portName();
    }
    return ports;
}

QStringList SettingsDialog::profileNames() const
{
    QStringList profileList;
    QDir dir(apppaths::profilesDir());
    if (!dir.exists()) {
        return profileList;
    }
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Name);
    QStringList filters;
    filters << "*.eag";
    dir.setNameFilters(filters);
    const QFileInfoList list = dir.entryInfoList();
    for (const QFileInfo &fileInfo : list) {
        profileList << fileInfo.fileName();
    }
    return profileList;
}

QString SettingsDialog::currentProfileName() const
{
    return QFileInfo(selectedProfile).fileName();
}

void SettingsDialog::selectFirstProfile()
{ //при старте подхватываем первый профиль каталога (без выпадающего списка)
    const QStringList names = profileNames();
    if (names.isEmpty()) {
        return;
    }
    QDir dir(apppaths::profilesDir());
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Name);
    QStringList filter;
    filter << names.first();
    dir.setNameFilters(filter);
    const QFileInfoList list = dir.entryInfoList();
    if (!list.isEmpty()) {
        selectedProfile = list.at(0).filePath();
        m_model.mutableSettings().profilePath = selectedProfile;
    }
}

void SettingsDialog::selectProfile(const QString &fileName)
{
    if (fileName.isEmpty()) {
        return;
    }
    QDir dir(apppaths::profilesDir());
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    QStringList filter;
    filter << fileName;
    dir.setNameFilters(filter);
    const QFileInfoList list = dir.entryInfoList();
    if (list.isEmpty()) {
        return;
    }
    selectedProfile = list.at(0).filePath();
    m_model.mutableSettings().profilePath = selectedProfile;
    emit loadSelectedProfile(); //профиль выбран - загружаем его
}

void SettingsDialog::createNewProfile()
{ //диалог создания нового профиля, затем сразу выбираем его
#ifdef Q_OS_WIN32
    QString fileName = QFileDialog::getSaveFileName(this, tr("newprofile"), apppaths::profilesDir(), "Etrodiag devices profile(*.eag)");
#endif
#ifdef Q_OS_ANDROID
    QString fileName = apppaths::profilesDir() + QDir::separator() + QInputDialog::getText(this, tr("Enter profile name"), tr("Enter profile name"), QLineEdit::Normal, "", &ok);
#endif
    if (fileName.isEmpty()) {
        return;
    }
    if (!fileName.endsWith("eag")) {
        fileName = fileName + ".eag";
    }
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    file.close();
    selectProfile(QFileInfo(fileName).fileName());
}

void SettingsDialog::deleteCurrentProfile()
{
    if (selectedProfile.isEmpty()) {
        return;
    }
    const QMessageBox::StandardButton answer = QMessageBox::question(this, tr("Delete profile"),
            tr("Delete profile %1?").arg(QFileInfo(selectedProfile).fileName()),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) {
        return; //отказ от удаления
    }
    QFile profile(selectedProfile);
    if (profile.exists()) {
        profile.remove();
    }
    QFile::remove(selectedProfile + ".tmp"); //удаляем и временный/резервный файлы профиля
    QFile::remove(selectedProfile + ".bak");
    selectedProfile.clear();
    m_model.mutableSettings().profilePath.clear();
}

bool SettingsDialog::writeTxtEnabled() const
{
    return m_writeTxt;
}

bool SettingsDialog::writeBinEnabled() const
{
    return m_writeBin;
}

bool SettingsDialog::writeJsonEnabled() const
{
    return m_writeJson;
}

void SettingsDialog::setWriteTxt(bool on)
{
    if (m_writeTxt == on) {
        return;
    }
    m_writeTxt = on;
    emit writeTextLog(on);
}

void SettingsDialog::setWriteBin(bool on)
{
    if (m_writeBin == on) {
        return;
    }
    m_writeBin = on;
    emit writeBinLog(on);
}

void SettingsDialog::setWriteJson(bool on)
{
    if (m_writeJson == on) {
        return;
    }
    m_writeJson = on;
    emit writeJsonLog(on);
}

void SettingsDialog::setPortName(const QString &portName)
{
    m_model.mutableSettings().name = portName;
    m_model.mutableSettings().readFromFileFlag = false;
    m_model.mutableSettings().pathToBinFile.clear();
    m_readFromFileMode = false;
    emit settingsChanged();
}

void SettingsDialog::setReadFromFile(const QString &filePath)
{
    m_model.mutableSettings().name = filePath;
    m_model.mutableSettings().pathToBinFile = filePath;
    m_model.mutableSettings().readFromFileFlag = true;
    m_readFromFileMode = true;
    emit settingsChanged();
}

void SettingsDialog::applyConnection(int baud, int dataBits, int parity, int stopBits, int flowControl)
{
    m_model.mutableSettings().baudRate = baud;
    m_model.mutableSettings().stringBaudRate = QString::number(baud);
    m_model.mutableSettings().dataBits = static_cast<QSerialPort::DataBits>(dataBits);
    m_model.mutableSettings().stringDataBits = QString::number(dataBits);
    m_model.mutableSettings().parity = static_cast<QSerialPort::Parity>(parity);
    m_model.mutableSettings().stringParity = (parity == QSerialPort::EvenParity ? QStringLiteral("Even")
                                      : parity == QSerialPort::OddParity ? QStringLiteral("Odd")
                                      : parity == QSerialPort::MarkParity ? QStringLiteral("Mark")
                                      : parity == QSerialPort::SpaceParity ? QStringLiteral("Space")
                                      : QStringLiteral("None"));
    m_model.mutableSettings().stopBits = static_cast<QSerialPort::StopBits>(stopBits);
    m_model.mutableSettings().stringStopBits = (stopBits == QSerialPort::TwoStop ? QStringLiteral("2")
                                       : stopBits == QSerialPort::OneAndHalfStop ? QStringLiteral("1.5")
                                       : QStringLiteral("1"));
    m_model.mutableSettings().flowControl = static_cast<QSerialPort::FlowControl>(flowControl);
    m_model.mutableSettings().stringFlowControl = (flowControl == QSerialPort::HardwareControl ? QStringLiteral("RTS/CTS")
                                          : flowControl == QSerialPort::SoftwareControl ? QStringLiteral("XON/XOFF")
                                          : QStringLiteral("None"));
    emit settingsChanged();
}

void SettingsDialog::applyConnectionSettings(const s_Settings &s)
{
    //Из профиля применяем только параметры связи.
    //COM-порт и режим "чтение из файла" не трогаем: порт всегда выбирается вручную.
    applyConnection(s.baudRate, static_cast<int>(s.dataBits), static_cast<int>(s.parity),
                    static_cast<int>(s.stopBits), static_cast<int>(s.flowControl));
    emit settingsChanged();
}

void SettingsDialog::apply()
{ //сохраняем профиль; показом правой половины вкладки управляет главное окно
    updateProtocol();
    emit setProtocol(m_model.protocol());
    emit prepareToSaveProfile();
    emit saveProfile();
    emit restoreConsoleAndButtons();
}

void SettingsDialog::markerTextNormalisation(int numberByte, QString text)
{
    bool ok;
    int val = text.toInt(&ok, 16);
    if (val < 0) {
        val = 0;
    }
    else if (val > 0xFF) {
        val = 0xFF;
    }
    if (numberByte == 1) {
        m_ui->b1MarkerLineEdit->setText(QString::number(val, 16).toUpper());
    }
    else if (numberByte == 2) {
        m_ui->b2MarkerLineEdit->setText(QString::number(val, 16).toUpper());
    }
}

void SettingsDialog::updateProtocol()
{
    s_protocolDescription p;
    p.packetSize = m_ui->packetSizeSpinBox->value();
    p.blockIdentifycatorPosition = m_ui->BlockIdentifycatorPositionSpinBox->value();
    p.calcCRCFromPosition = m_ui->calcCRCFromSpinBox->value();
    p.markerPacketBeginSize = m_ui->markerSizeSpinBox->value();
    p.markerPacketBeginByte1 = QString(m_ui->b1MarkerLineEdit->text()).toInt(0, 16);
    p.markerPacketBeginByte2 = QString(m_ui->b2MarkerLineEdit->text()).toInt(0, 16);
    p.description = m_ui->descriptionTextEdit->toPlainText();
    p.varControl = m_ui->varConrolCheckBox->checkState() ? true : false;
    m_model.setProtocol(p);
}

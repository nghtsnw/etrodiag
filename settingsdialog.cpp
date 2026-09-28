#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QApplication>
#include <QLineEdit>
#include <QSerialPortInfo>
#include <QDir>
#include <QFileDialog>
#include <QDebug>
#include <QStandardPaths>
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
#ifdef Q_OS_WIN32
    appHomeDir = qApp->applicationDirPath() + QDir::separator();
#endif
#ifdef Q_OS_ANDROID
    appHomeDir = QStandardPaths::standardLocations(QStandardPaths::DataLocation)[1] + QDir::separator();
#endif
    connect(m_ui->applyButton, &QPushButton::clicked, this, &SettingsDialog::apply);
    connect(this, &SettingsDialog::loadProtocol, this, [ = ](s_protocolDescription p) { //Заполнение полей выбранным протоколом
        m_ui->packetSizeSpinBox->setValue(p.packetSize);
        m_ui->BlockIdentifycatorPositionSpinBox->setValue(p.blockIdentifycatorPosition);
        m_ui->calcCRCFromSpinBox->setValue(p.calcCRCFromPosition);
        m_ui->markerSizeSpinBox->setValue(p.markerPacketBeginSize);
        m_ui->b1MarkerLineEdit->setText(QString::number(p.markerPacketBeginByte1, 16).toUpper());
        m_ui->b2MarkerLineEdit->setText(QString::number(p.markerPacketBeginByte2, 16).toUpper());
        m_ui->descriptionTextEdit->setText(p.description);
        m_ui->varConrolCheckBox->setChecked(p.varControl);
    } );
    connect(m_ui->b1MarkerLineEdit, &QLineEdit::textEdited, this, [ = ](QString text) {
        markerTextNormalisation(1, text);
    } );
    connect(m_ui->b2MarkerLineEdit, &QLineEdit::textEdited, this, [ = ](QString text) {
        markerTextNormalisation(2, text);
    });
    fillProfileList();
    updateSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete m_ui;
}

s_Settings SettingsDialog::settings() const
{
    return m_currentSettings;
}

s_Settings SettingsDialog::currentSettings()
{
    updateSettings();
    return m_currentSettings;
}

QString SettingsDialog::connectionSummary() const
{
    QString parityLetter = QStringLiteral("N");
    if (m_currentSettings.parity == QSerialPort::EvenParity) {
        parityLetter = QStringLiteral("E");
    }
    else if (m_currentSettings.parity == QSerialPort::OddParity) {
        parityLetter = QStringLiteral("O");
    }
    else if (m_currentSettings.parity == QSerialPort::MarkParity) {
        parityLetter = QStringLiteral("M");
    }
    else if (m_currentSettings.parity == QSerialPort::SpaceParity) {
        parityLetter = QStringLiteral("S");
    }
    return m_currentSettings.stringBaudRate + " " + m_currentSettings.stringDataBits
           + parityLetter + m_currentSettings.stringStopBits;
}

bool SettingsDialog::isReadFromFile() const
{
    return m_readFromFileMode;
}

QString SettingsDialog::selectedPortName() const
{
    return m_currentSettings.name;
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
    QDir dir(appHomeDir + "Profiles");
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
    return m_ui->profileSelectBox->currentText();
}

void SettingsDialog::selectProfile(const QString &fileName)
{
    if (fileName.isEmpty()) {
        return;
    }
    if (m_ui->profileSelectBox->findText(fileName) < 0) {
        m_ui->profileSelectBox->addItem(fileName);
    }
    if (m_ui->profileSelectBox->currentText() == fileName) {
        emit loadSelectedProfile(); //профиль уже выбран - всё равно перезагружаем его
    }
    else {
        m_ui->profileSelectBox->setCurrentText(fileName); //вызовет загрузку выбранного профиля
    }
}

void SettingsDialog::createNewProfile()
{
    on_newProfileButton_clicked();
}

void SettingsDialog::deleteCurrentProfile()
{
    on_deleteProfileButton_clicked();
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
    m_currentSettings.name = portName;
    m_currentSettings.readFromFileFlag = false;
    m_currentSettings.pathToBinFile.clear();
    m_readFromFileMode = false;
    emit settingsChanged();
}

void SettingsDialog::setReadFromFile(const QString &filePath)
{
    m_currentSettings.name = filePath;
    m_currentSettings.pathToBinFile = filePath;
    m_currentSettings.readFromFileFlag = true;
    m_readFromFileMode = true;
    emit settingsChanged();
}

void SettingsDialog::applyConnection(int baud, int dataBits, int parity, int stopBits, int flowControl)
{
    m_currentSettings.baudRate = baud;
    m_currentSettings.stringBaudRate = QString::number(baud);
    m_currentSettings.dataBits = static_cast<QSerialPort::DataBits>(dataBits);
    m_currentSettings.stringDataBits = QString::number(dataBits);
    m_currentSettings.parity = static_cast<QSerialPort::Parity>(parity);
    m_currentSettings.stringParity = (parity == QSerialPort::EvenParity ? QStringLiteral("Even")
                                      : parity == QSerialPort::OddParity ? QStringLiteral("Odd")
                                      : parity == QSerialPort::MarkParity ? QStringLiteral("Mark")
                                      : parity == QSerialPort::SpaceParity ? QStringLiteral("Space")
                                      : QStringLiteral("None"));
    m_currentSettings.stopBits = static_cast<QSerialPort::StopBits>(stopBits);
    m_currentSettings.stringStopBits = (stopBits == QSerialPort::TwoStop ? QStringLiteral("2")
                                       : stopBits == QSerialPort::OneAndHalfStop ? QStringLiteral("1.5")
                                       : QStringLiteral("1"));
    m_currentSettings.flowControl = static_cast<QSerialPort::FlowControl>(flowControl);
    m_currentSettings.stringFlowControl = (flowControl == QSerialPort::HardwareControl ? QStringLiteral("RTS/CTS")
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
    updateSettings();
    emit settingsChanged();
}

void SettingsDialog::apply()
{
    updateSettings();
    updateProtocol();
    emit setProtocol(currentProtocol);
    this->hide();
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

void SettingsDialog::updateSettings()
{
    m_currentSettings.profilePath = selectedProfile;
    m_currentSettings.readFromFileFlag = m_readFromFileMode;
}

void SettingsDialog::updateProtocol()
{
    currentProtocol.packetSize = m_ui->packetSizeSpinBox->value();
    currentProtocol.blockIdentifycatorPosition = m_ui->BlockIdentifycatorPositionSpinBox->value();
    currentProtocol.calcCRCFromPosition = m_ui->calcCRCFromSpinBox->value();
    currentProtocol.markerPacketBeginSize = m_ui->markerSizeSpinBox->value();
    currentProtocol.markerPacketBeginByte1 = QString(m_ui->b1MarkerLineEdit->text()).toInt(0, 16);
    currentProtocol.markerPacketBeginByte2 = QString(m_ui->b2MarkerLineEdit->text()).toInt(0, 16);
    //currentProtocol.timeoutAfterLastByte = m_ui->timeoutSpinBox->value();
    currentProtocol.description = m_ui->descriptionTextEdit->toPlainText();
    currentProtocol.varControl = m_ui->varConrolCheckBox->checkState() ? true : false;
}

void SettingsDialog::fillProfileList()
{
    m_ui->profileSelectBox->clear();
    QStringList profileList;
    QDir dir(appHomeDir + "Profiles");
    if (!dir.exists()) {
        QDir().mkdir(appHomeDir + "Profiles");
    }
    bool ok = dir.exists();
    if (ok)
    {
        dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
        dir.setSorting(QDir::Name);
        QStringList filters;
        filters << "*.eag";
        dir.setNameFilters(filters);
        QFileInfoList list = dir.entryInfoList();
        for (int i = 0; i < list.size(); ++i)
        {
            QFileInfo fileInfo = list.at(i);
            profileList << fileInfo.fileName();
        }
        m_ui->profileSelectBox->addItems(profileList);
        if (list.size() == 0)
        {
            m_ui->applyButton->setDisabled(true);
        }
        else {
            m_ui->applyButton->setEnabled(true);
        }
    }
}

void SettingsDialog::on_newProfileButton_clicked()
{
#ifdef Q_OS_WIN32
    QString fileName = QFileDialog::getSaveFileName(this, tr("newprofile"), appHomeDir + "Profiles", "Etrodiag devices profile(*.eag)");
#endif
#ifdef Q_OS_ANDROID
    QString fileName = appHomeDir + "Profiles" + QDir::separator() + QInputDialog::getText(this, tr("Enter profile name"), tr("Enter profile name"), QLineEdit::Normal, "", &ok);
#endif
    if (!fileName.isEmpty())
    {
        if (!fileName.endsWith("eag")) {
            fileName = fileName + ".eag";
        }
        QFile file(fileName);
        file.open(QIODevice::WriteOnly);
        file.close();
        fillProfileList();
        m_ui->profileSelectBox->setCurrentText(QFileInfo(fileName).fileName()); //сразу выбираем созданный профиль
    }
}

void SettingsDialog::on_profileSelectBox_currentTextChanged(const QString & arg1)
{
    QDir profilesDir(appHomeDir + "Profiles");
    QStringList nameFilter;
    nameFilter << arg1;
    profilesDir.setNameFilters(nameFilter);
    QFileInfoList infoList(profilesDir.entryInfoList());
    if (infoList.size() > 0)
    {
        QFileInfo fileInfo(infoList.at(0));
        QString currentProfile = fileInfo.filePath();
        selectedProfile = currentProfile;
        m_currentSettings.profilePath = selectedProfile;
        m_ui->packetSizeSpinBox->clear();
        m_ui->markerSizeSpinBox->clear();
        //m_ui->timeoutSpinBox->clear();
        m_ui->BlockIdentifycatorPositionSpinBox->clear();
        m_ui->calcCRCFromSpinBox->clear();
        m_ui->b1MarkerLineEdit->clear();
        m_ui->b2MarkerLineEdit->clear();
        m_ui->descriptionTextEdit->clear();
        m_ui->varConrolCheckBox->setCheckState(Qt::Unchecked);
        nameFilter.clear();
        infoList.clear();
        emit loadSelectedProfile();
    }
}

void SettingsDialog::on_deleteProfileButton_clicked()
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
    fillProfileList();
}

#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QIntValidator>
#include <QLineEdit>
#include <QSerialPortInfo>
#include <QDir>
#include <QFileDialog>
#include <QDebug>
#include <QStandardPaths>
#include <QInputDialog>
#include <QFileInfo>
#include "global.h"

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

SettingsDialog::SettingsDialog(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::SettingsDialog),
    m_intValidator(new QIntValidator(0, 4000000, this))
{
    m_ui->setupUi(this);
    m_ui->baudRateBox->setInsertPolicy(QComboBox::NoInsert);
#ifdef Q_OS_WIN32
    appHomeDir = qApp->applicationDirPath() + QDir::separator();
#endif
#ifdef Q_OS_ANDROID
    appHomeDir = QStandardPaths::standardLocations(QStandardPaths::DataLocation)[1] + QDir::separator();
#endif
    connect(m_ui->applyButton, &QPushButton::clicked,
            this, &SettingsDialog::apply);
    connect(m_ui->serialPortInfoListBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::showPortInfo);
    connect(m_ui->baudRateBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::checkCustomBaudRatePolicy);
    connect(m_ui->serialPortInfoListBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::checkCustomDevicePathPolicy);
    connect(m_ui->serialPortInfoListBox, QOverload<int>::of(&QComboBox::activated), this, &SettingsDialog::portBoxEvent);
    connect(this, &SettingsDialog::loadProtocol, this, [ = ](s_protocolDescription p) { //Заполнение полей выбранным протоколом
        m_ui->packetSizeSpinBox->setValue(p.packetSize);
        m_ui->BlockIdentifycatorPositionSpinBox->setValue(p.blockIdentifycatorPosition);
        m_ui->calcCRCFromSpinBox->setValue(p.calcCRCFromPosition);
        m_ui->markerSizeSpinBox->setValue(p.markerPacketBeginSize);
        //m_ui->timeoutSpinBox->setValue(p.timeoutAfterLastByte);
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
    fillPortsParameters();
    fillProfileList();
    fillPortsInfo();
    m_ui->readOnlyCheckBox->setChecked(m_currentSettings.readOnlyProfile);
    on_readOnlyCheckBox_stateChanged(0);
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
    const QString parityText = m_ui->parityBox->currentText();
    if (parityText == tr("Even")) {
        parityLetter = QStringLiteral("E");
    }
    else if (parityText == tr("Odd")) {
        parityLetter = QStringLiteral("O");
    }
    else if (parityText == tr("Mark")) {
        parityLetter = QStringLiteral("M");
    }
    else if (parityText == tr("Space")) {
        parityLetter = QStringLiteral("S");
    }
    return m_ui->baudRateBox->currentText() + " " + m_ui->dataBitsBox->currentText()
           + parityLetter + m_ui->stopBitsBox->currentText();
}

bool SettingsDialog::isReadFromFile() const
{
    return m_currentSettings.readFromFileFlag;
}

QString SettingsDialog::selectedPortName() const
{
    return m_ui->serialPortInfoListBox->currentText();
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

void SettingsDialog::refreshPorts()
{ //перечитываем список портов, сохраняя текущий выбор (в режиме файла список не трогаем)
    if (m_currentSettings.readFromFileFlag) {
        return;
    }
    const QString current = m_ui->serialPortInfoListBox->currentText();
    fillPortsInfo();
    const int idx = m_ui->serialPortInfoListBox->findText(current);
    if (idx >= 0) {
        m_ui->serialPortInfoListBox->blockSignals(true);
        m_ui->serialPortInfoListBox->setCurrentIndex(idx);
        m_ui->serialPortInfoListBox->blockSignals(false);
    }
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
    fillProfileList();
    m_ui->profileSelectBox->setCurrentText(fileName); //вызовет загрузку выбранного профиля
}

void SettingsDialog::createNewProfile()
{
    on_newProfileButton_clicked();
}

bool SettingsDialog::readOnlyProfile() const
{
    return m_ui->readOnlyCheckBox->isChecked();
}

bool SettingsDialog::writeTxtEnabled() const
{
    return m_ui->writeTxtChkBox->isChecked();
}

bool SettingsDialog::writeBinEnabled() const
{
    return m_ui->writeBinChkBox->isChecked();
}

bool SettingsDialog::writeJsonEnabled() const
{
    return m_ui->writeJsonChkBox->isChecked();
}

void SettingsDialog::setWriteTxt(bool on)
{
    m_ui->writeTxtChkBox->setChecked(on);
}

void SettingsDialog::setWriteBin(bool on)
{
    m_ui->writeBinChkBox->setChecked(on);
}

void SettingsDialog::setWriteJson(bool on)
{
    m_ui->writeJsonChkBox->setChecked(on);
}

void SettingsDialog::setPortName(const QString &portName)
{
    m_ui->serialPortInfoListBox->blockSignals(true);
    m_ui->serialPortInfoListBox->setEditable(false);
    const int idx = m_ui->serialPortInfoListBox->findText(portName);
    if (idx >= 0) {
        m_ui->serialPortInfoListBox->setCurrentIndex(idx);
    }
    m_ui->serialPortInfoListBox->blockSignals(false);
    m_currentSettings.readFromFileFlag = false;
    m_currentSettings.pathToBinFile.clear();
    m_ui->parametersBox->setDisabled(false);
    updateSettings();
    emit settingsChanged();
}

void SettingsDialog::setReadFromFile(const QString &filePath)
{
    m_ui->serialPortInfoListBox->blockSignals(true);
    m_ui->serialPortInfoListBox->setEditable(true);
    m_ui->serialPortInfoListBox->setCurrentText(filePath);
    m_ui->serialPortInfoListBox->blockSignals(false);
    m_currentSettings.readFromFileFlag = true;
    m_currentSettings.pathToBinFile = filePath;
    m_ui->parametersBox->setDisabled(true);
    updateSettings();
    emit settingsChanged();
}

void SettingsDialog::applyConnection(int baud, int dataBits, int parity, int stopBits, int flowControl)
{
    int idx = m_ui->baudRateBox->findData(baud);
    if (idx >= 0) {
        m_ui->baudRateBox->setCurrentIndex(idx);
    }
    else {
        m_ui->baudRateBox->setCurrentIndex(m_ui->baudRateBox->count() - 1); //Custom
        m_ui->baudRateBox->setCurrentText(QString::number(baud));
    }
    idx = m_ui->dataBitsBox->findData(dataBits);
    if (idx >= 0) {
        m_ui->dataBitsBox->setCurrentIndex(idx);
    }
    idx = m_ui->parityBox->findData(parity);
    if (idx >= 0) {
        m_ui->parityBox->setCurrentIndex(idx);
    }
    idx = m_ui->stopBitsBox->findData(stopBits);
    if (idx >= 0) {
        m_ui->stopBitsBox->setCurrentIndex(idx);
    }
    idx = m_ui->flowControlBox->findData(flowControl);
    if (idx >= 0) {
        m_ui->flowControlBox->setCurrentIndex(idx);
    }
    updateSettings();
    emit settingsChanged();
}

void SettingsDialog::applyConnectionSettings(const s_Settings &s)
{
    if (s.readFromFileFlag) {
        setReadFromFile(s.pathToBinFile);
    }
    else {
        setPortName(s.name);
    }
    applyConnection(s.baudRate, static_cast<int>(s.dataBits), static_cast<int>(s.parity),
                    static_cast<int>(s.stopBits), static_cast<int>(s.flowControl));
    m_ui->readOnlyCheckBox->setChecked(s.readOnlyProfile);
    updateSettings();
    emit settingsChanged();
}

void SettingsDialog::showPortInfo(int idx)
{
    if (idx == -1) {
        return;
    }
    const QStringList list = m_ui->serialPortInfoListBox->itemData(idx).toStringList();
    m_ui->descriptionLabel->setText(tr("Description: %1").arg(list.count() > 1 ? list.at(1) : tr(blankString)));
    m_ui->manufacturerLabel->setText(tr("Manufacturer: %1").arg(list.count() > 2 ? list.at(2) : tr(blankString)));
    m_ui->serialNumberLabel->setText(tr("Serial number: %1").arg(list.count() > 3 ? list.at(3) : tr(blankString)));
    m_ui->locationLabel->setText(tr("Location: %1").arg(list.count() > 4 ? list.at(4) : tr(blankString)));
    m_ui->vidLabel->setText(tr("Vendor Identifier: %1").arg(list.count() > 5 ? list.at(5) : tr(blankString)));
    m_ui->pidLabel->setText(tr("Product Identifier: %1").arg(list.count() > 6 ? list.at(6) : tr(blankString)));
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

void SettingsDialog::checkCustomBaudRatePolicy(int idx)
{
    const bool isCustomBaudRate = !m_ui->baudRateBox->itemData(idx).isValid();
    m_ui->baudRateBox->setEditable(isCustomBaudRate);
    if (isCustomBaudRate) {
        m_ui->baudRateBox->clearEditText();
        QLineEdit *edit = m_ui->baudRateBox->lineEdit();
        edit->setValidator(m_intValidator);
    }
}

void SettingsDialog::portBoxEvent(int ev)
{
    if (m_ui->serialPortInfoListBox->currentText() == tr(("Read from file"))) {
        checkCustomDevicePathPolicy(ev);
    }
}

void SettingsDialog::checkCustomDevicePathPolicy(int idx)
{
    const bool isCustomPath = !m_ui->serialPortInfoListBox->itemData(idx).isValid();
    m_ui->serialPortInfoListBox->setEditable(isCustomPath);
    if (isCustomPath && m_ui->serialPortInfoListBox->currentText() == tr("Custom")) {
        m_ui->serialPortInfoListBox->clearEditText();
    }
    if (m_ui->serialPortInfoListBox->currentText() == tr("Read from file"))
    {
        m_ui->parametersBox->setDisabled(true);
        m_ui->serialPortInfoListBox->clearEditText();
        QString file = QFileDialog::getOpenFileName(this, tr("Open csv data file"), appHomeDir + "Logs", tr("csv data (*.csv)"));
        m_ui->serialPortInfoListBox->setCurrentText(file);
        if (!m_ui->serialPortInfoListBox->currentText().isEmpty())
        {
            m_currentSettings.readFromFileFlag = true;
            m_currentSettings.pathToBinFile = file;
        }
    }
    else {
        m_currentSettings.readFromFileFlag = false;
        m_ui->parametersBox->setDisabled(false);
    }
}

void SettingsDialog::fillPortsParameters()
{
    m_ui->baudRateBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    m_ui->baudRateBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    m_ui->baudRateBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    m_ui->baudRateBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    m_ui->baudRateBox->addItem(tr("Custom"));
    m_ui->dataBitsBox->addItem(QStringLiteral("5"), QSerialPort::Data5);
    m_ui->dataBitsBox->addItem(QStringLiteral("6"), QSerialPort::Data6);
    m_ui->dataBitsBox->addItem(QStringLiteral("7"), QSerialPort::Data7);
    m_ui->dataBitsBox->addItem(QStringLiteral("8"), QSerialPort::Data8);
    m_ui->dataBitsBox->setCurrentIndex(3);
    m_ui->parityBox->addItem(tr("None"), QSerialPort::NoParity);
    m_ui->parityBox->addItem(tr("Even"), QSerialPort::EvenParity);
    m_ui->parityBox->addItem(tr("Odd"), QSerialPort::OddParity);
    m_ui->parityBox->addItem(tr("Mark"), QSerialPort::MarkParity);
    m_ui->parityBox->addItem(tr("Space"), QSerialPort::SpaceParity);
    m_ui->stopBitsBox->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
    m_ui->stopBitsBox->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
#endif
    m_ui->stopBitsBox->addItem(QStringLiteral("2"), QSerialPort::TwoStop);
    m_ui->flowControlBox->addItem(tr("None"), QSerialPort::NoFlowControl);
    m_ui->flowControlBox->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
    m_ui->flowControlBox->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);
}

void SettingsDialog::fillPortsInfo()
{
    m_ui->serialPortInfoListBox->clear();
    QString description;
    QString manufacturer;
    QString serialNumber;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
        serialNumber = info.serialNumber();
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);
        m_ui->serialPortInfoListBox->addItem(list.first(), list);
    }
    m_ui->serialPortInfoListBox->addItem(tr("Custom"));
    m_ui->serialPortInfoListBox->addItem(tr("Read from file"));
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
    m_currentSettings.name = m_ui->serialPortInfoListBox->currentText();
    if (m_ui->baudRateBox->currentIndex() == 4) {
        m_currentSettings.baudRate = m_ui->baudRateBox->currentText().toInt();
    }
    else {
        m_currentSettings.baudRate = static_cast<QSerialPort::BaudRate>(
                                         m_ui->baudRateBox->itemData(m_ui->baudRateBox->currentIndex()).toInt());
    }
    m_currentSettings.stringBaudRate = QString::number(m_currentSettings.baudRate);
    m_currentSettings.dataBits = static_cast<QSerialPort::DataBits>(
                                     m_ui->dataBitsBox->itemData(m_ui->dataBitsBox->currentIndex()).toInt());
    m_currentSettings.stringDataBits = m_ui->dataBitsBox->currentText();
    m_currentSettings.parity = static_cast<QSerialPort::Parity>(
                                   m_ui->parityBox->itemData(m_ui->parityBox->currentIndex()).toInt());
    m_currentSettings.stringParity = m_ui->parityBox->currentText();
    m_currentSettings.stopBits = static_cast<QSerialPort::StopBits>(
                                     m_ui->stopBitsBox->itemData(m_ui->stopBitsBox->currentIndex()).toInt());
    m_currentSettings.stringStopBits = m_ui->stopBitsBox->currentText();
    m_currentSettings.flowControl = static_cast<QSerialPort::FlowControl>(
                                        m_ui->flowControlBox->itemData(m_ui->flowControlBox->currentIndex()).toInt());
    m_currentSettings.stringFlowControl = m_ui->flowControlBox->currentText();
    m_currentSettings.profilePath = selectedProfile;
    m_currentSettings.readOnlyProfile = m_ui->readOnlyCheckBox->isChecked();
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
    if (!(m_ui->readOnlyCheckBox->isChecked()))
    {
        QFile profile(selectedProfile);
        if (profile.exists()) {
            profile.remove();
        }
        fillProfileList();
    }
}

void SettingsDialog::on_readOnlyCheckBox_stateChanged(int)
{
    m_ui->deleteProfileButton->setDisabled(m_ui->readOnlyCheckBox->isChecked());
    m_ui->protocolSetupBox->setDisabled(m_ui->readOnlyCheckBox->isChecked());
    m_ui->descriptionTextEdit->setReadOnly(m_ui->readOnlyCheckBox->isChecked());
}

void SettingsDialog::on_writeBinChkBox_stateChanged(int)
{
    emit writeBinLog(m_ui->writeBinChkBox->isChecked());
}

void SettingsDialog::on_writeTxtChkBox_stateChanged(int)
{
    emit writeTextLog(m_ui->writeTxtChkBox->isChecked());
}

void SettingsDialog::on_writeJsonChkBox_stateChanged(int)
{
    emit writeJsonLog(m_ui->writeJsonChkBox->isChecked());
}

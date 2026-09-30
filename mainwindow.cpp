#include "mainwindow.h"
#include "aboutdialog.h"
#include "apppaths.h"
#include "packetdiagram.h"
#include "protocolsettings.h"
#include "controlboard.h"
#include "ui_mainwindow.h"
#include "newconnect.h"
#include <QtWidgets>
#include <QDebug>
#include <QPushButton>
#include <QProgressBar>
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QFileInfo>
#include <QCloseEvent>
#include <QList>
#include "device.h"
#include "devsettingsform.h"
#include "bytesettingsform.h"
#include <QMap>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), statuslbl (new QLabel), crcerrorlbl (new QLabel), aboutButton (new QPushButton), m_ui (new Ui::MainWindow)

{
    m_ui->setupUi(this);
    crcerrorlbl->setText(tr("CRC Errors: ") + QString::number(CRCErrorCount));
    statuslbl->setText(tr("Etrodiag"));
    aboutButton->setText(tr("About"));
    //Схема формата пакета пересобирается по изменениям профиля (одна пересборка на пачку изменений)
    diagramRefreshTimer = new QTimer(this);
    diagramRefreshTimer->setSingleShot(true);
    connect(diagramRefreshTimer, &QTimer::timeout, this, &MainWindow::updatePacketDiagram);
    logger = new Logger;
    addConnection();
    setupProfileArea();
    setupStatusBar();
    connection->readProfile(); //применяем профиль (протокол/настройки) при старте, как раньше открытие настроек
    connect (&byteSettForm, &ByteSettingsForm::editMask, &maskSettForm, &maskSettingsDialog::requestDataOnId);
    connect (this, &MainWindow::dvsfAfterCloseClear, &devSettForm, &devSettingsForm::afterCloseClearing);
    //Изменения масок в формах тоже отражаются на схеме формата пакета
    connect (&maskSettForm, &maskSettingsDialog::sendMaskData, this, &MainWindow::schedulePacketDiagramUpdate);
    connect (&byteSettForm, &ByteSettingsForm::createMask, this, &MainWindow::schedulePacketDiagramUpdate);
    connect (&byteSettForm, &ByteSettingsForm::deleteMaskObj, this, &MainWindow::schedulePacketDiagramUpdate);
    connect (&byteSettForm, &ByteSettingsForm::setWordBit, this, &MainWindow::schedulePacketDiagramUpdate);
    connect (m_ui->valueArea, &QTabWidget::currentChanged, this, &MainWindow::setCurrentOpenTab);
    connect (logger, &Logger::showStatusMessage, this, &MainWindow::showStatusMessage);
    connect (logger, &Logger::logLoadProgress, this, &MainWindow::setLogLoadProgress);
    connect (logger, &Logger::toTextLog, this, [this](QString text, bool redflag) {
        textLogWindow(QDateTime::currentDateTime(), text, redflag);
    });
    connect (logger, &Logger::readFromCsv, connection, &newconnect::sendRawDataWithTime);
    connect (this, &MainWindow::toTxtLogger, logger, &Logger::incomingTxtData);
    connect (aboutButton, &QPushButton::clicked, this, &MainWindow::onAboutButtonClicked);
    m_ui->logArea->viewport()->installEventFilter(this);
    graphiq.setParent(m_ui->graphLabel);
    m_ui->graphLayout->addWidget(&cBoard);
    cBoard.setVisible(false); //управление переменными - тестовая функция, в интерфейсе она скрыта
    connect (&cBoard, &ControlBoard::controlCommand, this, &MainWindow::guiCommandHandler);
    logger->setModel(&connection->m_settings->model()); //логгер читает настройки (путь к логу) из модели
    connect (this, &MainWindow::emitCommand, connection, &newconnect::receiveCommandFromGui);
    //Пропорции панели мониторинга: график сверху, под ним ряд "устройства | параметры | лог".
    //Ряду отдаём 3/5 высоты, чтобы форма настроек маски (до 16 бит) помещалась при минимуме окна.
    m_ui->verticalLayout_3->setStretch(0, 2);
    m_ui->verticalLayout_3->setStretch(1, 3);
    m_ui->monitorRow->setStretch(0, 0); //кнопки устройств фиксированной ширины
    m_ui->monitorRow->setStretch(1, 1); //таблица параметров
    m_ui->monitorRow->setStretch(2, 1); //текстовый лог
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
    m_ui->horizontalLayout_3->addWidget(connection, 1);
    connect (connection, &newconnect::loadMask, this, &MainWindow::loadProfile);
    connect (connection, &newconnect::sendStatusStr, this, &MainWindow::showStatusMessage);
    connect (connection, &newconnect::transmitData, this, &MainWindow::addDeviceToList);
    connect (connection, &newconnect::cleanDevListSig, this, &MainWindow::cleanDevList);
    connect (connection, &newconnect::writeTextLog, logger, &Logger::setTxt);
    connect (connection, &newconnect::writeJsonLog, logger, &Logger::setJson);
    connect (connection, &newconnect::writeBinLog, logger, &Logger::setBin);
    connect (connection, &newconnect::connected, &graphiq, &liveGraph::cleanGraph);
    connect (connection, &newconnect::connected, &graphiq, &liveGraph::startOscillator);
    connect (connection, &newconnect::readFromFileSignal, &graphiq, &liveGraph::cleanGraph);
    connect (connection, &newconnect::readFromFileSignal, &graphiq, &liveGraph::readFromFileSignal);
    connect (connection, &newconnect::readFromFileSignal, logger, &Logger::binReadFromCsv);
    connect (connection, &newconnect::disconnected, &graphiq, &liveGraph::stopOscillator);
    connect (this, &MainWindow::prepareToSaveProfile, connection, &newconnect::prepareToSaveProfile);
    connect (this, &MainWindow::saveProfile, connection, &newconnect::saveProfile);
    connect (connection, &newconnect::sendRawData, logger, &Logger::incomingBinData);
    connect (connection, &newconnect::connected, logger, &Logger::startLog);
    connect (connection, &newconnect::disconnected, logger, &Logger::stopLog);
    connect (connection, &newconnect::profileName2log, logger, &Logger::setProfileName);
    connect (connection, &newconnect::profileLoaded, this, &MainWindow::schedulePacketDiagramUpdate); //маски профиля разосланы - обновляем схему
    //панель информации о профиле обновляем сразу по загрузке профиля, иначе при старте
    //в ней остаются значения по умолчанию, а имя профиля уже подставлено
    connect (connection, &newconnect::profileLoaded, this, &MainWindow::updateProfileInfo);
    //таблицы параметров наполняем сразу по профилю, данные потом только обновляют значения
    connect (connection, &newconnect::profileLoaded, this, &MainWindow::fillValueAreaFromProfile);
    connect (connection, &newconnect::setProtocol, this, &MainWindow::updateProfileInfo); //протокол изменён в редакторе профиля
    connect (connection, &newconnect::badCRC, this, &MainWindow::badCRCEvent);
    connect (connection, &newconnect::logLoadProgress, this, &MainWindow::setLogLoadProgress);
    connect(this, &MainWindow::emitCommand, connection, &newconnect::receiveCommandFromGui);
    connection->show();
}

void MainWindow::setupStatusBar()
{ //Строка состояния управляет подключением, профилем, портом, параметрами связи и логами
    profileButton = new QToolButton;
    profileButton->setPopupMode(QToolButton::InstantPopup);
    profileButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    profileButton->setMenu(new QMenu(profileButton));
    connect(profileButton->menu(), &QMenu::aboutToShow, this, &MainWindow::fillProfileMenu);

    portButton = new QToolButton;
    portButton->setPopupMode(QToolButton::InstantPopup);
    portButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    portButton->setMenu(new QMenu(portButton));
    connect(portButton->menu(), &QMenu::aboutToShow, this, &MainWindow::fillPortMenu);

    paramsButton = new QToolButton;
    paramsButton->setPopupMode(QToolButton::InstantPopup);
    paramsButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    paramsButton->setMenu(new QMenu(paramsButton));
    connect(paramsButton->menu(), &QMenu::aboutToShow, this, &MainWindow::fillParamsMenu);

    logButton = new QToolButton;
    logButton->setPopupMode(QToolButton::InstantPopup);
    logButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    QMenu *logMenu = new QMenu(logButton);
    QAction *txtAction = logMenu->addAction(tr("Write text log"));
    txtAction->setCheckable(true);
    connect(txtAction, &QAction::toggled, this, [this](bool on) { connection->m_settings->setWriteTxt(on); });
    QAction *binAction = logMenu->addAction(tr("Write binary data"));
    binAction->setCheckable(true);
    connect(binAction, &QAction::toggled, this, [this](bool on) { connection->m_settings->setWriteBin(on); });
    QAction *jsonAction = logMenu->addAction(tr("Write json log"));
    jsonAction->setCheckable(true);
    connect(jsonAction, &QAction::toggled, this, [this](bool on) { connection->m_settings->setWriteJson(on); });
    logButton->setMenu(logMenu);

    connectButton = new QPushButton(tr("Connect"));

    loadProgress = new QProgressBar;
    loadProgress->setRange(0, 100);
    loadProgress->setValue(0);
    loadProgress->setFixedWidth(160);
    loadProgress->setTextVisible(true);
    loadProgress->setStyleSheet("QProgressBar{border:1px solid #808080;border-radius:2px;text-align:center;}"
                                "QProgressBar::chunk{background-color:#00CC00;}");
    loadProgress->hide();

    statusBar()->addWidget(statuslbl, 1);
    statusBar()->addWidget(profileButton);
    statusBar()->addWidget(portButton);
    statusBar()->addWidget(paramsButton);
    statusBar()->addWidget(logButton);
    statusBar()->addWidget(connectButton);
    statusBar()->addWidget(loadProgress);
    statusBar()->addWidget(crcerrorlbl);
    statusBar()->addWidget(aboutButton);

    connect(connectButton, &QPushButton::clicked, this, [this]() { connection->toggleConnection(); });
    connect(connection, &newconnect::connectButtonTextChanged, connectButton, &QPushButton::setText);
    connect(connection->m_settings, &SettingsDialog::settingsChanged, this, &MainWindow::refreshConnectionButtons);
    connect(connection, &newconnect::connected, this, [this]() {
        serialConnected = true;
        refreshConnectionButtons();
    });
    connect(connection, &newconnect::disconnected, this, [this]() {
        serialConnected = false;
        refreshConnectionButtons();
    });

    connectButton->setText(tr("Connect"));
    refreshConnectionButtons();

    //Автоматическое обновление списка COM-портов (без перезапуска программы)
    knownPortList = connection->m_settings->availablePortNames();
    portPollTimer = new QTimer(this);
    portPollTimer->setInterval(1000);
    connect(portPollTimer, &QTimer::timeout, this, &MainWindow::pollPorts);
    portPollTimer->start();
}

void MainWindow::refreshConnectionButtons()
{
    if (!connection || !connection->m_settings) {
        return;
    }
    SettingsDialog *s = connection->m_settings;
    const QString profileName = s->currentProfileName();
    profileButton->setText(profileName.isEmpty() ? tr("Profile") : profileName);

    if (s->isReadFromFile()) {
        portButton->setText(QFileInfo(s->selectedPortName()).fileName());
    }
    else {
        const QString port = s->selectedPortName();
        portButton->setText(port.isEmpty() ? tr("Port") : port);
    }

    paramsButton->setText(s->connectionSummary());
    paramsButton->setDisabled(serialConnected);
    paramsButton->setVisible(!s->isReadFromFile());
    portButton->setDisabled(serialConnected); //при активном соединении порт не меняем
    profileButton->setDisabled(serialConnected); //при активном соединении профиль не переключаем
    logButton->setVisible(!s->isReadFromFile()); //при чтении из файла настройка логов не нужна

    QStringList activeLogs;
    if (s->writeTxtEnabled()) {
        activeLogs << QStringLiteral("txt");
    }
    if (s->writeBinEnabled()) {
        activeLogs << QStringLiteral("bin");
    }
    if (s->writeJsonEnabled()) {
        activeLogs << QStringLiteral("json");
    }
    logButton->setText(activeLogs.isEmpty() ? tr("Logs") : (tr("Logs") + ": " + activeLogs.join('+')));

    if (connectButton && loadProgress && !loadProgress->isVisible()) {
        if (s->isReadFromFile()) {
            connectButton->setText(connection->isReaderBusy() ? tr("Stop read log") : tr("Read log"));
        }
        else {
            connectButton->setText(serialConnected ? tr("Disconnect") : tr("Connect"));
        }
    }
    updateProfileInfo();
}

void MainWindow::setupProfileArea()
{ //Делим вкладку соединения: слева консоль, справа информация о профиле / редактор профиля
    profileInfoLabel = new QLabel;
    profileInfoLabel->setTextFormat(Qt::RichText);
    profileInfoLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    profileInfoLabel->setWordWrap(true);
    profileInfoLabel->setMargin(8);
    profileInfoLabel->setMinimumWidth(0);
    profileInfoLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    //Схема формата пакета: моноширинный шрифт без переноса, иначе столбцы разъедутся
    packetDiagramView = new QPlainTextEdit;
    packetDiagramView->setReadOnly(true);
    packetDiagramView->setLineWrapMode(QPlainTextEdit::NoWrap);
    packetDiagramView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    packetDiagramView->setFrameShape(QFrame::StyledPanel);

    QWidget *infoPage = new QWidget;
    QVBoxLayout *infoLayout = new QVBoxLayout(infoPage);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    infoLayout->addWidget(profileInfoLabel, 0);
    infoLayout->addWidget(packetDiagramView, 1);

    //Редактор профиля кладём в область прокрутки: так страница может быть уже своего
    //естественного минимума, и при маленьком окне ничего не наезжает на консоль слева.
    QScrollArea *editorScroll = new QScrollArea;
    editorScroll->setWidgetResizable(true);
    editorScroll->setFrameShape(QFrame::NoFrame);
    editorScroll->setWidget(connection->m_settings);

    profileArea = new QStackedWidget;
    profileArea->addWidget(infoPage);                       // 0 - информация о профиле
    profileArea->addWidget(editorScroll);                   // 1 - редактор профиля
    profileArea->setCurrentIndex(0);

    connect(connection->m_settings, &SettingsDialog::restoreConsoleAndButtons, this, [this]() {
        if (profileArea) { //после "Применить" возвращаемся к информации о профиле
            profileArea->setCurrentIndex(0);
            updateProfileInfo();
        }
    });

    //Консоль занимает треть ширины вкладки, информация о профиле - две трети
    m_ui->horizontalLayout_3->addWidget(profileArea, 2);
    updateProfileInfo();
}

void MainWindow::updateProfileInfo()
{
    if (!connection || !connection->m_settings || !profileInfoLabel) {
        return;
    }
    SettingsDialog *s = connection->m_settings;
    const QString profileName = s->currentProfileName();
    const s_protocolDescription &protocol = connection->m_settings->model().protocol(); //протокол берём из модели профиля

    QString text;
    text += "<b>" + tr("Profile: ") + (profileName.isEmpty() ? tr("not selected") : profileName) + "</b><br><br>";
    text += tr("Packet size") + ": " + QString::number(protocol.packetSize) + "<br>";
    text += tr("Block identifycator position") + ": " + QString::number(protocol.blockIdentifycatorPosition) + "<br>";
    text += tr("Calc CRC from position") + ": " + QString::number(protocol.calcCRCFromPosition) + "<br>";
    text += tr("Marker of begin - size") + ": " + QString::number(protocol.markerPacketBeginSize) + "<br>";
    text += tr("Marker b0/b1") + ": " + QString::number(protocol.markerPacketBeginByte1, 16).toUpper()
            + " / " + QString::number(protocol.markerPacketBeginByte2, 16).toUpper() + "<br>";
    text += tr("Variables control") + ": " + (protocol.varControl ? tr("yes") : tr("no")) + "<br>";
    text += tr("Description") + ": " + protocol.description + "<br><br>";
    text += tr("Connection") + ": " + s->connectionSummary();
    profileInfoLabel->setText(text);
    schedulePacketDiagramUpdate(); //протокол мог измениться - схему пересобираем
}

void MainWindow::schedulePacketDiagramUpdate()
{ //одна пересборка на пачку изменений: загрузка профиля рассылает маски по одной
    if (diagramRefreshTimer) {
        diagramRefreshTimer->start(0);
    }
}

void MainWindow::updatePacketDiagram()
{ //схема строится из текущего протокола и живых масок устройств
    if (!packetDiagramView || !connection || !connection->m_settings) {
        return;
    }
    QVector<s_parameterMask> masks;
    const QList<Device*> devices = m_ui->devArea->findChildren<Device*>();
    for (const Device *device : devices) {
        masks += device->currentMasks();
    }
    const s_protocolDescription &protocol = connection->m_settings->model().protocol();
    const packetdiagram::DiagramPicture diagram = packetdiagram::pictureDevices(protocol, masks);
    packetDiagramView->setPlainText(diagram.text);
    applyParamColors(diagram.colors);

    //Минимальная ширина панели - по самой длинной строке схемы: так схема видна целиком,
    //без горизонтальной прокрутки, и вместе с ней растёт минимальная ширина окна
    const QFontMetricsF metrics(packetDiagramView->font());
    qreal widestLine = 0;
    for (const QString &line : packetDiagramView->toPlainText().split('\n')) {
        widestLine = qMax(widestLine, metrics.horizontalAdvance(line));
    }
    const int chrome = packetDiagramView->frameWidth() * 2 + 8
                       + packetDiagramView->verticalScrollBar()->sizeHint().width();
    packetDiagramView->setMinimumWidth(static_cast<int>(widestLine) + chrome);
}

void MainWindow::applyParamColors(const QVector<packetdiagram::ParamColor> &colors)
{ //обозначения параметров с включённым графиком выделяем цветом из настроек маски
    QList<QTextEdit::ExtraSelection> selections;
    for (const packetdiagram::ParamColor &color : colors) {
        //Схема собрана заранее, поэтому место обозначения известно точно - ищем его по позиции
        QTextCursor range(packetDiagramView->document());
        range.setPosition(color.position);
        range.setPosition(color.position + color.length, QTextCursor::KeepAnchor);
        QTextEdit::ExtraSelection selection;
        selection.cursor = range;
        selection.format.setBackground(QColor(color.background));
        selection.format.setForeground(QColor(color.foreground));
        selections.append(selection);
    }
    packetDiagramView->setExtraSelections(selections);
}

void MainWindow::saveProfileChanges()
{ //изменения профиля (маски, параметры связи) собираем и пишем в <профиль>.eag.tmp;
  //сам профиль заменяется по подтверждению при разрыве соединения или закрытии программы
    emit prepareToSaveProfile();
    emit saveProfile();
}

void MainWindow::onEditProfile()
{ //в правой половине вкладки соединения показываем редактор профиля
    if (!connection || !connection->m_settings || !profileArea) {
        return;
    }
    connection->editProfile(); //заполняем поля редактора текущим профилем
    profileArea->setCurrentIndex(1);
}

void MainWindow::fillProfileMenu()
{
    QMenu *menu = profileButton->menu();
    menu->clear();
    SettingsDialog *s = connection->m_settings;
    const QStringList profiles = s->profileNames();
    for (const QString &name : profiles) {
        QAction *a = menu->addAction(name);
        a->setCheckable(true);
        a->setChecked(name == s->currentProfileName());
        connect(a, &QAction::triggered, this, [this, name]() {
            connection->m_settings->selectProfile(name);
            refreshConnectionButtons();
        });
    }
    if (!profiles.isEmpty()) {
        menu->addSeparator();
    }
    QAction *createAction = menu->addAction(tr("Create new profile"));
    connect(createAction, &QAction::triggered, this, [this]() {
        connection->m_settings->createNewProfile();
        refreshConnectionButtons();
    });
    QAction *editAction = menu->addAction(tr("Edit profile"));
    connect(editAction, &QAction::triggered, this, [this]() {
        m_ui->tabWidget->setCurrentIndex(0);
        onEditProfile();
    });
    QAction *deleteAction = menu->addAction(tr("Delete profile"));
    deleteAction->setEnabled(!profiles.isEmpty());
    connect(deleteAction, &QAction::triggered, this, [this]() {
        connection->m_settings->deleteCurrentProfile();
        refreshConnectionButtons();
    });
}

void MainWindow::fillPortMenu()
{
    QMenu *menu = portButton->menu();
    menu->clear();
    SettingsDialog *s = connection->m_settings;
    const QStringList ports = s->availablePortNames();
    for (const QString &name : ports) {
        QAction *a = menu->addAction(name);
        a->setCheckable(true);
        a->setChecked(!s->isReadFromFile() && name == s->selectedPortName());
        connect(a, &QAction::triggered, this, [this, name]() {
            connection->m_settings->setPortName(name);
            refreshConnectionButtons();
        });
    }
    menu->addSeparator();
    QAction *fileAction = menu->addAction(tr("Read from file"));
    fileAction->setCheckable(true);
    fileAction->setChecked(s->isReadFromFile());
    connect(fileAction, &QAction::triggered, this, [this]() {
        SettingsDialog *st = connection->m_settings;
        const QString file = QFileDialog::getOpenFileName(this, tr("Open csv data file"),
                                                          apppaths::logsDir(), tr("csv data (*.csv)"));
        if (!file.isEmpty()) {
            st->setReadFromFile(file);
            refreshConnectionButtons();
        }
    });
}

void MainWindow::pollPorts()
{ //Периодически проверяем список портов: при изменении обновляем открытое меню
    if (!connection || !connection->m_settings) {
        return;
    }
    const QStringList ports = connection->m_settings->availablePortNames();
    if (ports == knownPortList) {
        return;
    }
    knownPortList = ports;
    if (portButton && portButton->menu() && portButton->menu()->isVisible()) {
        fillPortMenu();
    }
}

void MainWindow::fillParamsMenu()
{
    QMenu *menu = paramsButton->menu();
    menu->clear();
    SettingsDialog *s = connection->m_settings;
    const s_Settings cur = s->currentSettings();
    const int curBaud = cur.baudRate;
    const int curData = static_cast<int>(cur.dataBits);
    const int curParity = static_cast<int>(cur.parity);
    const int curStop = static_cast<int>(cur.stopBits);
    const int curFlow = static_cast<int>(cur.flowControl);
    auto apply = [this](int baud, int data, int parity, int stop, int flow) {
        connection->m_settings->applyConnection(baud, data, parity, stop, flow);
        refreshConnectionButtons();
        saveProfileChanges(); //изменённые параметры связи сразу уходят на сохранение в профиль
    };

    QMenu *baudMenu = menu->addMenu(tr("Baud rate"));
    const QVector<int> bauds = {9600, 19200, 38400, 115200};
    for (int b : bauds) {
        QAction *a = baudMenu->addAction(QString::number(b));
        a->setCheckable(true);
        a->setChecked(curBaud == b);
        connect(a, &QAction::triggered, this, [apply, curBaud, curData, curParity, curStop, curFlow, b]() {
            Q_UNUSED(curBaud)
            apply(b, curData, curParity, curStop, curFlow);
        });
    }

    QMenu *dataMenu = menu->addMenu(tr("Data bits"));
    const QVector<int> dataBits = {5, 6, 7, 8};
    for (int d : dataBits) {
        QAction *a = dataMenu->addAction(QString::number(d));
        a->setCheckable(true);
        a->setChecked(curData == d);
        connect(a, &QAction::triggered, this, [apply, curBaud, curParity, curStop, curFlow, d]() {
            apply(curBaud, d, curParity, curStop, curFlow);
        });
    }

    const QVector<QPair<QString, int>> parities = {
        {tr("None"), static_cast<int>(QSerialPort::NoParity)},
        {tr("Even"), static_cast<int>(QSerialPort::EvenParity)},
        {tr("Odd"), static_cast<int>(QSerialPort::OddParity)},
        {tr("Mark"), static_cast<int>(QSerialPort::MarkParity)},
        {tr("Space"), static_cast<int>(QSerialPort::SpaceParity)}
    };
    QMenu *parityMenu = menu->addMenu(tr("Parity"));
    for (const QPair<QString, int> &p : parities) {
        QAction *a = parityMenu->addAction(p.first);
        a->setCheckable(true);
        a->setChecked(curParity == p.second);
        const int value = p.second;
        connect(a, &QAction::triggered, this, [apply, curBaud, curData, curStop, curFlow, value]() {
            apply(curBaud, curData, value, curStop, curFlow);
        });
    }

    const QVector<QPair<QString, int>> stopBits = {
        {tr("1"), static_cast<int>(QSerialPort::OneStop)},
#ifdef Q_OS_WIN
        {tr("1.5"), static_cast<int>(QSerialPort::OneAndHalfStop)},
#endif
        {tr("2"), static_cast<int>(QSerialPort::TwoStop)}
    };
    QMenu *stopMenu = menu->addMenu(tr("Stop bits"));
    for (const QPair<QString, int> &p : stopBits) {
        QAction *a = stopMenu->addAction(p.first);
        a->setCheckable(true);
        a->setChecked(curStop == p.second);
        const int value = p.second;
        connect(a, &QAction::triggered, this, [apply, curBaud, curData, curParity, curFlow, value]() {
            apply(curBaud, curData, curParity, value, curFlow);
        });
    }

    const QVector<QPair<QString, int>> flowControls = {
        {tr("None"), static_cast<int>(QSerialPort::NoFlowControl)},
        {tr("RTS/CTS"), static_cast<int>(QSerialPort::HardwareControl)},
        {tr("XON/XOFF"), static_cast<int>(QSerialPort::SoftwareControl)}
    };
    QMenu *flowMenu = menu->addMenu(tr("Flow control"));
    for (const QPair<QString, int> &p : flowControls) {
        QAction *a = flowMenu->addAction(p.first);
        a->setCheckable(true);
        a->setChecked(curFlow == p.second);
        const int value = p.second;
        connect(a, &QAction::triggered, this, [apply, curBaud, curData, curParity, curStop, value]() {
            apply(curBaud, curData, curParity, curStop, value);
        });
    }
}

void MainWindow::showStatusMessage(QString message)
{
    statuslbl->setText(message);
    textLogWindow(QDateTime::currentDateTime(), message, true);
}

void MainWindow::setLogLoadProgress(int percent)
{
    if (percent >= 100) { //загрузка завершена - возвращаем кнопку подключения, прячем прогрессбар
        loadProgress->setValue(100);
        loadProgress->hide();
        if (connectButton) {
            connectButton->show();
        }
        return;
    }
    if (!loadProgress->isVisible()) { //во время чтения лога кнопка подключения заменяется прогрессбаром
        loadProgress->show();
        if (connectButton) {
            connectButton->hide();
        }
    }
    loadProgress->setValue(percent);
    statuslbl->setText(tr("Reading log") + ": " + QString::number(percent) + "%");
}

void MainWindow::addDeviceToList(QDateTime currentTime, QVector<int> ddata)
{
    const s_protocolDescription &protocol = connection->m_settings->model().protocol(); //протокол берём из модели профиля
    if (protocol.blockIdentifycatorPosition < 0 || protocol.blockIdentifycatorPosition >= ddata.size()) {
        return; //некорректный протокол/данные - не даём выйти за границы
    }
    devNum = ddata.at(protocol.blockIdentifycatorPosition);//узнаём номер устройства в посылке
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
    dev->setModel(&connection->m_settings->model()); //устройство читает протокол из модели профиля
    //имя по умолчанию - id узла из пакета (в hex); имя из профиля подставится при загрузке масок
    dev->setDeviceName(devNum, QString("%1").arg(devNum, 0, 16).toUpper());
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

void MainWindow::closeMaskSettings(int devNum)
{ //закрываем настройки маски: возвращаемся либо к списку значений, либо к настройкам байта
    maskSettForm.sendMask2Profile();
    maskSettForm.hide();
    maskSettForm.killChildren();
    if (maskSettForm.openDirectly)
    {
        emit hideOtherDevButtons(false, devNum);
        saveProfileChanges();
        maskSettForm.openDirectly = false;
        fillValueAreaFromProfile(); //возвращаемся к таблицам параметров - наполняем их по профилю
        graphiq.clearAnnotations();
        m_ui->valueArea->show();
    }
    else {
        byteSettForm.show();
        byteSettForm.resize(m_ui->rightFrame->size());
    }
}

void MainWindow::closeByteSettings()
{ //из настроек байта возвращаемся к списку устройств
    byteSettForm.hide();
    byteSettForm.cleanForm();
    devSettForm.show();
    devSettForm.resize(m_ui->rightFrame->size());
}

void MainWindow::toggleDeviceSettings(int devNum, QVector<int> data)
{ //открываем форму устройства или возвращаемся к списку значений
    devSettForm.setParent(m_ui->rightFrame);
    if (m_ui->valueArea->isHidden())
    {
        devSettForm.hide();
        emit dvsfAfterCloseClear();
        fillValueAreaFromProfile(); //возвращаемся к таблицам параметров - наполняем их по профилю
        graphiq.clearAnnotations();
        m_ui->valueArea->show();
        emit hideOtherDevButtons(false, devNum);
        saveProfileChanges();
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

void MainWindow::openDevSett(int devNum, QVector<int> data)
{ //все реакции на нажатие кнопки устройства в зависимости от состояния окна
    if (maskSettForm.isVisible()) {
        closeMaskSettings(devNum);
    }
    else if (byteSettForm.isVisible()) {
        closeByteSettings();
    }
    else {
        toggleDeviceSettings(devNum, data);
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

QTableWidget *MainWindow::valueTableForDevice(const QString &devName)
{ //вкладка устройства (по имени) и её таблица, либо nullptr
    for (int i = 0; i < m_ui->valueArea->count(); ++i) {
        if (m_ui->valueArea->tabText(i) == devName) {
            return qobject_cast<QTableWidget*>(m_ui->valueArea->widget(i));
        }
    }
    return nullptr;
}

QTableWidget *MainWindow::createValueTable(const QString &devName)
{ //создаём таблицу значений и вкладку с именем устройства
    QTableWidget *table = new QTableWidget(m_ui->valueArea);
    connect(table, &QTableWidget::cellClicked, this, &MainWindow::ValueArea_CellClicked);
    table->insertColumn(0);//name
    table->insertColumn(1);//value
    table->insertColumn(2);//devnum
    table->insertColumn(3);//bytenum
    table->insertColumn(4);//maskid
    table->hideColumn(2);//скрываем колонки: данные нужны только для открытия настроек нужной маски
    table->hideColumn(3);
    table->hideColumn(4);
    //Заголовок скрыт, но режимы секций нужны: имя - по содержимому, значение растягивается на остаток
    //ширины, поэтому длинное значение больше не обрезается и нет горизонтальной прокрутки.
    table->horizontalHeader()->hide();
    table->horizontalHeader()->setStretchLastSection(false);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_ui->valueArea->addTab(table, devName);
    return table;
}

int MainWindow::valueTableRowFor(QTableWidget *table, const s_parameterMask &mask) const
{ //строку параметра ищем по номеру устройства, байту и id маски: имена параметров могут совпадать
    for (int i = 0; i < table->rowCount(); ++i) {
        const QTableWidgetItem *devItem = table->item(i, 2);
        const QTableWidgetItem *byteItem = table->item(i, 3);
        const QTableWidgetItem *idItem = table->item(i, 4);
        if (devItem && byteItem && idItem
                && devItem->text().toInt() == mask.devNum
                && byteItem->text().toInt() == mask.byteNum
                && idItem->text().toInt() == mask.id) {
            return i;
        }
    }
    return -1;
}

void MainWindow::addValueTableRow(QTableWidget *table, const s_parameterMask &mask, const QString &valueText)
{ //новая строка параметра: подпись, значение и скрытые номера для открытия настроек маски
    const int row = table->rowCount();
    table->setRowCount(row + 1);
    table->setItem(row, 0, new QTableWidgetItem(mask.parameterName + '@' + mask.devName));
    table->setItem(row, 1, new QTableWidgetItem(valueText));
    table->setItem(row, 2, new QTableWidgetItem(QString::number(mask.devNum)));
    table->setItem(row, 3, new QTableWidgetItem(QString::number(mask.byteNum)));
    table->setItem(row, 4, new QTableWidgetItem(QString::number(mask.id)));
    table->resizeRowsToContents();
}

void MainWindow::updateValueTableRow(QTableWidget *table, const s_parameterMask &mask)
{ //обновляем значение параметра или создаём строку, если её ещё нет
    const QString valueText = QString::number(mask.endValue, 'g', 6);
    const int row = valueTableRowFor(table, mask);
    if (row < 0) {
        addValueTableRow(table, mask, valueText);
        return;
    }
    table->item(row, 0)->setText(mask.parameterName + '@' + mask.devName); //имя могло измениться в настройках маски
    if (valueText != table->item(row, 1)->text()) { //значение изменилось - подсвечиваем
        table->item(row, 1)->setText(valueText);
        table->item(row, 1)->setBackground(Qt::green);
    }
    else {
        table->item(row, 1)->setBackground(Qt::white);
    }
}

void MainWindow::fillValueAreaFromProfile()
{ //таблицы параметров заполняем сразу по профилю (устройство - вкладка, параметр - строка),
  //а приход данных будет только обновлять значения: до первого кадра в значении прочерк
    const QString openedTab = m_ui->valueArea->tabText(m_ui->valueArea->currentIndex());
    m_ui->valueArea->clear(); //вкладки прежнего профиля больше не нужны
    const QList<Device*> devices = m_ui->devArea->findChildren<Device*>();
    for (const Device *device : devices) {
        const QVector<s_parameterMask> masks = device->currentMasks();
        for (const s_parameterMask &mask : masks) {
            QTableWidget *table = valueTableForDevice(mask.devName);
            if (!table) {
                table = createValueTable(mask.devName);
            }
            if (table) {
                addValueTableRow(table, mask, QStringLiteral("-"));
            }
        }
    }
    for (int i = 0; i < m_ui->valueArea->count(); ++i) {
        if (m_ui->valueArea->tabText(i) == openedTab) {
            m_ui->valueArea->setCurrentIndex(i); //возвращаемся на ту вкладку устройства, где были
            break;
        }
    }
}

void MainWindow::updValueArea(s_parameterMask mask)
{
    QTableWidget *table = valueTableForDevice(mask.devName);
    if (!table) {
        table = createValueTable(mask.devName);
    }
    if (!table) {
        return;
    }
    updateValueTableRow(table, mask);
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
    { //создаём устройство и инициализируем пустым пакетом в oneMsgLeight байт
        createDevice(mask.devNum);
        const s_protocolDescription &protocol = connection->m_settings->model().protocol();
        const int packetSize = (protocol.packetSize > 0) ? protocol.packetSize : 1; //защита от невалидного профиля
        QVector<int> devInitArray(packetSize, 0);
        const int idPosition = protocol.blockIdentifycatorPosition;
        if (idPosition >= 0 && idPosition < devInitArray.size()) { //не даём выйти за границы массива
            devInitArray.replace(idPosition, mask.devNum);
        }
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

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (connection) {
        connection->offerToSaveProfile(); //при закрытии программы предлагаем сохранить изменения профиля
    }
    event->accept();
}

void MainWindow::cleanDevList()
{
    QList<Device*> vlayChildList = m_ui->devArea->findChildren<Device*>();
    QListIterator<Device*> vlayChildListIt(vlayChildList);
    while(vlayChildListIt.hasNext()) {
        delete vlayChildListIt.next();
    }
    CRCErrorCount = 0;
    graphiq.cleanGraph(); //чистим графики, чтобы после смены профиля не оставались чужие кривые
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
bool MainWindow::event(QEvent *event)
{
    if ((event->type() == QEvent::MouseButtonDblClick) && graphiq.isVisible())
    {
        graphiq.chngMinMaxVisible();
    }
    return QMainWindow::event(event);
}

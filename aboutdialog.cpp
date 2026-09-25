#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QDateTime>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    ui->aboutImg->setPixmap(pixmap->scaledToWidth(400, Qt::FastTransformation));
    ui->aboutImg->setScaledContents(true);
    ui->aboutImg->show();
    //Дата и время сборки берутся из времени изменения самого исполняемого файла:
    //оно гарантированно обновляется при каждой линковке, тогда как __DATE__/__TIME__
    //зашиваются в момент компиляции файла и не меняются, пока он не пересобран.
    const QDateTime buildTime = QFileInfo(QCoreApplication::applicationFilePath()).lastModified();
    if (buildTime.isValid()) {
        ui->aboutTextBrowser->append(tr("Build from ") + buildTime.toString("dd.MM.yyyy hh:mm:ss"));
    }
    else {
        ui->aboutTextBrowser->append(tr("Build from ") + QStringLiteral(__DATE__ " " __TIME__));
    }
    connect(ui->closeAboutButton, &QPushButton::clicked, this, &QDialog::hide);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

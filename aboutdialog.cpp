#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    ui->aboutImg->setPixmap(pixmap->scaledToWidth(400, Qt::FastTransformation));
    ui->aboutImg->setScaledContents(true);
    ui->aboutImg->show();
    ui->aboutTextBrowser->append(BUILDV);
    connect(ui->closeAboutButton, &QPushButton::clicked, this, &QDialog::hide);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

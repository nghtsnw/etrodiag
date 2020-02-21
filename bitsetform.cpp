#include "bitsetform.h"
#include "ui_bitsetform.h"
#include <QDebug>

bitSetForm::bitSetForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::bitSetForm)
{
    ui->setupUi(this);
    connect (timer, &QTimer::timeout, this, &bitSetForm::backgroundReturn);
}

bitSetForm::~bitSetForm()
{
    delete ui;
}

void bitSetForm::setNumLabel(int num)
{
    ui->bitNumLabel->setText(QString::number(num,10));
}

bool bitSetForm::checkboxStatus()
{
    return ui->bitCheckBox->isChecked();
}

void bitSetForm::setCheckBox(bool bit, int id)
{
    if (id == ui->bitNumLabel->text().toInt(0,10))
    ui->bitCheckBox->setChecked(bit);
}

void bitSetForm::setCheckboxText(int i, bool data)
{
    if (i == ui->bitNumLabel->text().toInt(0,10))
    {
        oldCheckboxText = ui->bitCheckBox->text();
        ui->bitCheckBox->setText(QString::number(data,10));
    }
    if (ui->bitCheckBox->text() != oldCheckboxText)
    {
        //this->setStyleSheet("QWidget{background:#00FF00;}");
        this->ui->bitCheckBox->setStyleSheet("QCheckBox{background:#00FF00;}");
        timer->setInterval(500);
        timer->start();
    }
}

void bitSetForm::backgroundReturn()
{
    this->ui->bitCheckBox->setStyleSheet("QCheckBox{background:none;}");
    timer->stop();
}

void bitSetForm::on_bitCheckBox_clicked()
{
    emit scanCheckboxesToMask();
}

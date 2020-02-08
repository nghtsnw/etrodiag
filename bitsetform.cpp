#include "bitsetform.h"
#include "ui_bitsetform.h"
#include <QDebug>

bitSetForm::bitSetForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::bitSetForm)
{
    ui->setupUi(this);
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
    ui->bitCheckBox->setText(QString::number(data,10));
}

void bitSetForm::on_bitCheckBox_clicked()
{
    emit scanCheckboxesToMask();
}

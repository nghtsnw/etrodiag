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
    //qDebug() << "checkbox status " << ui->bitNumLabel->text() << " = " << ui->bitCheckBox->isChecked();
    return ui->bitCheckBox->isChecked();
}

void bitSetForm::setCheckBox(bool bit, int id)
{
    if (id == ui->bitNumLabel->text().toInt(0,10))
    ui->bitCheckBox->setChecked(bit);
}

void bitSetForm::setCheckboxText(QString data)
{
    ui->bitCheckBox->setText(data);
}

void bitSetForm::on_bitCheckBox_clicked()
{
    emit scanCheckboxesToMask();
}

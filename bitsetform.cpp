#include "bitsetform.h"
#include "ui_bitsetform.h"

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

void bitSetForm::setCheckbox(bool bit)
{
    ui->bitCheckBox->setTristate(bit);
}

//void bitSetForm::setCheckboxText(QChar ch);

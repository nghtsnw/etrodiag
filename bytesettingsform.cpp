#include "bytesettingsform.h"
#include "ui_bytesettingsform.h"
#include <QDebug>
#include "bitmaskobj.h"


ByteSettingsForm::ByteSettingsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ByteSettingsForm)
{
    ui->setupUi(this);
}


ByteSettingsForm::~ByteSettingsForm()
{
    delete ui;
}

void ByteSettingsForm::open(int _devNum, int _byteNum)
{
    devNum = _devNum;
    byteNum = _byteNum;
    getWordTypeFromProfile(devNum, byteNum);
}

void ByteSettingsForm::on_bitBox_valueChanged(int arg1)
{//по изменению битбокса в форме, отправляем значение в byteButton
    emit setWordBit(devNum, byteNum, arg1);
}

void ByteSettingsForm::getWordTypeFromProfile(int _devNum, int _byteNum)
{//при создании формы bytesettingsform, отправляем запрос на длину слова в bytedefinition
    emit getWordType(_devNum, _byteNum);
}

void ByteSettingsForm::returnWordType(int _devNum, int _byteNum, int wordType)
{//возврат значения длины слова из bytedefinition в ответ на запрос из bytesettingsform
    //для того что-бы после открытия формы, в битбоксе было корректное значение из профиля
    if (devNum == _devNum && byteNum == _byteNum)
        this->ui->bitBox->setValue(wordType);
}

void ByteSettingsForm::on_pushButton_clicked()
{//по нажатию кнопки добавления маски в bytesettingsform, отправляем сигнал в bytedefinition на создание маски
    emit createMask(devNum, byteNum);
}


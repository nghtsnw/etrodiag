#include "bytesettingsform.h"
#include "ui_bytesettingsform.h"
#include <QDebug>
#include "bitmaskobj.h"
#include <QTableWidgetItem>

ByteSettingsForm::ByteSettingsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ByteSettingsForm)
{
    ui->setupUi(this);
    ui->masksWidget->setEditTriggers(0);
    ui->masksWidget->setHorizontalHeaderLabels(lst);
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
    requestAllMasks();
    if ((40-byteNum) >= 4) ui->bitBox->setMaximum(32);
    else if ((40-byteNum) >= 2) ui->bitBox->setMaximum(16);
    else if ((40-byteNum) >= 0) ui->bitBox->setMaximum(8);
    if (ui->masksWidget->rowCount() > 0)
        ui->bitBox->setDisabled(true);
    else ui->bitBox->setEnabled(true);
}

void ByteSettingsForm::addMaskItem(int _devNum, QString _devName, int _byteNum, QString _byteName, int _id, QString _paramName, QString _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogFlag, int _wordType)
{
    if (devNum == _devNum && byteNum == _byteNum)
    {
        bool findRow = 0;
        if (ui->masksWidget->rowCount() > 0)
        {
            for (int i = 0; i < ui->masksWidget->rowCount(); i++)
            {
                if (QString::number(_id,10) == ui->masksWidget->item(i,2)->text())
                findRow = true;
            }
        }
        if (!findRow)
        {
            ui->masksWidget->setRowCount(ui->masksWidget->rowCount()+1); //добавляем новую строку
            int row = ui->masksWidget->rowCount()-1;//определяем индекс строки
            QTableWidgetItem *nameItem = new QTableWidgetItem;
            nameItem->setText(_paramName);
            ui->masksWidget->setItem(row, 0, nameItem);
            QTableWidgetItem *valueItem = new QTableWidgetItem;
            valueItem->setText("waiting new data...");
            ui->masksWidget->setItem(row, 1, valueItem);
            QTableWidgetItem *idItem = new QTableWidgetItem;
            idItem->setText(QString::number(_id, 10));
            ui->masksWidget->setItem(row, 2, idItem);
            QTableWidgetItem *deleteItem = new QTableWidgetItem;
            deleteItem->setText("Delete");
            ui->masksWidget->setItem(row, 3, deleteItem);
        }
        if (ui->masksWidget->rowCount() > 0)
            ui->bitBox->setDisabled(true);
    }
}

void ByteSettingsForm::deleteMaskItem(int row)
{
    emit deleteMaskObj(devNum,byteNum, ui->masksWidget->item(row,2)->text().toInt(0,10));
    ui->masksWidget->removeRow(row);
    if (ui->masksWidget->rowCount() == 0)
        ui->bitBox->setEnabled(true);
}
void ByteSettingsForm::on_masksWidget_cellClicked(int row, int column)
{    
    if (column == 3) deleteMaskItem(row);
    else emit editMask(devNum, byteNum, ui->masksWidget->item(row,2)->text().toInt(0,10));
}//найти откуда вызывается второй раз

void ByteSettingsForm::on_bitBox_valueChanged(int arg1)
{//по изменению битбокса в форме, отправляем значение в byteButton

    if (arg1 == 8)
    {
        arg2index = 0;
    }
    else if (arg1 == 16)
    {
        arg2index = 1;
    }
    else if (arg1 == 24)//проскакиваем неиспользуемое значение
    {
        if (arg2index == 1)
        {
            arg2index = 2;
            ui->bitBox->setValue(32);
        }
        else
        {
            arg2index = 1;
            ui->bitBox->setValue(16);
        }
    }
    else if (arg1 == 32) arg2index = 2;
    emit setWordBit(devNum, byteNum, arg2index);
}

void ByteSettingsForm::getWordTypeFromProfile(int _devNum, int _byteNum)
{//при создании формы bytesettingsform, отправляем запрос на длину слова в bytedefinition
    emit getWordType(_devNum, _byteNum);
}

void ByteSettingsForm::returnWordType(int _devNum, int _byteNum, int wordType)
{//возврат значения длины слова из bytedefinition в ответ на запрос из bytesettingsform
    //для того что-бы после открытия формы, в битбоксе было корректное значение из профиля
    if (devNum == _devNum && byteNum == _byteNum)
    {
        if (wordType == 0) this->ui->bitBox->setValue(8);
        else if (wordType == 1) this->ui->bitBox->setValue(16);
        else if (wordType == 2) this->ui->bitBox->setValue(32);
    }
}

void ByteSettingsForm::on_pushButton_clicked()
{//по нажатию кнопки добавления маски в bytesettingsform, отправляем сигнал в bytedefinition на создание маски
    emit createMask(devNum, byteNum);
}

void ByteSettingsForm::requestAllMasks()
{
    emit requestAllMaskToList(devNum,byteNum,999);
}

void ByteSettingsForm::updateMasksList(int _devNum, QString _devName, int _byteNum, QString _byteName, int _wordData, int _id, QString parameterName, int _binRawValue, float _endValue, bool viewInLogFlag, bool isNewData, bool _drawGraphFlag, QString _drawGraphColor)
{
    if (devNum == _devNum && byteNum == _byteNum)
    {

        for (int i = 0; i < ui->masksWidget->rowCount(); i++) {
            if (ui->masksWidget->item(i,2)->text() == QString::number(_id, 10))
            {
                ui->masksWidget->item(i,0)->setText(parameterName);
                QString endValue2String;
                endValue2String.setNum(_endValue);
                ui->masksWidget->item(i,1)->setText(endValue2String);
            }
        }
    }
}

void ByteSettingsForm::updateHexWordData(int _devNum, int _byteNum, QString _txt)
{
    if (devNum == _devNum && byteNum == _byteNum && this->isVisible())
    {
        QString wordInfoString;
        if (ui->bitBox->value() == 8) wordInfoString = "Byte num: " + QString::number(byteNum);
        else if (ui->bitBox->value() == 16)
        {
            wordInfoString = "Word bytes: [%1, %2]";
            wordInfoString = wordInfoString.arg(QString::number(byteNum+1)).arg(QString::number(byteNum));
        }
        else if (ui->bitBox->value() == 32)
        {
            wordInfoString = "Word bytes: [%1, %2, %3, %4]";
            wordInfoString = wordInfoString.arg(QString::number(byteNum+3)).arg(QString::number(byteNum+2)).arg(QString::number(byteNum+1)).arg(QString::number(byteNum));
        }
        ui->hexNumber->setText("Dev num: " + QString("%1").arg(devNum,0,16).toUpper() + ", " + wordInfoString + ", Word data: " + _txt);
    }
}

void ByteSettingsForm::cleanForm()
{
    ui->bitBox->setMaximum(32);
    while (ui->masksWidget->rowCount() > 0)
            ui->masksWidget->removeRow(0);
    ui->hexNumber->clear();
}

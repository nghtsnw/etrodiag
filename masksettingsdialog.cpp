#include "masksettingsdialog.h"
#include "ui_masksettingsdialog.h"
#include <QList>
#include "bitsetform.h"
#include <QDebug>

maskSettingsDialog::maskSettingsDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::maskSettingsDialog)
{
    ui->setupUi(this);

}

maskSettingsDialog::~maskSettingsDialog()
{
    delete ui;
}

void maskSettingsDialog::requestDataOnId(int _devNum, int _byteNum, int _id)
{//после создания новой маски сразу посылаем сигнал на открытие формы masksettingsdialog, сообщая ей параметры маски которую нужно редактировать
    devNum = _devNum;
    byteNum = _byteNum;
    id = _id;
    emit requestMaskData(_devNum, _byteNum, _id);

    //ответный сигнал от masksettingsdialog с запросом всех параметров маски bitmaskobject
}

void maskSettingsDialog::getDataOnId(int _devNum, int _byteNum, int _id, QString _paramName, int _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogFlag, int _wordType)
{//ответный сигнал со всеми данными маски bitmaskobj в masksettingsdialog
    if (id == _id)
    {
        wordType = _wordType;
        ui->maskName->setText(_paramName);
        ui->binNum->setText(QString::number(_paramMask));
        //int _paramType = 0;
        ui->shiftTxt->setText(QString::number(_valueShift));
        ui->koeffTxt->setText(QString::number(_valueKoef));
        ui->checkBox->setTristate(_viewInLogFlag);
        emit getWordBit(_devNum, _byteNum);
        initBitButtonsAndCheckBoxes(wordType);
    }
}

void maskSettingsDialog::initBitButtonsAndCheckBoxes(int _wordType)
{//создаём чекбоксы, пронумеровываем, расставляем согласно имеющейся маске
    int wordBit = 8;
    if (_wordType == 0) wordBit = 8;
    else if (_wordType == 1) wordBit = 16;
    else if (_wordType == 2) wordBit = 32;
    else wordBit = 8;

    int x = 0, y = 0;
    for (int var = 0; var < wordBit; ++var, ++y) {
        if (y > 7) //по 8 бит в одной строке
        {
            y = 0;
            x++;
        }
        bitSetForm *bs = new bitSetForm(this);
        bitSetList.append(bs);
        this->ui->dynMaskForm->addWidget(bs, x, y);

        bs->setNumLabel(var);
        bs->show();

    }
    int mask = ui->binNum->text().toInt(nullptr,10);
    uint32_t one = 1;
    QListIterator<bitSetForm*> bitSetListIt(bitSetList);
    while (bitSetListIt.hasNext())
    {
        if (mask & one)
            bitSetListIt.next()->setCheckbox(true);
        else bitSetListIt.next()->setCheckbox(false);
        one = one << 1;
    }
}

void maskSettingsDialog::updateBitButtonsAndCheckBoxes()
{

}

void maskSettingsDialog::sendMask2Profile()//int _devNum, int _byteNum, int _id)
{//забор данных из формы masksettingsdialog и отправка в профиль bitmaskobj
    //if (id == _id)
    {
        QString _paramName = this->ui->maskName->text();
        int _paramMask =  this->ui->binNum->text().toInt(nullptr,10);
        int _paramType = 0;
        int _valueShift = ui->shiftTxt->text().toInt(nullptr,10);
        float _valueKoef = ui->koeffTxt->text().toFloat(nullptr);
        bool _viewInLogFlag = ui->checkBox->isTristate();
        emit sendMaskData(devNum, byteNum, id, _paramName, _paramMask, _paramType, _valueShift, _valueKoef, _viewInLogFlag, wordType);
    }
}

void maskSettingsDialog::killChildren() //очистка формы от объектов кнопок
{
    QList<bitSetForm*> devChildList = this->findChildren<bitSetForm*>();
    qDebug() << "Bitsetforms living = " << devChildList.size();
    QListIterator<bitSetForm*> devChildListIt(devChildList);
    while (devChildListIt.hasNext())
        devChildListIt.next()->~bitSetForm();
    devChildList = this->findChildren<bitSetForm*>();
    qDebug() << "Bitsetforms after holokost = " << devChildList.size();
}

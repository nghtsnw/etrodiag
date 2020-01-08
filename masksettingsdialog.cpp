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
    if (!(this->isVisible()))//костыль от непонятного глюка qtableview в bytesettingsform
    emit requestMaskData(_devNum, _byteNum, _id);

    //ответный сигнал от masksettingsdialog с запросом всех параметров маски bitmaskobject
}

void maskSettingsDialog::getDataOnId(int _devNum, int _byteNum, int _id, QString _paramName, QString _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogFlag, int _wordType)
{//ответный сигнал со всеми данными маски bitmaskobj в masksettingsdialog
    if (devNum == _devNum && byteNum == _byteNum && id == _id)
    {
        wordType = _wordType;
        ui->maskName->setText(_paramName);
        ui->binNum->setText(_paramMask);//QString::number(_paramMask));
        //int _paramType = 0;
        ui->shiftTxt->setText(QString::number(_valueShift));
        ui->koeffTxt->setText(QString::number(_valueKoef));
        ui->checkBox->setChecked(_viewInLogFlag);
        //emit getWordBit(_devNum, _byteNum);
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
    for (int var = wordBit; var > 0; --var, ++y) {
        if (y > 7) //по 8 бит в одной строке
        {
            y = 0;
            x++;
        }
        bitSetForm *bs = new bitSetForm(this);
        connect (bs, &bitSetForm::scanCheckboxesToMask, this, &maskSettingsDialog::scanCheckboxesToMask);
        connect (this, &maskSettingsDialog::setCheckBox, bs, &bitSetForm::setCheckBox);
        bitSetList.append(bs);
        qDebug() << "bitsetlist size after add new = " << bitSetList.size();
        this->ui->dynMaskForm->addWidget(bs, x, y);
        bs->setNumLabel(var-1);
        bs->show();
    }
    QString mask = ui->binNum->text();//.toInt(nullptr,10);
    uint32_t one = 1;
    QListIterator<bitSetForm*> bitSetListIt(bitSetList);
    bitSetListIt.toBack();
    for (int var = wordBit-1, i = 0; var > 0; --var, i++) {
        bool chk;
        int curmasksym = QString("%1").arg(mask[var],0,10).toInt(0,10);
        if (curmasksym & one)
            chk = true;
        else chk = false;
            emit setCheckBox(chk, i);
        //one = one << 1;
        }
    }

void maskSettingsDialog::scanCheckboxesToMask()
{
    QListIterator<bitSetForm*> bitSetListIt(bitSetList);
    bitSetListIt.toFront();
    int masktmp = 0;
    QString mask;
    qDebug() << "bitsetlist size = " << bitSetList.size();
    while (bitSetListIt.hasNext())
    {
        //qDebug() << "checkbox status = " << bitSetListIt.next()->checkboxStatus();
        masktmp = bitSetListIt.next()->checkboxStatus();
        qDebug() << "masktmp = " << masktmp;
        mask = mask + QString::number(masktmp,10);
    }
    ui->binNum->setText(mask);//QString::number(masktmp,10));
}

void maskSettingsDialog::updateBitButtonsAndCheckBoxes()
{

}

void maskSettingsDialog::sendMask2Profile()//int _devNum, int _byteNum, int _id)
{//забор данных из формы masksettingsdialog и отправка в профиль bitmaskobj
    //if (id == _id)
    {
        QString _paramName = this->ui->maskName->text();
        QString _paramMask =  this->ui->binNum->text();//.toInt(nullptr,10);
        int _paramType = 0;
        int _valueShift = ui->shiftTxt->text().toInt(nullptr,10);
        float _valueKoef = ui->koeffTxt->text().toFloat(nullptr);
        bool _viewInLogFlag = ui->checkBox->isChecked();
        emit sendMaskData(devNum, byteNum, id, _paramName, _paramMask, _paramType, _valueShift, _valueKoef, _viewInLogFlag, wordType);
    }
}

void maskSettingsDialog::killChildren() //очистка формы от объектов кнопок
{
    QList<bitSetForm*> devChildList = this->findChildren<bitSetForm*>();
    qDebug() << "Bitsetforms before delete = " << devChildList.size();
    QListIterator<bitSetForm*> devChildListIt(devChildList);
    while (devChildListIt.hasNext())
        devChildListIt.next()->~bitSetForm();
    devChildList = this->findChildren<bitSetForm*>();
    qDebug() << "Bitsetforms after delete = " << devChildList.size();
    bitSetList.clear();
}

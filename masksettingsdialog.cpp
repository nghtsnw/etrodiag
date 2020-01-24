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

void maskSettingsDialog::getDataOnId(int _devNum, int _byteNum, int _id, QString _paramName, QString _paramMask, int _paramType, double _valueShift, double _valueKoef, bool _viewInLogFlag, int _wordType)
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
    wordBit = 8;
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
        connect (this, &maskSettingsDialog::wordData2bitSetForm, bs, &bitSetForm::setCheckboxText);
        bitSetList.append(bs);
        //qDebug() << "bitsetlist size after add new = " << bitSetList.size();
        this->ui->dynMaskForm->addWidget(bs, x, y);
        bs->setNumLabel(var-1);
        bs->show();
    }
    QString mask = ui->binNum->text();//.toInt(nullptr,10);
    uint32_t one = 1;
    QListIterator<bitSetForm*> bitSetListIt(bitSetList);
    bitSetListIt.toBack();
    for (int var = wordBit-1, i = 0; var > -1; var--, i++) {
        bool chk;
        int curmasksym = QString("%1").arg(mask[var],0,10).toInt(0,10);
        if (curmasksym & one)
            chk = true;
        else chk = false;
            emit setCheckBox(chk, i);
        }
    }

void maskSettingsDialog::scanCheckboxesToMask()
{
    QListIterator<bitSetForm*> bitSetListIt(bitSetList);
    bitSetListIt.toFront();
    int masktmp = 0;
    QString mask;
    //qDebug() << "bitsetlist size = " << bitSetList.size();
    while (bitSetListIt.hasNext())
    {
        masktmp = bitSetListIt.next()->checkboxStatus();
        //qDebug() << "masktmp = " << masktmp;
        mask = mask + QString::number(masktmp,10);
    }
    ui->binNum->setText(mask);
    sendMask2Profile();
}

void maskSettingsDialog::liveDataSlot(int _devNum, QString _devName, int _byteNum, QString _byteName, int _wordData, int _id, QString parameterName, int _binRawValue, double _endValue, bool viewInLogFlag)
{//устанавливаем текст каждому чекбоксу, 0 или 1
    bool wordDataIntArray[wordBit];
    uint32_t mask = 1;
    QString endValueToString;
    int i = 0;
    for (i = 0; i < wordBit; i++) {
        if (_wordData & mask)
            wordDataIntArray[i] = true;
        else wordDataIntArray[i] = false;
        mask = mask << 1;
        emit wordData2bitSetForm(i, wordDataIntArray[i]);
    }    
    endValueToString.setNum(_endValue);
    ui->decimalInt->setText(endValueToString);

}

void maskSettingsDialog::sendMask2Profile()
{//забор данных из формы masksettingsdialog и отправка в профиль bitmaskobj
    {
        QString _paramName = this->ui->maskName->text();
        QString _paramMask =  this->ui->binNum->text();
        int _paramType = 0;
        int _valueShift = ui->shiftTxt->text().toInt(nullptr,10);
        float _valueKoef = ui->koeffTxt->text().toFloat(nullptr);
        bool _viewInLogFlag = ui->checkBox->isChecked();
        emit sendMaskData(devNum, "",byteNum, "", id, _paramName, _paramMask, _paramType, _valueShift, _valueKoef, _viewInLogFlag, wordType);
    }
}

void maskSettingsDialog::killChildren() //очистка формы от объектов кнопок
{
    QList<bitSetForm*> devChildList = this->findChildren<bitSetForm*>();
    //qDebug() << "Bitsetforms before delete = " << devChildList.size();
    QListIterator<bitSetForm*> devChildListIt(devChildList);
    while (devChildListIt.hasNext())
        devChildListIt.next()->~bitSetForm();
    devChildList = this->findChildren<bitSetForm*>();
    //qDebug() << "Bitsetforms after delete = " << devChildList.size();
    bitSetList.clear();
}

void maskSettingsDialog::on_maskName_editingFinished()
{
    sendMask2Profile();
}

void maskSettingsDialog::on_shiftTxt_editingFinished()
{
    sendMask2Profile();
}

void maskSettingsDialog::on_koeffTxt_editingFinished()
{
    sendMask2Profile();
}

void maskSettingsDialog::on_checkBox_stateChanged(int arg1)
{
    sendMask2Profile();
}

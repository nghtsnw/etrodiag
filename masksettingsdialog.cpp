#include "masksettingsdialog.h"
#include "ui_masksettingsdialog.h"
#include <QList>
#include "bitsetform.h"
#include <QDebug>
#include <QColorDialog>

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
    if (!(this->isVisible()))
    emit requestMaskData(_devNum, _byteNum, _id);
    //ответный сигнал от masksettingsdialog с запросом всех параметров маски bitmaskobject
}

void maskSettingsDialog::getDataOnId(int _devNum, int _byteNum, int _id, QString _paramName, QString _paramMask, int _paramType, double _valueShift, double _valueKoef, bool _viewInLogFlag, int _wordType, bool _drawGraphFlag, QString _drawGraphColor)
{//ответный сигнал со всеми данными маски bitmaskobj в masksettingsdialog
    if (devNum == _devNum && byteNum == _byteNum && id == _id)
    {
        wordType = _wordType;
        ui->maskName->setText(_paramName);
        ui->binNum->setText(_paramMask);
        ui->shiftTxt->setText(QString::number(_valueShift));
        ui->koeffTxt->setText(QString::number(_valueKoef));
        chkBoxStopSignal = true; //на время инициализации окна глушим переменную, что-бы не сработал сигнал state changed
        ui->logCheckBox->setChecked(_viewInLogFlag);
        ui->drawGraphCheckBox->setChecked(_drawGraphFlag);
        chkBoxStopSignal = false;
        QString style = "background: %1;";
        style = style.arg(_drawGraphColor);
        ui->drawGraphCheckBox->setStyleSheet(style);
        drawColor.setNamedColor(_drawGraphColor);
        initBitButtonsAndCheckBoxes(wordType);
    }
}

void maskSettingsDialog::initBitButtonsAndCheckBoxes(int _wordType)
{//создаём чекбоксы, пронумеровываем, расставляем согласно имеющейся маске
    killChildren();
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
        this->ui->dynamicMaskFormLay->addWidget(bs, x, y);
        bs->setNumLabel(var-1);
        bs->show();
    }
    QString mask = ui->binNum->text();
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
    while (bitSetListIt.hasNext())
    {
        masktmp = bitSetListIt.next()->checkboxStatus();        
        mask = mask + QString::number(masktmp,10);
    }
    ui->binNum->setText(mask);
    sendMask2Profile();
}

void maskSettingsDialog::liveDataSlot(int _devNum, QString _devName, int _byteNum, QString _byteName, uint32_t _wordData, int _id, QString parameterName, int _binRawValue, double _endValue, bool viewInLogFlag, bool isNewData)
{//устанавливаем текст каждому чекбоксу, 0 или 1
   if (devNum == _devNum && byteNum == _byteNum)
   {
    bool wordDataBoolBit;
    uint32_t mask = 1;
    QString endValueToString;
    int i = 0;
    //qDebug() << "endvalue = " << _endValue;
    for (i = 0; i < wordBit; i++)
    {
        if (_wordData & mask)
            wordDataBoolBit = true;
        else wordDataBoolBit = false;
        mask = mask << 1;
        emit wordData2bitSetForm(i, wordDataBoolBit);
        //qDebug() << "wordData2bitSetForm("<<i<<", "<<wordDataBoolBit<<");";
    }    
    endValueToString.setNum(_endValue);
    ui->decimalInt->setText(endValueToString);
}
}

void maskSettingsDialog::sendMask2Profile()
{//забор данных из формы masksettingsdialog и отправка в профиль bitmaskobj
    {
        QString _paramName = this->ui->maskName->text();
        QString _paramMask =  this->ui->binNum->text();
        int _paramType = 0;
        int _valueShift = ui->shiftTxt->text().toInt(nullptr,10);
        float _valueKoef = ui->koeffTxt->text().toFloat(nullptr);
        bool _viewInLogFlag = ui->logCheckBox->isChecked();
        bool _drawGraphFlag = ui->drawGraphCheckBox->isChecked();
        QString _drawGraphColor = drawColor.name();
        emit sendMaskData(devNum, "",byteNum, "", id, _paramName, _paramMask, _paramType, _valueShift, _valueKoef, _viewInLogFlag, wordType, _drawGraphFlag, _drawGraphColor);
    }
}

void maskSettingsDialog::killChildren() //очистка формы от объектов кнопок
{
    QList<bitSetForm*> devChildList = this->findChildren<bitSetForm*>();    
    QListIterator<bitSetForm*> devChildListIt(devChildList);
    while (devChildListIt.hasNext())
        devChildListIt.next()->~bitSetForm();
    devChildList = this->findChildren<bitSetForm*>();
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

void maskSettingsDialog::on_logCheckBox_stateChanged(int)
{
    if (!chkBoxStopSignal) sendMask2Profile();
}

void maskSettingsDialog::on_drawGraphCheckBox_stateChanged(int)
{
    if (!chkBoxStopSignal)
    {
        QString style = "background: %1;";
        if (ui->drawGraphCheckBox->isChecked())
        {
            drawColor = QColorDialog::getColor(Qt::white, this, "Choose color");
            if (!drawColor.isValid())
            {
                ui->drawGraphCheckBox->setChecked(false);
                drawColor.setNamedColor("#ffffff");
            }
        }
        else drawColor.setNamedColor("#ffffff");
        style = style.arg(drawColor.name());
        ui->drawGraphCheckBox->setStyleSheet(style);
        sendMask2Profile();
    }
}

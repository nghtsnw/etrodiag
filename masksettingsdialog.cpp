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

void maskSettingsDialog::getDataOnId(bitMaskDataStruct &bitMask)
{//ответный сигнал со всеми данными маски bitmaskobj в masksettingsdialog
    if (devNum == bitMask.devNum && byteNum == bitMask.byteNum && id == bitMask.id)
    {
        wordType = bitMask.wordType;
        QString wordInfoString;
        if (wordType == 0) wordInfoString = "Byte num: " + QString::number(byteNum);
        else if (wordType == 1)
        {
            wordInfoString = "Word bytes: [%1, %2]";
            wordInfoString = wordInfoString.arg(QString::number(byteNum+1)).arg(QString::number(byteNum));
        }
        else if (wordType == 2)
        {
            wordInfoString = "Word bytes: [%1, %2, %3, %4]";
            wordInfoString = wordInfoString.arg(QString::number(byteNum+3)).arg(QString::number(byteNum+2)).arg(QString::number(byteNum+1)).arg(QString::number(byteNum));
        }
        ui->wordInfo->setText("Dev num: " + QString::number(devNum) + ", " + wordInfoString);
        ui->maskName->setText(bitMask.paramName);
        binMaskInTxt = bitMask.paramMask;
        ui->shiftTxt->setText(QString::number(bitMask.valueShift));
        ui->koeffTxt->setText(QString::number(bitMask.valueKoef));
        chkBoxStopSignal = true; //на время инициализации окна глушим переменную, что-бы не сработал сигнал state changed
        ui->logCheckBox->setChecked(bitMask.viewInLogFlag);
        ui->drawGraphCheckBox->setChecked(bitMask.drawGraphFlag);
        chkBoxStopSignal = false;
        QString style = "background: %1;";
        style = style.arg(bitMask.drawGraphColor);
        ui->drawGraphCheckBox->setStyleSheet(style);
        drawColor.setNamedColor(bitMask.drawGraphColor);
        initBitButtonsAndCheckBoxes(wordType);
    }
}

void maskSettingsDialog::initBitButtonsAndCheckBoxes(int _wordType)
{//создаём чекбоксы, пронумеровываем, расставляем согласно имеющейся маске
    killChildren();//кек
    wordBit = 8;
    if (_wordType == 0) wordBit = 8;
    else if (_wordType == 1) wordBit = 16;
    else if (_wordType == 2) wordBit = 32;
    else wordBit = 8;

    int x = 0, y = 0;
    for (int var = wordBit; var > 0; --var, ++y)
    {
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
    QString mask = binMaskInTxt;
    uint32_t one = 1;
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
    binMaskInTxt = mask;
    sendMask2Profile();
}

void maskSettingsDialog::liveDataSlot(bitMaskDataStruct &bitMask)
{//устанавливаем текст каждому чекбоксу, 0 или 1
   if (devNum == bitMask.devNum && byteNum == bitMask.byteNum)
   {
    bool wordDataBoolBit;
    uint32_t mask = 1;
    QString endValueToString;
    int i = 0;
    //qDebug() << "endvalue = " << _endValue;
    for (i = 0; i < wordBit; i++)
    {
        if (bitMask.wordData & mask)
            wordDataBoolBit = true;
        else wordDataBoolBit = false;
        mask = mask << 1;
        emit wordData2bitSetForm(i, wordDataBoolBit);
        //qDebug() << "wordData2bitSetForm("<<i<<", "<<wordDataBoolBit<<");";
    }    
    endValueToString.setNum(bitMask.endValue);
    ui->decimalInt->setText(endValueToString);
}
}

void maskSettingsDialog::sendMask2Profile()
{//забор данных из формы masksettingsdialog и отправка в профиль bitmaskobj
    {
        QString _paramName = this->ui->maskName->text();
        QString _paramMask = binMaskInTxt;
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

void maskSettingsDialog::on_checkAllButton_clicked()
{
    for (int var = wordBit-1, i = 0; var > -1; var--, i++)
    {
        emit setCheckBox(true, i);
    }
    scanCheckboxesToMask();
}

void maskSettingsDialog::on_uncheckAllButton_clicked()
{
    for (int var = wordBit-1, i = 0; var > -1; var--, i++)
    {
        emit setCheckBox(false, i);
    }
    scanCheckboxesToMask();
}

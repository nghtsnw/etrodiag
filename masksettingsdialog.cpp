#include "masksettingsdialog.h"
#include "qdatetime.h"
#include "ui_masksettingsdialog.h"
#include <QList>
#include "bitsetform.h"
#include <QDebug>
#include <QColorDialog>
#include "global.h"

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
{ //после создания новой маски сразу посылаем сигнал на открытие формы masksettingsdialog, сообщая ей параметры маски которую нужно редактировать
    devNum = _devNum;
    byteNum = _byteNum;
    id = _id;
    if (!(this->isVisible())) {
        emit requestMaskData(_devNum, _byteNum, _id);
    }
    //ответный сигнал от masksettingsdialog с запросом всех параметров маски bitmaskobject
}

void maskSettingsDialog::getDataOnId(s_parameterMask mask)
{ //ответный сигнал со всеми данными маски bitmaskobj в masksettingsdialog
    if (devNum == mask.devNum && byteNum == mask.byteNum && id == mask.id)
    {
        maskToEdit = mask; //длину слова, имена и значение форма не показывает - храним маску целиком
        wordType = mask.wordType;
        QString wordInfoString;
        if (wordType == 0) {
            wordInfoString = tr("Byte num: %1").arg(byteNum);
        }
        else if (wordType == 1)
        {
            wordInfoString = tr("Word bytes: [%1, %2]");
            wordInfoString = wordInfoString.arg(QString::number(byteNum + 1)).arg(QString::number(byteNum));
        }
        else if (wordType == 2)
        {
            wordInfoString = tr("Word bytes: [%1, %2, %3, %4]");
            wordInfoString = wordInfoString.arg(QString::number(byteNum + 3)).arg(QString::number(byteNum + 2)).arg(QString::number(byteNum + 1)).arg(QString::number(byteNum));
        }
        ui->wordInfo->setText(tr("Dev num: %1, %2").arg(QString::number(devNum)).arg(wordInfoString));
        ui->maskName->setText(mask.parameterName);
        binMaskInTxt = mask.parameterMask;
        ui->shiftTxt->setText(QString::number(mask.valueShift));
        ui->koeffTxt->setText(QString::number(mask.valueKoef));
        chkBoxStopSignal = true; //на время инициализации окна глушим переменную, что-бы не сработал сигнал state changed
        ui->logCheckBox->setChecked(mask.viewInLogFlag);
        ui->drawGraphCheckBox->setChecked(mask.drawGraphFlag);
        chkBoxStopSignal = false;
        drawColor = QColor::fromString(mask.drawGraphColor);
        updateDrawGraphColorStyle();
        initBitButtonsAndCheckBoxes(wordType);
    }
}

void maskSettingsDialog::updateDrawGraphColorStyle()
{ //цвет показываем только у включённого чекбокса: без выбранного цвета фон берём у родителя
    if (!ui->drawGraphCheckBox->isChecked() || !drawColor.isValid()) {
        ui->drawGraphCheckBox->setStyleSheet(QString());
        return;
    }
    ui->drawGraphCheckBox->setStyleSheet(QStringLiteral("background: %1;").arg(drawColor.name()));
}

void maskSettingsDialog::initBitButtonsAndCheckBoxes(int _wordType)
{ //создаём чекбоксы, пронумеровываем, расставляем согласно имеющейся маске
    killChildren();//кек
    wordBit = 8;
    if (_wordType == 0) {
        wordBit = 8;
    }
    else if (_wordType == 1) {
        wordBit = 16;
    }
    else if (_wordType == 2) {
        wordBit = 32;
    }
    else {
        wordBit = 8;
    }
    int x = 0, y = 0;
    for (int var = wordBit; var > 0; --var, ++y)
    {
        if (y > 7) //по 8 бит в одной строке (ячейки бит узкие, поэтому строка помещается в колонку)
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
        bs->setNumLabel(var - 1);
        bs->show();
    }
    QString mask = binMaskInTxt;
    uint32_t one = 1;
    for (int var = wordBit, i = 0; var > 0; var--, i++) {
        bool chk;
        int curmasksym = 0;
        if (mask.size() >= var) {
            curmasksym = QString("%1").arg(mask[var - 1]).toInt(0, 10);
        }
        if (curmasksym & one) {
            chk = true;
        }
        else {
            chk = false;
        }
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
        mask = mask + QString::number(masktmp, 10);
    }
    binMaskInTxt = mask;
    sendMask2Profile();
}

void maskSettingsDialog::liveDataSlot(QDateTime, s_parameterMask mask)
{ //устанавливаем текст каждому чекбоксу, 0 или 1
    if (devNum == mask.devNum && byteNum == mask.byteNum)
    {
        bool wordDataBoolBit;
        uint32_t bit = 1;
        QString endValueToString;
        int i = 0;
        //qDebug() << "endvalue = " << _endValue;
        for (i = 0; i < wordBit; i++)
        {
            if (mask.wordData & bit) {
                wordDataBoolBit = true;
            }
            else {
                wordDataBoolBit = false;
            }
            bit = bit << 1;
            emit wordData2bitSetForm(i, wordDataBoolBit);
            //qDebug() << "wordData2bitSetForm("<<i<<", "<<wordDataBoolBit<<");";
        }
        endValueToString.setNum(mask.endValue);
        ui->decimalInt->setText(endValueToString);
        maskToEdit.endValue = mask.endValue; //держим последнее значение: его же отдадим при сохранении
    }
}

void maskSettingsDialog::sendMask2Profile()
{ //отправляем в профиль маску с изменёнными в форме полями: длину слова, имена устройства/байта
  //и значение форма не показывает, поэтому берём их из загруженной маски
    {
        s_parameterMask mask = maskToEdit;
        mask.id = id;
        mask.devNum = devNum;
        mask.byteNum = byteNum;
        mask.parameterName = this->ui->maskName->text();
        mask.parameterMask = binMaskInTxt;
        mask.valueShift = ui->shiftTxt->text().toInt(nullptr, 10);
        mask.valueKoef = ui->koeffTxt->text().toFloat(nullptr);
        mask.viewInLogFlag = ui->logCheckBox->isChecked();
        mask.drawGraphFlag = ui->drawGraphCheckBox->isChecked();
        //цвет графика сохраняем только выбранный: у чекбокса без цвета его нет
        mask.drawGraphColor = drawColor.isValid() ? drawColor.name() : QString();
        emit sendMaskData(mask);
    }
}

void maskSettingsDialog::killChildren() //очистка формы от объектов кнопок
{
    QList<bitSetForm*> devChildList = this->findChildren<bitSetForm*>();
    QListIterator<bitSetForm*> devChildListIt(devChildList);
    while (devChildListIt.hasNext()) {
        delete devChildListIt.next();
    }
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
    if (!chkBoxStopSignal) {
        sendMask2Profile();
    }
}

void maskSettingsDialog::on_drawGraphCheckBox_stateChanged(int)
{
    if (!chkBoxStopSignal)
    {
        if (ui->drawGraphCheckBox->isChecked())
        {
            drawColor = QColorDialog::getColor(Qt::white, this, "Choose color");
            if (!drawColor.isValid())
            { //цвет не выбран - снимаем галочку, но фон чекбокса остаётся как у родителя
                chkBoxStopSignal = true;
                ui->drawGraphCheckBox->setChecked(false);
                chkBoxStopSignal = false;
            }
        }
        else {
            drawColor.setNamedColor("#ffffff");
        }
        updateDrawGraphColorStyle();
        sendMask2Profile();
    }
}

void maskSettingsDialog::on_checkAllButton_clicked()
{
    for (int var = wordBit - 1, i = 0; var > -1; var--, i++)
    {
        emit setCheckBox(true, i);
    }
    scanCheckboxesToMask();
}

void maskSettingsDialog::on_uncheckAllButton_clicked()
{
    for (int var = wordBit - 1, i = 0; var > -1; var--, i++)
    {
        emit setCheckBox(false, i);
    }
    scanCheckboxesToMask();
}

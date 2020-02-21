#include "devsettingsform.h"
#include "ui_devsettingsform.h"
#include <QRegExpValidator>
#include <QDebug>
#include <QPushButton>
#include <QGridLayout>
#include "bytesettingsform.h"
#include "bytebutton.h"

devSettingsForm::devSettingsForm(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::devSettingsForm)
{
    m_ui->setupUi(this);
    //запиливаем валидатор только ради того что бы работал сигнал завершения редактирования формы имени
    QRegExpValidator *val = new QRegExpValidator(this);
    devNameForm->setValidator(val);
}

devSettingsForm::~devSettingsForm()
{
    delete m_ui;
}

void devSettingsForm::killChildren() //очистка формы от объектов кнопок
{
    QList<byteButton*> devChildList = this->findChildren<byteButton*>(); //времянка для поиска бага
    qDebug() << "dev childrens living = " << devChildList.size();
    QListIterator<byteButton*> devChildListIt(devChildList);
    while (devChildListIt.hasNext())
        devChildListIt.next()->~byteButton();
    devChildList = this->findChildren<byteButton*>(); //времянка для поиска бага
    qDebug() << "dev childrens after delete = " << devChildList.size();

    m_ui->devNameEdit->clear();
}

void devSettingsForm::setDevName(int id, QString devName) //получаем имя из профиля в форму
{
    if (id == devNum && devName != m_ui->devNameEdit->text())
    m_ui->devNameEdit->setText(devName);
}

void devSettingsForm::initByteButtons(int id, QVector<int> data)
{//сначала создаём кнопки в форме, потом делаем запрос длины слова из профиля для каждой
    devNum = id;
    for (int x = 2, y = 0, count = data.size()-1; count >= 0; count--, y++) //забиваем раскладку динамически
    {
        if (y > 4)
        {
            x++;
            y = 0;
        }
        byteButton *byteBtn = new byteButton;
        connect(byteBtn, &byteButton::setButton, this, &devSettingsForm::retranslateByteButtonSetStatus);
        connect(this, &devSettingsForm::setByteButtonStatus, byteBtn, &byteButton::setButtonStatus);
        connect(byteBtn, &byteButton::openByteSettingsForm, this, &devSettingsForm::openByteSettingsFormTX); //ретрансляция для дальнейшей передачи в майнвиндов
        connect(this, &devSettingsForm::updateBtnDataSIG, byteBtn, &byteButton::updateBtnData);
        connect (this, &devSettingsForm::wordType2ByteBtn, byteBtn, &byteButton::setWordType);
        connect (byteBtn, &byteButton::wordDataFullHex, this, &devSettingsForm::wordDataFullHex);
        QString hexBtnTxt = QString("%1").arg(data.at(count),0,16).toUpper();
        byteBtn->setText(hexBtnTxt);
        byteBtn->setByteNum(id, count);
        m_ui->gridLayoutButtonBox->addWidget(byteBtn,x,y);
    }
    for (int count = 0; count < data.size(); count++)
        emit initByteButtonsWordLeight(id, count); //запрос длины слова из bytedefinition

}

void devSettingsForm::updByteButtons(int _devNum, QVector<int> data)
{
    emit updateBtnDataSIG(_devNum, data);
}

void devSettingsForm::on_devNameEdit_editingFinished()
{
    QString text = m_ui->devNameEdit->text();
    emit returnDevNameAfterEdit(devNum, text);
}

//void devSettingsForm::openByteSettingsFormRX(int devNum, int byteNum)
//{//принимаем сигнал от кнопки, отправляем то же в mainwindow чтобы открыть bytesettingsform
//    emit openByteSettingsFormTX(devNum, byteNum);
//}

void devSettingsForm::wordTypeChangeRX(int _devNum, int _byteNum, int wordType)
{//прослойка для передачи сигнала из bytesettingsform в bytebutton
    emit wordType2ByteBtn(_devNum, _byteNum, wordType);
}

void devSettingsForm::retranslateByteButtonSetStatus(int byteNum, bool status)
{//установка кнопок enable или disable по запросу других кнопок, в зависимости от длины слова
    emit setByteButtonStatus(byteNum, status);
}




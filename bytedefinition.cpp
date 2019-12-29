#include "bytedefinition.h"
#include <QDebug>
#include "bitmaskobj.h"
//Создаётся для каждого байта при инициализации устройства.
//Данные отсюда будут подтягиваться в гуй (ещё не создал форму) параметров байта, и сюда же сохраняться.
//Вместе с параметрами устройства (dynamicbaseprofile) данные будут сохраняться в файл.
byteDefinition::byteDefinition()//(QObject *parent)
{
    devNum = 0;
    th_byteNum = 0;
    th_data = 0;
}

byteDefinition::byteDefinition(int numDev, int byteNum, int data)
{
    //qDebug() << "new byte def";
    devNum = numDev;
    th_byteNum = byteNum;
    th_data = data;
    QString byteName = ("<empty>");

}

 byteDefinition::~byteDefinition()
{

}


void byteDefinition::setWordBitRX(int _devNum, int _byteNum, int _argBit)
{//по изменению битбокса в форме bytesettingsform, отправляем значение в byteDefinition
    //qDebug() << "device "<<devNum<<", byte "<<th_byteNum<<", word type "<<wordType;
    //qDebug() << "received devNum " << _devNum<<", received byteNum "<< _byteNum;
    if (devNum == _devNum && th_byteNum == _byteNum)
    {
    wordType = _argBit;
    //qDebug() << "wordType "<<wordType<<", byte "<<th_byteNum<<", dev "<<devNum;//"device "<<devNum<<", byte "<<th_byteNum<<", word type "<<wordType;
    }
}

void byteDefinition::getWordType(int _devNum, int _byteNum)
{//при создании формы bytesettingsform, отправляем запрос на длину слова в bytedefinition
    if (devNum == _devNum && th_byteNum == _byteNum)
    emit returnWordType(_devNum, _byteNum, wordType);    
}

void byteDefinition::createNewMask(int _devNum, int _byteNum)
{//по нажатию кнопки добавления маски в bytesettingsform, отправляем сигнал в bytedefinition на создание маски
    //найти всех детей типа bitMaskObj, что-бы присвоить маске айди
    if (_devNum == devNum && _byteNum == th_byteNum)
    {
        int id = 0;
        bool notFoundFlag = 0;
        QList<bitMaskObj*> bytedefChildList = this->findChildren<bitMaskObj*>();
        QListIterator<bitMaskObj*> bytedefChildListIt(bytedefChildList);
        //qDebug() << "Children of bytedef = " << bytedefChildList.size();

        if (bytedefChildList.size() != 0)
        {
        for (int n = 0; notFoundFlag == 0 ; n++)
        {
            notFoundFlag = 1;
            while (bytedefChildListIt.hasNext())
            {//прогоняем число n по всем id
                if (n == bytedefChildListIt.next()->id)
                notFoundFlag = 0;//если маска с таким id хоть раз попалась, то скидываем флаг
            }
            bytedefChildListIt.toFront();
            //если после работы цикла флаг остался в состоянии 1, то назначаем ненайденый id новой маске
            //цикл for прекратится по условию достижения notFoundFlag != 0
            if (notFoundFlag == 1)
            {
                id = n;
            }
        }
        }

        bitMaskObj *mask = new bitMaskObj;
        mask->wordType = wordType;
        mask->setParent(this);
        connect (mask, &bitMaskObj::mask2byteSettingsForm, this, &byteDefinition::mask2FormRX); //открытие формы
        connect (this, &byteDefinition::requestMaskDataTX, mask, &bitMaskObj::maskToForm);//запрос от формы
        connect (mask, &bitMaskObj::maskToFormSIG, this, &byteDefinition::maskData2FormRX);//ответ форме
        connect (this, &byteDefinition::sendDataToProfileTX, mask, &bitMaskObj::sendMaskToProfile);
        connect (this, &byteDefinition::wordData2Mask, mask, &bitMaskObj::calculateValue);
        mask->newMaskObj(devNum, th_byteNum, id);
    }
}

void byteDefinition::mask2FormRX(int _devNum, int _byteNum, int _id)
{//после создания новой маски сразу посылаем сигнал на открытие формы masksettingsdialog, сообщая ей параметры маски которую нужно редактировать
    if (devNum == _devNum && th_byteNum == _byteNum)
    emit mask2FormTX(_devNum, _byteNum, _id);
}

void byteDefinition::requestMaskDataRX(int _devNum, int _byteNum, int _id)
{//ответный сигнал от masksettingsdialog с запросом всех параметров маски bitmaskobject
    if (devNum == _devNum && th_byteNum == _byteNum)
    emit requestMaskDataTX(_devNum, _byteNum, _id);
}

void byteDefinition::maskData2FormRX(int _devNum, int _byteNum, int _id, QString _paramName, int _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogFlag, int _wordType)
{//ответный сигнал со всеми данными маски bitmaskobj в masksettingsdialog
    if (devNum == _devNum && th_byteNum == _byteNum)
    emit maskData2FormTX(_devNum, _byteNum, _id, _paramName, _paramMask, _paramType, _valueShift, _valueKoef, _viewInLogFlag, _wordType);
}

void byteDefinition::sendDataToProfileRX(int _devNum, int _byteNum, int _id, QString _paramName, int _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogFlag)
{//забор данных из формы masksettingsdialog и отправка в профиль bitmaskobj
    if (devNum == _devNum && th_byteNum == _byteNum)
    emit sendDataToProfileTX(_devNum, _byteNum, _id, _paramName, _paramMask, _paramType, _valueShift, _valueKoef, _viewInLogFlag);
}

void byteDefinition::calcWordData(int _devNum, QVector<int> data)
{//формируем слово из полных данных устройства и заданной длины, и рассылаем слово маскам
    if (_devNum == devNum)
    {
    if (wordType == 0)
        wordData = (data.at(th_byteNum));
    else if (wordType == 1)
        wordData = (data.at(th_byteNum)) + (data.at(th_byteNum+1));
    else if (wordType == 2)
        wordData = (data.at(th_byteNum)) + (data.at(th_byteNum+1))
                + (data.at(th_byteNum+2)) + (data.at(th_byteNum+3));
    emit wordData2Mask(wordData);
    }
}

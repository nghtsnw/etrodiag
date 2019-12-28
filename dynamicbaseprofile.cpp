#include "dynamicbaseprofile.h"
#include <QVector>
#include "bytedefinition.h"
#include <QDebug>

//Базовый новый профиль для одного устройства
//тут будут храниться промежуточные переменные с именами и значениями для текущего нового профиля одного устройства
//изменение и отображение будет производиться через devsettingsform

dynamicBaseProfile::dynamicBaseProfile(QObject *parent) : QObject(parent)
{
    QString profileName = "New profile";

}

//void dynamicBaseProfile::addDataDev(int devNum, QVector<int> data)
//{
//    int n = 0;
//    while (n<data.size()) //для каждого байта этого устройства создаём объект-описание
//    {
//        byteDefinition *byteDef = new byteDefinition(devNum, n, data.at(n));
//        connect (this, &dynamicBaseProfile::setWordBitTX, byteDef, &byteDefinition::setWordBitRX);// SIGNAL(setWordBitRX()), byteDef, SLOT()); //ВСЁ ПРОПАЛО
//        connect (this, &dynamicBaseProfile::getWordType, byteDef, &byteDefinition::getWordType);
//        connect (byteDef, &byteDefinition::returnWordType, this, &dynamicBaseProfile::returnWordTypeRX);
//        connect (this, &dynamicBaseProfile::createNewMaskTX, byteDef, &byteDefinition::createNewMask);
//        connect (byteDef, &byteDefinition::mask2FormTX, this, &dynamicBaseProfile::mask2FormRX);
//        n++;
//    }

//}

QString dynamicBaseProfile::getDeviceName()
{
    return deviceName;
}

void dynamicBaseProfile::setDeviceName(QString name)
{
    deviceName = name;
}

//void dynamicBaseProfile::setWordBitRX(int devNum, int byteNum, int argBit)
//{//по изменению битбокса в форме bytesettingsform, отправляем значение в bytedefinition
//    emit setWordBitTX(devNum,byteNum,argBit);
//}

//void dynamicBaseProfile::getWordTypeFromProfileRetranslator(int devNum, int byteNum)
//{//при создании формы bytesettingsform, отправляем запрос на длину слова в bytedefinition
//    emit getWordType(devNum, byteNum);
//}

//void dynamicBaseProfile::returnWordTypeRX(int _devNum, int _byteNum, int wordType)
//{//возврат значения длины слова из bytedefinition в ответ на запрос из bytesettingsform
//    emit returnWordTypeTX(_devNum, _byteNum, wordType);
//}

//void dynamicBaseProfile::createNewMaskRX(int _devNum, int _byteNum)
//{//по нажатию кнопки добавления маски в bytesettingsform, отправляем сигнал в bytedefinition на создание маски
//    emit createNewMaskTX(_devNum, _byteNum);
//}

//void dynamicBaseProfile::mask2FormRX(int _devNum, int _byteNum, int _id)
//{//после создания новой маски сразу посылаем сигнал на открытие формы masksettingsdialog, сообщая ей параметры маски которую нужно редактировать
//    emit mask2FormTX(_devNum, _byteNum, _id);
//}

//void dynamicBaseProfile::requestMaskDataRX(int _devNum, int _byteNum, int _id)
//{//ответный сигнал от masksettingsdialog с запросом всех параметров маски bitmaskobject
//    emit requestMaskDataTX(_devNum, _byteNum, _id);
//}

//void dynamicBaseProfile::maskData2FormRX(int _devNum, int _byteNum, int _id, QString _paramName, int _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogFlag)
//{//ответный сигнал со всеми данными маски bitmaskobj в masksettingsdialog
//    emit maskData2FormTX(_devNum, _byteNum, _id, _paramName, _paramMask, _paramType, _valueShift, _valueKoef, _viewInLogFlag);
//}

//void dynamicBaseProfile::sendDataToProfileRX(int _devNum, int _byteNum, int _id, QString _paramName, int _paramMask, int _paramType, int _valueShift, float _valueKoef, bool _viewInLogFlag)
//{//забор данных из формы masksettingsdialog и отправка в профиль bitmaskobj
//    emit sendDataToProfileTX(_devNum, _byteNum, _id, _paramName, _paramMask, _paramType, _valueShift, _valueKoef, _viewInLogFlag);
//}

#include "txtmaskobj.h"

txtmaskobj::txtmaskobj(QObject *parent) : QObject(parent)
{

}

txtmaskobj::txtmaskobj(QList<QString> mask)
{
    lst = mask;
    devNum = mask.at(4);
    byteNum = mask.at(5);
    id = mask.at(1);
}

txtmaskobj::~txtmaskobj()
{

}

QList<QString> txtmaskobj::getTxtMask()
{
    return lst;
}

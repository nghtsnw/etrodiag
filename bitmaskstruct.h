#ifndef BITMASKSTRUCT_H
#define BITMASKSTRUCT_H

#include "qobject.h"

struct bitMaskDataStruct{
        int devNum;
        QString devName;
        int byteNum;
        QString byteName;
        int id;
        int wordType = 0;
        uint32_t wordData = 0;
        QString paramName = "Parameter";
        QString paramMask = "00000000";
        int paramType = 0;
        double valueShift = 0;
        double valueKoef = 1;
        bool viewInLogFlag = true;
        bool drawGraphFlag = false;
        QString drawGraphColor;
        bool isNewData = true;
        int binRawValue;
        double endValue;
    };

#endif // BITMASKSTRUCT_H

#ifndef FRAMECHECK_H
#define FRAMECHECK_H

#include <QVector>
#include "global.h"

//Проверки целостности кадра протокола: контрольная сумма и байты маркера.
//Функции чистые - не зависят от состояния разбора, поэтому их можно тестировать отдельно.
namespace framecheck {

//Сумма байтов кадра от fromPosition до предпоследнего включительно (контрольная сумма протокола)
uint8_t checksum(const QVector<int> &frame, int fromPosition);

//Совпадает ли посчитанная сумма с последним байтом кадра
bool isChecksumValid(uint8_t calculated, const QVector<int> &frame);

//Считает сумму и сверяет её с последним байтом кадра
bool isChecksumValid(const QVector<int> &frame, int fromPosition);

//Совпадает ли начало кадра с маркером протокола.
//markerPacketBeginSize == 0 - маркера нет, проверять нечего
bool isMarkerValid(const QVector<int> &frame, const s_protocolDescription &protocol);

}

#endif // FRAMECHECK_H

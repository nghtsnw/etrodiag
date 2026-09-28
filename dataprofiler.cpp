#include "dataprofiler.h"
#include "framecheck.h"
#include "protocolsettings.h"
#include "qdatetime.h"
#include <QDebug>

dataprofiler::dataprofiler(QWidget *parent) : QObject(parent)
{
    connect(this, &dataprofiler::setTime, this, [this](QDateTime t) {
        currentTime = t;
    });
}

void dataprofiler::setModel(const ProtocolSettings *model)
{
    m_model = model;
}

void dataprofiler::getByte(int byteFromBuf)
{
    if (!m_model) {
        return; //разбор не подключён к модели - принимать нечего
    }
    const s_protocolDescription &protocol = m_model->protocol();
    emit ready4read(false);
    frameMsg.append(byteFromBuf);
    if (frameMsg.size() >= protocol.markerPacketBeginSize) //если принятые данные уже можно проверять на наличие маркера - начинаем обработку
    {
        const bool marker = framecheck::isMarkerValid(frameMsg, protocol);
        if (frameMsg.size() == protocol.packetSize) { //когда набрался весь пакет
            handleFullFrame(marker);
        }
    }
    emit ready4read(true);
    emit readNext();
}

void dataprofiler::handleFullFrame(bool marker)
{ //кадр набран целиком: проверяем контрольную сумму и отдаём в гуй
    const s_protocolDescription &protocol = m_model->protocol();
    if (marker) {
        calculatedCRC = framecheck::checksum(frameMsg, protocol.calcCRCFromPosition);
        if (framecheck::isChecksumValid(calculatedCRC, frameMsg)) {
            if (!m_model->settings().readFromFileFlag) {
                currentTime = QDateTime::currentDateTime();
            }
            emit deviceData(currentTime, frameMsg); //пакет сформирован - отправляем и очищаем буфер
            frameMsg.clear();
        }
        else {
            emit badCRC(calculatedCRC, frameMsg);
        }
    }
    if (!frameMsg.isEmpty()) {
        frameMsg.removeFirst(); //выкидываем байт, пока контрольная сумма не сойдётся
    }
}

#ifndef PROTOCOLSETTINGS_H
#define PROTOCOLSETTINGS_H

#include "global.h"

//Единственное хранилище протокола и настроек связи.
//Владелец - SettingsDialog (он же их редактирует и загружает из профиля),
//остальные модули получают только константный доступ через SettingsDialog::model()
//и своих копий не держат. Класс без QObject: подписки на изменения не нужны,
//читатели берут значения в момент использования.
class ProtocolSettings
{
public:
    const s_protocolDescription &protocol() const;
    const s_Settings &settings() const;

    void setProtocol(const s_protocolDescription &protocol);
    void setSettings(const s_Settings &settings);

    //Доступ для правки отдельных полей. Выдаётся только владельцу модели
    s_Settings &mutableSettings();

private:
    s_protocolDescription m_protocol;
    s_Settings m_settings;
};

#endif // PROTOCOLSETTINGS_H

#include "protocolsettings.h"

const s_protocolDescription &ProtocolSettings::protocol() const
{
    return m_protocol;
}

const s_Settings &ProtocolSettings::settings() const
{
    return m_settings;
}

void ProtocolSettings::setProtocol(const s_protocolDescription &protocol)
{
    m_protocol = protocol;
}

void ProtocolSettings::setSettings(const s_Settings &settings)
{
    m_settings = settings;
}

s_Settings &ProtocolSettings::mutableSettings()
{
    return m_settings;
}

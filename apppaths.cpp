#include "apppaths.h"
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>

namespace {

QString computeAppHomeDir()
{
#ifdef Q_OS_ANDROID
    return QStandardPaths::standardLocations(QStandardPaths::DataLocation)[1] + QDir::separator();
#else
    //На Windows и остальных платформах каталоги лежат рядом с исполняемым файлом
    return QCoreApplication::applicationDirPath() + QDir::separator();
#endif
}

}

QString apppaths::appHomeDir()
{
    static const QString dir = computeAppHomeDir(); //каталог приложения за время работы не меняется
    return dir;
}

QString apppaths::profilesDir()
{
    return appHomeDir() + QStringLiteral("Profiles");
}

QString apppaths::logsDir()
{
    return appHomeDir() + QStringLiteral("Logs");
}

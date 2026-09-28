#include "testapppaths.h"
#include <QCoreApplication>
#include <QDir>
#include <QtTest>
#include "apppaths.h"

void TestAppPaths::homeDirIsExecutableDirectory()
{
#ifdef Q_OS_ANDROID
    QSKIP("на Android каталог берётся из DataLocation");
#else
    QCOMPARE(apppaths::appHomeDir(), QCoreApplication::applicationDirPath() + QDir::separator());
#endif
}

void TestAppPaths::subDirsAreBuiltFromHomeDir()
{
    //Оба каталога строятся от одного корня, иначе профили и логи разъедутся
    QVERIFY(!apppaths::appHomeDir().isEmpty());
    QVERIFY(apppaths::appHomeDir().endsWith(QDir::separator()));
    QCOMPARE(apppaths::profilesDir(), apppaths::appHomeDir() + QStringLiteral("Profiles"));
    QCOMPARE(apppaths::logsDir(), apppaths::appHomeDir() + QStringLiteral("Logs"));
}

void TestAppPaths::homeDirIsStable()
{
    //Каталог кэшируется: повторные вызовы не должны пересчитывать путь
    const QString first = apppaths::appHomeDir();
    QCOMPARE(apppaths::appHomeDir(), first);
    QCOMPARE(apppaths::profilesDir(), first + QStringLiteral("Profiles"));
}

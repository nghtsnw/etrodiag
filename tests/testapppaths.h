#ifndef TESTAPPPATHS_H
#define TESTAPPPATHS_H

#include <QObject>

//Тесты путей приложения: каталог программы, профили, логи
class TestAppPaths : public QObject
{
    Q_OBJECT

private slots:
    void homeDirIsExecutableDirectory();
    void subDirsAreBuiltFromHomeDir();
    void homeDirIsStable();
};

#endif // TESTAPPPATHS_H

#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QDebug>
#include <QDir>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator myappTranslator;
    if (myappTranslator.load(QLocale(), "etrodiag", "_", ":/", ".qm"))
    a.installTranslator(&myappTranslator);

    MainWindow w;
    w.show();
    return a.exec();
}

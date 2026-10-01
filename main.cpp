#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QIcon>
#include <QDebug>
#include <QDir>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(QStringLiteral(":/etrodiag_icon.png"))); //иконка программы

    QTranslator myappTranslator;
    if (myappTranslator.load(QLocale(), "etrodiag", "_", ":/", ".qm"))
    a.installTranslator(&myappTranslator);

    MainWindow w;
    w.show();
    return a.exec();
}

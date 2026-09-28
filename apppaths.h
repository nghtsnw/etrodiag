#ifndef APPPATHS_H
#define APPPATHS_H

#include <QString>

//Пути приложения. Считаются в одном месте, чтобы каталоги профилей и логов
//не расходились между модулями (раньше каждый класс вычислял их сам).
namespace apppaths {

//Каталог программы: папка исполняемого файла, на Android - каталог данных приложения
QString appHomeDir();

//Каталог профилей .eag
QString profilesDir();

//Каталог логов
QString logsDir();

}

#endif // APPPATHS_H

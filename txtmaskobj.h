#ifndef TXTMASKOBJ_H
#define TXTMASKOBJ_H

#include <QObject>

class txtmaskobj : public QObject
{
    Q_OBJECT
public:
    explicit txtmaskobj(QObject *parent = nullptr);
    txtmaskobj(QList<QString> mask);
    QString devNum;
    QString byteNum;
    QString id;
    QList<QString> lst;
    QList<QString> getTxtMask();
    ~txtmaskobj();

signals:

};

#endif // TXTMASKOBJ_H

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();

private:
    Ui::AboutDialog *ui;
    QPixmap *pixmap = new QPixmap(":/etrodiag.png");
    const QString BUILDV =  tr("Build from ") + QStringLiteral(__DATE__ " " __TIME__);
};

#endif // ABOUTDIALOG_H

#ifndef BITSETFORM_H
#define BITSETFORM_H

#include <QWidget>
#include <QTimer>

namespace Ui {
class bitSetForm;
}

class bitSetForm : public QWidget
{
    Q_OBJECT

public:
    explicit bitSetForm(QWidget *parent = nullptr);
    ~bitSetForm();
    void setNumLabel(int num);
    bool checkboxStatus();
    void setCheckBox(bool bit, int id);
    void setCheckboxText(int i, bool data);

signals:
    void scanCheckboxesToMask();
private slots:
    void on_bitCheckBox_clicked();
    void backgroundReturn();

private:
    QString oldCheckboxText;
    QTimer *timer = new QTimer;
    Ui::bitSetForm *ui;
};

#endif // BITSETFORM_H

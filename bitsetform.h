#ifndef BITSETFORM_H
#define BITSETFORM_H

#include <QWidget>

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
    void setCheckboxText(QString data);

signals:
    void scanCheckboxesToMask();
private slots:
    void on_bitCheckBox_clicked();

private:
    Ui::bitSetForm *ui;
};

#endif // BITSETFORM_H

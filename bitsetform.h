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
    void setCheckbox(bool bit);
    void setCheckboxText(QChar ch);


private:
    Ui::bitSetForm *ui;
};

#endif // BITSETFORM_H

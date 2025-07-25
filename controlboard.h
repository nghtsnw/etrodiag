#ifndef CONTROLBOARD_H
#define CONTROLBOARD_H

#include <QWidget>

namespace Ui {
class ControlBoard;
}

class ControlBoard : public QWidget
{
    Q_OBJECT

public:
    explicit ControlBoard(QWidget *parent = nullptr);
    ~ControlBoard();

signals:
    void controlCommand(int varNumber, bool action); //action: 0 - down, 1 - up

private:
    Ui::ControlBoard *ui;
};

#endif // CONTROLBOARD_H

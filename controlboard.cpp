#include "controlboard.h"
#include "ui_controlboard.h"

ControlBoard::ControlBoard(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ControlBoard)
{
    ui->setupUi(this);
    connect(ui->v1Up, &QPushButton::clicked, this, [this](){emit controlCommand(1,1);});
    connect(ui->v2Up, &QPushButton::clicked, this, [this](){emit controlCommand(2,1);});
    connect(ui->v3Up, &QPushButton::clicked, this, [this](){emit controlCommand(3,1);});
    connect(ui->v4Up, &QPushButton::clicked, this, [this](){emit controlCommand(4,1);});
    connect(ui->v1Down, &QPushButton::clicked, this, [this](){emit controlCommand(1,0);});
    connect(ui->v2Down, &QPushButton::clicked, this, [this](){emit controlCommand(2,0);});
    connect(ui->v3Down, &QPushButton::clicked, this, [this](){emit controlCommand(3,0);});
    connect(ui->v4Down, &QPushButton::clicked, this, [this](){emit controlCommand(4,0);});
}

ControlBoard::~ControlBoard()
{
    delete ui;
}

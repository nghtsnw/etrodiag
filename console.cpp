#include "console.h"
#include <QScrollBar>

Console::Console(QWidget *parent) :
    QPlainTextEdit(parent)
{
    document()->setMaximumBlockCount(1000);
    QPalette p = palette();
    p.setColor(QPalette::Base, Qt::darkBlue);
    p.setColor(QPalette::Text, Qt::gray);
    setPalette(p);
}

void Console::putData(const QByteArray &data)
{
    if (data.size() > 1) {
        insertPlainText((data.toHex(':')) + ('\n'));
    }
    else if (data.size() == 1) {
        insertPlainText((data.toHex()) + (':'));
    }
    QScrollBar *bar = verticalScrollBar();
    bar->setValue(bar->maximum());
}

void Console::putIntData(const QVector<uint8_t> data)
{
    QString string;
    for (auto i : data)
    {
        if (i < 0x10) {
            string += '0';
        }
        string += QString::number(i, 16).toUpper() + ':';
    }
    string += '\n';
    insertPlainText(string);
    QScrollBar *bar = verticalScrollBar();
    bar->setValue(bar->maximum());
}

void Console::setLocalEchoEnabled(bool set)
{
    m_localEchoEnabled = set;
}

void Console::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Backspace:
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
        break;
    default:
        if (m_localEchoEnabled) {
            QPlainTextEdit::keyPressEvent(e);
        }
    }
}

void Console::mousePressEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
    setFocus();
}

void Console::mouseDoubleClickEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
}

void Console::contextMenuEvent(QContextMenuEvent *e)
{
    Q_UNUSED(e)
}

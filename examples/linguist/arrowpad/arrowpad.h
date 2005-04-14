#ifndef ARROWPAD_H
#define ARROWPAD_H

#include <QWidget>

class QPushButton;

class ArrowPad : public QWidget
{
    Q_OBJECT

public:
    ArrowPad(QWidget *parent = 0);

private:
    QPushButton *upButton;
    QPushButton *downButton;
    QPushButton *leftButton;
    QPushButton *rightButton;
};

#endif

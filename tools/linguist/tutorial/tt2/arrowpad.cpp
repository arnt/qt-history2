/****************************************************************
**
** Implementation of ArrowPad class, translation tutorial 2
**
****************************************************************/

#include "arrowpad.h"

#include <QGridLayout>
#include <QPushButton>

ArrowPad::ArrowPad(QWidget *parent)
    : QWidget(parent)
{
    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(new QPushButton(tr("&Up")), 0, 1);
    grid->addWidget(new QPushButton(tr("&Left")), 1, 0);
    grid->addWidget(new QPushButton(tr("&Right")), 1, 2);
    grid->addWidget(new QPushButton(tr("&Down")), 2, 1);
}


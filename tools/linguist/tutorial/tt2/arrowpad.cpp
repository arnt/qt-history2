/****************************************************************
**
** Implementation of ArrowPad class, translation tutorial 2
**
****************************************************************/

#include "arrowpad.h"

#include <QGridLayout>
#include <QPushButton>

ArrowPad::ArrowPad(QWidget *parent)
    : QGridWidget(3, Qt::Horizontal, parent)
{
    skip();
    (void) new QPushButton(tr("&Up"), this);
    skip();
    (void) new QPushButton(tr("&Left"), this);
    skip();
    (void) new QPushButton(tr("&Right"), this);
    skip();
    (void) new QPushButton(tr("&Down"), this);
    skip();
}

void ArrowPad::skip()
{
    (void) new QWidget(this);
}

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
    QGridLayout *l = static_cast<QGridLayout*>(layout());
    l->setMargin(10);
    l->setSpacing(10);

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

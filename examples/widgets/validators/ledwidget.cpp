/****************************************************************************
**
** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "ledwidget.h"

LEDWidget::LEDWidget(QWidget *parent)
    : QLabel(parent), onPixmap(":/ledon.png"), offPixmap(":/ledoff.png")
{
    setPixmap(offPixmap);
    flashTimer.setInterval(200);
    flashTimer.setSingleShot(true);
    connect(&flashTimer, SIGNAL(timeout()), this, SLOT(extinguish()));
};

void LEDWidget::extinguish()
{
    setPixmap(offPixmap);
}

void LEDWidget::flash()
{
    setPixmap(onPixmap);
    flashTimer.start();
}


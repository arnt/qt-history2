/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>

#include "arrowpad.h"

ArrowPad::ArrowPad(QWidget *parent)
    : QWidget(parent)
{
    upButton = new QPushButton(tr("&Up"));
    downButton = new QPushButton(tr("&Down"));
    leftButton = new QPushButton(tr("&Left"));
    rightButton = new QPushButton(tr("&Right"));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(upButton, 0, 1);
    mainLayout->addWidget(leftButton, 1, 0);
    mainLayout->addWidget(rightButton, 1, 2);
    mainLayout->addWidget(downButton, 2, 1);
    setLayout(mainLayout);
}

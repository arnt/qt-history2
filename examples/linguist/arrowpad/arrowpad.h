/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef ARROWPAD_H
#define ARROWPAD_H

#include <QWidget>

QT_DECLARE_CLASS(QPushButton)

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

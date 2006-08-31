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

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "ui_controller.h"

class CarInterface;

class Controller : public QWidget
{
    Q_OBJECT

public:
    Controller(QWidget *parent = 0);

protected:
    void timerEvent(QTimerEvent *event);

private slots:
    void on_accelerate_clicked();
    void on_decelerate_clicked();
    void on_left_clicked();
    void on_right_clicked();

private:
    Ui::Controller ui;
    CarInterface *car;
};

#endif


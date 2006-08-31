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

#include <QtGui>

#include "controller.h"
#include "car_interface_p.h"

Controller::Controller(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    car = new CarInterface("com.trolltech.CarExample", "/Car",
                           QDBusConnection::sessionBus(), this);
    startTimer(1000);
}

void Controller::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    if (car->isValid())
        ui.label->setText("connected");
    else
        ui.label->setText("disconnected");
}

void Controller::on_accelerate_clicked()
{
    car->accelerate();
}

void Controller::on_decelerate_clicked()
{
    car->decelerate();
}

void Controller::on_left_clicked()
{
    car->turnLeft();
}

void Controller::on_right_clicked()
{
    car->turnRight();
}

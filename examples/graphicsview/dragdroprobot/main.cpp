/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
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

#include "coloritem.h"
#include "robot.h"

#include <math.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    QGraphicsScene scene(-200, -200, 400, 400);

    for (int i = 0; i < 10; ++i) {
        ColorItem *item = new ColorItem;
        item->setPos(::sin((i * 6.28) / 10.0) * 150,
                     ::cos((i * 6.28) / 10.0) * 150);

        scene.addItem(item);
    }

    Robot *robot = new Robot;
    robot->scale(1.2, 1.2);
    robot->setPos(0, -20);
    scene.addItem(robot);

    QGraphicsView view(&scene);
    view.setRenderHint(QPainter::Antialiasing);
    view.setBackgroundBrush(QColor(230, 200, 167));
    view.setWindowTitle("Drag and Drop Robot");
    view.show();

    return app.exec();
}

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

#ifndef WORLDTIMECLOCK_H
#define WORLDTIMECLOCK_H

#include <QTime>
#include <QWidget>

class WorldTimeClock : public QWidget
{
    Q_OBJECT

public:
    WorldTimeClock(QWidget *parent = 0);

public slots:
    void setTimeZone(int hourOffset);

signals:
    void updated(QTime currentTime);

protected:
    void paintEvent(QPaintEvent *event);

private:
    int timeZoneOffset;
};

#endif

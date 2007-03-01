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

#ifndef LEDWIDGET_H
#define LEDWIDGET_H

#include <QLabel>
#include <QPixmap>
#include <QTimer>

class LEDWidget : public QLabel
{
    Q_OBJECT
public:
    LEDWidget(QWidget *parent = 0);
public slots:
    void flash();

private slots:
    void extinguish();

private:
    QPixmap onPixmap, offPixmap;
    QTimer flashTimer;
};

#endif // LEDWIDGET_H

/****************************************************************************
** $Id: //depot/qt/main/examples/pdasingle/launcher.cpp $
**
** Qt/Embedded single application "launcher" demo for small devices.
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qwidget.h>
#include <qpixmap.h>
#include <qwsmouse_qws.h>

class QTimer;

class Calibrate : public QWidget
{
    Q_OBJECT
public:
    Calibrate();
    ~Calibrate();

private:
    void moveCrosshair( QPoint pt );
    void paintEvent( QPaintEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );

private slots:
    void timeout();

private:
    QPixmap *logo;
    QWSPointerCalibrationData cd;
    QWSPointerCalibrationData::Location location;
    QPoint crossPos;
    QPoint penPos;
    QPixmap saveUnder;
    QTimer *timer;
    int dx;
    int dy;
};


/****************************************************************************
** $Id: //depot/qt/qws/util/qws/qws.h#4 $
**
** Definition of Qt/FB central server classes
**
** Created : 991025
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#ifndef QWSMOUSE_H
#define QWSMOUSE_H

#ifndef QT_H
#include <qobject.h>
#endif // QT_H

class QWSPointerCalibrationData
{
public:
    enum Location { TopLeft = 0, BottomLeft = 1, BottomRight = 2, TopRight = 3,
		    Center = 4, LastLocation = Center };
    QPoint devPoints[5];
    QPoint screenPoints[5];
};

class QMouseHandler : public QObject {
    Q_OBJECT
public:
    QMouseHandler();
    virtual ~QMouseHandler();

    virtual void clearCalibration() {}
    virtual void calibrate( QWSPointerCalibrationData * ) {}

signals:
    void mouseChanged(const QPoint& pos, int bstate);
};

class QCalibratedMouseHandler : public QMouseHandler
{
    Q_OBJECT
public:
    QCalibratedMouseHandler();

    virtual void clearCalibration();
    virtual void calibrate( QWSPointerCalibrationData * );

protected:
    QPoint transform( const QPoint & );

private:
    int a, b, c;
    int d, e, f;
    int s;
};

#endif


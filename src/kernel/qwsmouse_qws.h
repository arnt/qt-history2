/****************************************************************************
** $Id: //depot/qt/qws/util/qws/qws.h#4 $
**
** Definition of Qt/FB central server classes
**
** Created : 991025
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

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
    void readCalibration();
    void writeCalibration();
    QPoint transform( const QPoint & );

private:
    int a, b, c;
    int d, e, f;
    int s;
};

#endif


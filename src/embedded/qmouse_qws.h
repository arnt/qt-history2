/****************************************************************************
** $Id: $
**
** Definition of Qt/Embedded mouse driver
**
** Created : 20020220
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QMOUSE_qws_H
#define QMOUSE_qws_H

#ifndef QT_H
#include <qobject.h>
#include <qpointarray.h>
#endif // QT_H

class QWSPointerCalibrationData
{
public:
    enum Location { TopLeft = 0, BottomLeft = 1, BottomRight = 2, TopRight = 3,
		    Center = 4, LastLocation = Center };
    QPoint devPoints[5];
    QPoint screenPoints[5];
};

class QWSMouseHandler
{
public:
    QWSMouseHandler( const QString &driver = QString::null, const QString &device = QString::null );
    virtual ~QWSMouseHandler();

    virtual void clearCalibration() {}
    virtual void calibrate( QWSPointerCalibrationData * ) {}
    virtual void getCalibration( QWSPointerCalibrationData * ) {}

    void limitToScreen( QPoint &pt );
    void mouseChanged(const QPoint& pos, int bstate);
    const QPoint &pos() const { return mousePos; }

protected:
    QPoint &mousePos;
};


class QWSCalibratedMouseHandler : public QWSMouseHandler
{
public:
    QWSCalibratedMouseHandler( const QString &device = QString::null, const QString &device = QString::null );

    virtual void clearCalibration();
    virtual void calibrate( QWSPointerCalibrationData * );
    virtual void getCalibration( QWSPointerCalibrationData * );

    bool sendFiltered( const QPoint &, int button );
    QPoint transform( const QPoint & );

protected:
    void readCalibration();
    void writeCalibration();
    void setFilterSize( int );

private:
    int a, b, c;
    int d, e, f;
    int s;
    QPointArray samples;
    unsigned int currSample;
    unsigned int numSamples;
};

#endif


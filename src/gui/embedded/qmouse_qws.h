/****************************************************************************
**
** Definition of Qt/Embedded mouse driver.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMOUSE_QWS_H
#define QMOUSE_QWS_H

#ifndef QT_H
#include "qobject.h"
#include "qpointarray.h"
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
    QWSMouseHandler(const QString &driver = QString::null, const QString &device = QString::null);
    virtual ~QWSMouseHandler();

    virtual void clearCalibration() {}
    virtual void calibrate(const QWSPointerCalibrationData *) {}
    virtual void getCalibration(QWSPointerCalibrationData *) const {}

    void limitToScreen(QPoint &pt);
    void mouseChanged(const QPoint& pos, int bstate);
    const QPoint &pos() const { return mousePos; }

protected:
    QPoint &mousePos;
};


class QWSCalibratedMouseHandler : public QWSMouseHandler
{
public:
    QWSCalibratedMouseHandler(const QString &device = QString::null, const QString &device = QString::null);

    virtual void clearCalibration();
    virtual void calibrate(const QWSPointerCalibrationData *);
    virtual void getCalibration(QWSPointerCalibrationData *) const;

    bool sendFiltered(const QPoint &, int button);
    QPoint transform(const QPoint &);

protected:
    void readCalibration();
    void writeCalibration();
    void setFilterSize(int);

private:
    int a, b, c;
    int d, e, f;
    int s;
    QPointArray samples;
    int currSample;
    int numSamples;
};

#endif // QMOUSE_QWS_H

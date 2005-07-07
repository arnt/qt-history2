/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qmouse_qws.h"
#include "qwindowsystem_qws.h"
#include "qscreen_qws.h"
#include "qapplication.h"
#include "qtextstream.h"
#include "qfile.h"
#include "qdebug.h"
/*!
    \class QWSMouseHandler qwsmouse_qws.h
    \brief The QWSMouseHandler class is a mouse driver for Qtopia Core.

    \ingroup qws

    The mouse driver handles events from system devices and generates
    mouse events.

    A QWSMouseHandler will usually open some system device in its
    constructor, create a QSocketNotifier on that opened device and
    when it receives data, it will call mouseChanged() to send the
    event to Qtopia Core for relaying to clients.
*/


/*!
  \fn void QWSMouseHandler::suspend()

  Must be implemented in subclasses to suspend reading and handling of mouse events.

  \sa resume()
*/

/*!
  \fn void QWSMouseHandler::resume()

  Must be implemented in subclasses to resume handling mouse events.

  \sa suspend()
*/

/*!
    \fn virtual void QWSMouseHandler::getCalibration(QWSPointerCalibrationData *) const

    \internal
*/

/*!
    \fn const QPoint &QWSMouseHandler::pos() const

    Returns the mouse position.
*/

/*!
    Constructs a mouse handler. This becomes the primary mouse
    handler.

    Note that once created, mouse handlers are controlled by the
    system and should not be deleted.

    The \a driver and \a device arguments are not used by this base
    class.
*/
QWSMouseHandler::QWSMouseHandler(const QString &, const QString &)
    : mousePos(QWSServer::mousePosition)
{
    QWSServer::setMouseHandler(this);
}

/*!
    Destroys the mouse handler. You should not call this directly.
*/
QWSMouseHandler::~QWSMouseHandler()
{
}

/*!
    Ensures that the given point, \a pt is within the screen's
    boundaries, changing \a pt if necessary.
*/

void QWSMouseHandler::limitToScreen(QPoint &pt)
{
    pt.setX(qMin(qt_screen->deviceWidth()-1, qMax(0, pt.x())));
    pt.setY(qMin(qt_screen->deviceHeight()-1, qMax(0, pt.y())));
}


/*!
    When a mouse event occurs this function is called with the mouse's
    position in \a pos, the state of its buttons in \a bstate, and the wheel motion in \a wheel.
*/
void QWSMouseHandler::mouseChanged(const QPoint &pos, int bstate, int wheel)
{
    mousePos = pos;
    QWSServer::sendMouseEvent(pos, bstate, wheel);
}

/*!
    \fn QWSMouseHandler::clearCalibration()

    This method is reimplemented in the calibrated mouse handler to
    clear calibration information. This version does nothing.
*/

/*!
    \fn QWSMouseHandler::calibrate(const QWSPointerCalibrationData *data)

    This method is reimplemented in the calibrated mouse handler to
    set calibration information passed in \a data, for example, from
    the Qtopia calibration screen. This version does nothing.
*/


/*
 *
 */

QWSCalibratedMouseHandler::QWSCalibratedMouseHandler(const QString &, const QString &)
    : samples(5), currSample(0), numSamples(0)
{
    clearCalibration();
    readCalibration();
}

void QWSCalibratedMouseHandler::getCalibration(QWSPointerCalibrationData *cd) const
{
    QPoint screen_tl = cd->screenPoints[QWSPointerCalibrationData::TopLeft];
    QPoint screen_br = cd->screenPoints[QWSPointerCalibrationData::BottomRight];

    int tlx = (s * screen_tl.x() - c) / a;
    int tly = (s * screen_tl.y() - f) / e;
    cd->devPoints[QWSPointerCalibrationData::TopLeft] = QPoint(tlx,tly);
    cd->devPoints[QWSPointerCalibrationData::BottomRight] =
        QPoint(tlx - (s * (screen_tl.x() - screen_br.x()) / a),
                tly - (s * (screen_tl.y() - screen_br.y()) / e));
}

void QWSCalibratedMouseHandler::clearCalibration()
{
    a = 1;
    b = 0;
    c = 0;
    d = 0;
    e = 1;
    f = 0;
    s = 1;
}

void QWSCalibratedMouseHandler::writeCalibration()
{
    QString calFile = "/etc/pointercal";
#ifndef QT_NO_TEXTSTREAM
    QFile file(calFile);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream t(&file);
        t << a << " " << b << " " << c << " ";
        t << d << " " << e << " " << f << " " << s;
    } else
#endif
    {
        qDebug() << "Could not save calibration:" << calFile;
    }
}

void QWSCalibratedMouseHandler::readCalibration()
{
    QString calFile = "/etc/pointercal";
#ifndef QT_NO_TEXTSTREAM
    QFile file(calFile);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream t(&file);
        t >> a >> b >> c >> d >> e >> f >> s;
    } else
#endif
    {
        qDebug() << "Could not read calibration:" <<calFile;
    }
}

void QWSCalibratedMouseHandler::calibrate(const QWSPointerCalibrationData *cd)
{
    QPoint dev_tl = cd->devPoints[QWSPointerCalibrationData::TopLeft];
    QPoint dev_br = cd->devPoints[QWSPointerCalibrationData::BottomRight];
    QPoint screen_tl = cd->screenPoints[QWSPointerCalibrationData::TopLeft];
    QPoint screen_br = cd->screenPoints[QWSPointerCalibrationData::BottomRight];

    s = 1 << 16;

    a = s * (screen_tl.x() - screen_br.x()) / (dev_tl.x() - dev_br.x());
    b = 0;
    c = s * screen_tl.x() - a * dev_tl.x();

    d = 0;
    e = s * (screen_tl.y() - screen_br.y()) / (dev_tl.y() - dev_br.y());
    f = s * screen_tl.y() - e * dev_tl.y();

    writeCalibration();
}

QPoint QWSCalibratedMouseHandler::transform(const QPoint &p)
{
    QPoint tp;

    tp.setX((a * p.x() + b * p.y() + c) / s);
    tp.setY((d * p.x() + e * p.y() + f) / s);

    return tp;
}

void QWSCalibratedMouseHandler::setFilterSize(int s)
{
    samples.resize(s);
    numSamples = 0;
    currSample = 0;
}

bool QWSCalibratedMouseHandler::sendFiltered(const QPoint &p, int button)
{
    if (!button) {
        if (numSamples >= samples.count())
            mouseChanged(mousePos, 0);
        currSample = 0;
        numSamples = 0;
        return true;
    }

    bool sent = false;
    samples[currSample] = p;
    numSamples++;
    if (numSamples >= samples.count()) {
        int maxd = 0;
        int ignore = 0;
        // throw away the "worst" sample
        for (int i = 0; i < samples.count(); i++) {
            int d = (mousePos - samples[i]).manhattanLength();
            if (d > maxd) {
                maxd = d;
                ignore = i;
            }
        }
        bool first = true;
        QPoint pos;
        // average the rest
        for (int i = 0; i < samples.count(); i++) {
            if (ignore != i) {
                if (first) {
                    pos = samples[i];
                    first = false;
                } else {
                    pos += samples[i];
                }
            }
        }
        pos /= (int)(samples.count() - 1);
        pos = transform(pos);
        if (pos != mousePos || numSamples == samples.count()) {
            mousePos = pos;
            mouseChanged(mousePos, button);
            sent = true;
        }
    }
    currSample++;
    if (currSample >= samples.count())
        currSample = 0;

    return sent;
}


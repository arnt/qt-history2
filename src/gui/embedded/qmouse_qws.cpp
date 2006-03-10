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
    \class QWSPointerCalibrationData

    \brief The QWSPointerCalibrationData class is a container for data
    used in calibration of a mouse handler.

    \ingroup qws

    QWSPointerCalibrationData stores device and screen coordinates in
    the devPoints and screenPoints variables, respectively.

    A calibration program should create an instance of this class,
    fill the devPoints and screenPoints variables with its device and
    screen coordinates, and pass the QWSPointerCalibrationData object
    to a an instance of the QWSMouseHandler class using the
    QWSMouseHandler::calibrate() function.

    \sa QWSMouseHandler, QWSCalibratedMouseHandler
*/

/*!
    \variable QWSPointerCalibrationData::devPoints
    \brief the raw device coordinates for each value of the Location enum.
*/

/*!
    \variable QWSPointerCalibrationData::screenPoints
    \brief the logical screen coordinates for each value of the Location enum.
*/

/*!
    \enum QWSPointerCalibrationData::Location

    This enum describes logical positions specified by the values
    stored in the devPoints and screenPoints variables.

    \value TopLeft           Index of the top left corner of the screen.
    \value BottomLeft     Index of the bottom left corner of the screen.
    \value BottomRight   Index of the bottom right corner of the screen.
    \value TopRight         Index of the top right corner of the screen.
    \value Center            Index of the center of the screen.
    \value LastLocation   Last index in the pointer arrays.
*/

/*!
    \class QWSMouseHandler
    \ingroup qws

    \brief The QWSMouseHandler class implements a mouse driver in
    Qtopia Core.

    A mouse driver handles events from system devices and generates
    mouse events. Custom mouse drivers can be added by subclassing the
    QMouseDriverPlugin class, using the QMouseDriverFactory class to
    dynamically load the driver into the application.

    A QWSMouseHandler object will usually open some system device, and
    create a QSocketNotifier object for that device. The
    QSocketNotifier class provides support for monitoring activity on
    a file descriptor. When the socket notifier receives data, it will
    call the mouse handler's mouseChanged() function to send the event
    to the \l {Qtopia Core} server application for relaying to
    clients.

    If you are creating a handler for a device that needs calibration
    or noise reduction, such as a touchscreen, use the
    QWSCalibratedMouseHandler subclass instead to take advantage of
    the calibrate() and clearCalibration() functions.

    Note that when deriving from the QWSMouseHandler class, the
    resume() and suspend() functions must be reimplemented to control
    the flow of mouse input. The default implementation does nothing;
    reimplementations of these functions typically call the
    QSocketNotifier::setEnabled() function to enable or disable the
    socket notifier, respectively.

    In addition, QWSMouseHandler provides the limitToScreen() function
    ensuring that the given position is within the screen's boundaries
    (changing the position if necessary), and the pos() function
    returning the current mouse position.

    \sa {Pointer Handling}, {qtopiacore/mousecalibration}{Mouse
    Calibration Example}, {Qtopia Core}
*/


/*!
    \fn void QWSMouseHandler::suspend()

    Suspends reading and handling of mouse events.

    Note that this function must be reimplemented in subclasses to
    control the flow of mouse input.  The default implementation does
    nothing; reimplementations typically call the
    QSocketNotifier::setEnabled() function to disable the socket
    notifier.

    \sa resume()
*/

/*!
    \fn void QWSMouseHandler::resume()

    Resumes reading and handling mouse events.

    Note that this function must be reimplemented in subclasses to
    control the flow of mouse input.  The default implementation does
    nothing; reimplementations typically call the
    QSocketNotifier::setEnabled() function to enable the socket
    notifier.

    \sa suspend()
*/

/*!
    \fn virtual void QWSMouseHandler::getCalibration(QWSPointerCalibrationData *) const
    \internal
*/

/*!
    \fn const QPoint &QWSMouseHandler::pos() const

    Returns the current mouse position.

    \sa mouseChanged(), limitToScreen()
*/

/*!
    Constructs a mouse handler which becomes the primary mouse
    handler.

    Note that once created, mouse handlers are controlled by the
    system and should not be deleted. The \a driver and \a device
    arguments are passed by the QWS_MOUSE_PROTO environment variable.
*/
QWSMouseHandler::QWSMouseHandler(const QString &, const QString &)
    : mousePos(QWSServer::mousePosition)
{
    QWSServer::setMouseHandler(this);
}

/*!
    Destroys this mouse handler.

    Do not call this function directly; it should only be called when
    the application terminates and from within \l {Qtopia Core}.
*/
QWSMouseHandler::~QWSMouseHandler()
{
}

/*!
    Ensures that the given \a position is within the screen's
    boundaries, changing the \a position if necessary.

    \sa pos()
*/

void QWSMouseHandler::limitToScreen(QPoint &position)
{
    position.setX(qMin(qt_screen->deviceWidth() - 1, qMax(0, position.x())));
    position.setY(qMin(qt_screen->deviceHeight() - 1, qMax(0, position.y())));
}


/*!
    Notifies the system of a new mouse event.

    This function updates the current mouse position and sends the
    event to the \l {Qtopia Core} server application for delivery to
    the correct widget.

    The given \a position is the global position of the mouse cursor.
    The \a state parameter is a bitmask of the Qt::MouseButton enum's
    values, indicating which mouse buttons are pressed. The \a wheel
    parameter is the delta value of the mouse wheel as returned by
    QWheelEvent::delta().

    A subclass must call this function whenever it wants to deliver
    a new mouse event.

    \sa pos()
*/
void QWSMouseHandler::mouseChanged(const QPoint &position, int state, int wheel)
{
    mousePos = position;
    QWSServer::sendMouseEvent(position, state, wheel);
}

/*!
    \fn QWSMouseHandler::clearCalibration()

    This virtual function allows subclasses of QWSMouseHandler to
    clear the calibration information. The default implementation does
    nothing.

    \sa QWSCalibratedMouseHandler::clearCalibration(), calibrate()
*/

/*!
    \fn QWSMouseHandler::calibrate(const QWSPointerCalibrationData *data)

    This virtual function allows subclasses of QWSMouseHandler to set
    the calibration information passed in the given \a data. The
    default implementation does nothing.

    \sa QWSCalibratedMouseHandler::calibrate(), clearCalibration()
*/

/*!
    \class QWSCalibratedMouseHandler
    \ingroup qws

    \brief The QWSCalibratedMouseHandler class implements a mouse
    driver providing calibration and noise reduction.

    A mouse driver handles events from system devices and generates
    mouse events. Custom mouse drivers can be added by subclassing the
    QMouseDriverPlugin class, using the QMouseDriverFactory class to
    dynamically load the driver into the application.

    Derive from the QWSCalibratedMouseHandler class when the system
    device does not have a fixed mapping between device and screen
    coordinates and/or produces noisy events, e.g. a touchscreen.

    QWSCalibratedMouseHandler provides an implementation of the
    calibrate() function to update the calibration parameters based on
    coordinate mapping of the given calibration data. The calibration
    data is represented by an QWSCalibrationData object. The linear
    transformation between device coordinates and screen coordinates
    is performed by calling the transform() function explicitly on the
    points passed to the QWSMouseHandler::mouseChanged() function.

    The calibration parameters are recalculated whenever calibrate()
    is called. They can be saved using the writeCalibration()
    function, and are stored in \c /etc/pointercal (separated by
    whitespace and in alphabetical order). The calibration parameters
    are read when the class is instantiated, but previously written
    parameters can be retrieved at any time using the
    readCalibration() function. Use the clearCalibration() function to
    make the mouse handler return mouse events in raw device
    coordinates and not in screen coordinates.

    To achieve noise reduction, QWSCalibratedMouseHandler provides the
    sendFiltered() function. Use this function instead of
    mouseChanged() whenever a mouse event occurs. The filter's size
    can be manipulated using the setFilterSize() function.

    \sa QWSMouseHandler, QWSPointerCalibrationData,
    {examples/qtopiacore/mousecalibration}{Mouse Calibration Example}
*/


/*!
    \internal
 */

QWSCalibratedMouseHandler::QWSCalibratedMouseHandler(const QString &, const QString &)
    : samples(5), currSample(0), numSamples(0)
{
    clearCalibration();
    readCalibration();
}

/*!
    \internal
*/
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

/*!
    Clears the current calibration, i.e. makes the mouse
    handler return mouse events in raw device coordinates instead of
    screen coordinates.

    \sa calibrate()
*/
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


/*!
    Saves the current calibration parameters in \c /etc/pointercal
    (separated by whitespace and in alphabetical order).

    \sa readCalibration()
*/
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
        qCritical("QWSCalibratedMouseHandler::writeCalibration: "
                  "Could not save calibration into %s", qPrintable(calFile));
    }
}

/*!
    Reads previously written calibration parameters which are stored
    in \c /etc/pointercal (separated by whitespace and in alphabetical
    order).

    \sa writeCalibration()
*/
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

/*!
    Updates the calibration parameters based on coordinate mapping of
    the given \a data.

    Create an instance of the QWSPointerCalibrationData class, fill in
    the device and screen coordinates and pass the object to the mouse
    handler using this function.

    \sa clearCalibration(), transform()
*/
void QWSCalibratedMouseHandler::calibrate(const QWSPointerCalibrationData *data)
{
    QPoint dev_tl = data->devPoints[QWSPointerCalibrationData::TopLeft];
    QPoint dev_br = data->devPoints[QWSPointerCalibrationData::BottomRight];
    QPoint screen_tl = data->screenPoints[QWSPointerCalibrationData::TopLeft];
    QPoint screen_br = data->screenPoints[QWSPointerCalibrationData::BottomRight];

    s = 1 << 16;

    a = s * (screen_tl.x() - screen_br.x()) / (dev_tl.x() - dev_br.x());
    b = 0;
    c = s * screen_tl.x() - a * dev_tl.x();

    d = 0;
    e = s * (screen_tl.y() - screen_br.y()) / (dev_tl.y() - dev_br.y());
    f = s * screen_tl.y() - e * dev_tl.y();

    writeCalibration();
}

/*!
    Transforms the given \a position from device coordinates to screen
    coordinates, and returns the transformed position.

    Typically, the transform() function is called explicitly on the
    points passed to the QWSMouseHandler::mouseChanged() function.

    This implementation is a linear transformation using 7 parameters
    (\c a, \c b, \c c, \c d, \c e, \c f and \c s) to transform the
    device coordinates (\c Xd, \c Yd) into screen coordinates (\c Xs,
    \c Ys) using the following equations:

    \code
        s*Xs = a*Xd + b*Yd + c
        s*Ys = d*Xd + e*Yd + f
    \endcode

    \sa mouseChanged()
*/
QPoint QWSCalibratedMouseHandler::transform(const QPoint &position)
{
    QPoint tp;

    tp.setX((a * position.x() + b * position.y() + c) / s);
    tp.setY((d * position.x() + e * position.y() + f) / s);

    return tp;
}

/*!
    Sets the size of the filter used in noise reduction to the given
    \a size.

    The sendFiltered() function reduces noice by calculating an
    average position from a collection of mouse event positions. The
    filter size determines the number of positions that forms the
    basis for the calculations.

    \sa sendFiltered()
*/
void QWSCalibratedMouseHandler::setFilterSize(int size)
{
    samples.resize(size);
    numSamples = 0;
    currSample = 0;
}

/*!
    \fn bool QWSCalibratedMouseHandler::sendFiltered(const QPoint &position, int state)

    Notifies the system of a new mouse event \e after applying a noise
    reduction filter. Returns true if the filtering process is
    successful; otherwise returns false. Note that if the filtering
    process failes, the system is not notified about the event.

    The given \a position is the global position of the mouse. The \a
    state parameter is a bitmask of the Qt::MouseButton enum's values
    indicating which mouse buttons are pressed.

    The sendFiltered() function reduces noice by calculating an
    average position from a collection of mouse event positions, and
    then call the mouseChanged() function with the new position. The
    number of positions that is used is determined by the filter size.

    \sa mouseChanged(), setFilterSize()
*/
bool QWSCalibratedMouseHandler::sendFiltered(const QPoint &position, int button)
{
    if (!button) {
        if (numSamples >= samples.count())
            mouseChanged(mousePos, 0);
        currSample = 0;
        numSamples = 0;
        return true;
    }

    bool sent = false;
    samples[currSample] = position;
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


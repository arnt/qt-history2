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

#include "qmousetslib_qws.h"

#ifndef QT_NO_QWS_MOUSE_TSLIB

#include "qsocketnotifier.h"
#include "qscreen_qws.h"

#include <tslib.h>
#include <errno.h>

#ifndef QT_QWS_TP_PRESSURE_THRESHOLD
#define QT_QWS_TP_PRESSURE_THRESHOLD 1
#endif

#ifndef QT_QWS_TP_JITTER_LIMIT
#define QT_QWS_TP_JITTER_LIMIT 3
#endif

/*!
    \class QWSTslibMouseHandler

    \brief The QWSTslibMouseHandler provides mouse events from the Universal
    Touch screen Library, tslib.

    To be able to compile this mouse handler, Qtopia Core must be configured
    with -qt-mouse-tslib. In addition, the tslib headers and libraries must
    be present in the build environment. Use -I and -L with configure if
    neccessary. The tslib sources can be downloaded from
    \l {http://cvs.arm.linux.org.uk}.

    In order to use this mouse handler, tslib must also be correctly installed
    on the target machine. This includes providing a ts.conf configuration
    file and setting the neccessary environment variables. See the README
    file provided with tslib for details.

    To make Qtopia Core explicitly choose the tslib mouse handler, set the
    QWS_MOUSE_PROTO environment variable to "tslib".
*/

class QWSTslibMouseHandlerPrivate : public QObject
{
    Q_OBJECT
public:
    QWSTslibMouseHandlerPrivate(QWSTslibMouseHandler *h,
                                const QString &device);
    ~QWSTslibMouseHandlerPrivate();

    void suspend();
    void resume();

    void calibrate(const QWSPointerCalibrationData *data);
    void clearCalibration();

private:
    QWSTslibMouseHandler *handler;
    struct tsdev *dev;
    QSocketNotifier *mouseNotifier;

    struct ts_sample lastSample;
    bool wasPressed;
    int lastdx;
    int lastdy;

    bool calibrated;
    QString devName;

    void open();
    void close();
    inline bool get_sample(struct ts_sample *sample);

private slots:
    void readMouseData();
};

QWSTslibMouseHandlerPrivate::QWSTslibMouseHandlerPrivate(QWSTslibMouseHandler *h,
                                                         const QString &device)
    : handler(h)
{
    devName = device;

    if (devName.isNull()) {
        const char *str = getenv("TSLIB_TSDEVICE");
        if (str)
            devName = QString(str);
    }
    if (devName.isNull())
        devName = QString("/dev/ts");

    open();
    calibrated = true;

    int fd = ts_fd(dev);
    mouseNotifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(mouseNotifier, SIGNAL(activated(int)),this, SLOT(readMouseData()));
    resume();
}

QWSTslibMouseHandlerPrivate::~QWSTslibMouseHandlerPrivate()
{
    close();
}

void QWSTslibMouseHandlerPrivate::open()
{
    dev = ts_open(devName.toLocal8Bit().constData(), 1);
    if (!dev) {
        qWarning("Unable to open touchscreen device '%s': %s",
                 qPrintable(devName), strerror(errno));
        return;
    }

    if (ts_config(dev)) {
        qWarning("Unable to configure touchscreen device '%s': %s",
                 qPrintable(devName), strerror(errno));
        close();
        return;
    }

    // always use the linear module which translates from touch screen
    // coordinates to screen coordinates
    if (ts_load_module(dev, "linear", 0))
        qWarning("Unable to load the linear module: '%s'", strerror(errno));
}

void QWSTslibMouseHandlerPrivate::close()
{
    if (dev)
        ts_close(dev);
}

void QWSTslibMouseHandlerPrivate::suspend()
{
    mouseNotifier->setEnabled(false);
}

void QWSTslibMouseHandlerPrivate::resume()
{
    memset(&lastSample, 0, sizeof(lastSample));
    wasPressed = false;
    lastdx = 0;
    lastdy = 0;
    mouseNotifier->setEnabled(true);
}

bool QWSTslibMouseHandlerPrivate::get_sample(struct ts_sample *sample)
{
    if (!calibrated)
        return (ts_read_raw(dev, sample, 1) == 1);

    return (ts_read(dev, sample, 1) == 1);
}

void QWSTslibMouseHandlerPrivate::readMouseData()
{
    if(!qt_screen)
        return;

    for(;;) {
        struct ts_sample sample = lastSample;
        bool pressed = wasPressed;

        // Fast return if there's no events.
        if (!get_sample(&sample))
            return;
        pressed = (sample.pressure >= QT_QWS_TP_PRESSURE_THRESHOLD);

        // Only return last sample unless there's a press/release event.
        while (pressed == wasPressed) {
            if (!get_sample(&sample))
                break;
            pressed = (sample.pressure >= QT_QWS_TP_PRESSURE_THRESHOLD);
        }

        int dx = sample.x - lastSample.x;
        int dy = sample.y - lastSample.y;

        // Remove small movements in oppsite direction
        if (dx * lastdx < 0 && qAbs(dx) < QT_QWS_TP_JITTER_LIMIT) {
            sample.x = lastSample.x;
            dx = 0;
        }
        if (dy * lastdy < 0 && qAbs(dy) < QT_QWS_TP_JITTER_LIMIT) {
            sample.y = lastSample.y;
            dy = 0;
        }

        if (wasPressed == pressed && dx == 0 && dy == 0)
            return;

#ifdef TSLIBMOUSEHANDLER_DEBUG
        qDebug() << "last" << QPoint(lastSample.x, lastSample.y)
                 << "curr" << QPoint(sample.x, sample.y)
                 << "dx,dy" << QPoint(dx, dy)
                 << "ddx,ddy" << QPoint(dx*lastdx, dy*lastdy)
                 << "pressed" << wasPressed << pressed;
#endif

        lastSample = sample;
        wasPressed = pressed;
        if (dx != 0)
            lastdx = dx;
        if (dy != 0)
            lastdy = dy;

        handler->mouseChanged(QPoint(sample.x, sample.y), pressed);
    }
}

void QWSTslibMouseHandlerPrivate::clearCalibration()
{
    suspend();
    close();
    handler->QWSCalibratedMouseHandler::clearCalibration();
    calibrated = false;
    open();
    resume();
}

void QWSTslibMouseHandlerPrivate::calibrate(const QWSPointerCalibrationData *data)
{
    suspend();
    close();
    // default implementation writes to /etc/pointercal, which is read by tslib
    handler->QWSCalibratedMouseHandler::calibrate(data);
    calibrated = true;
    open();
    resume();
}

/*!
  \reimp
*/
QWSTslibMouseHandler::QWSTslibMouseHandler(const QString &,
                                           const QString &device)
{
    d = new QWSTslibMouseHandlerPrivate(this, device);
}

/*!
    \reimp
*/
QWSTslibMouseHandler::~QWSTslibMouseHandler()
{
    delete d;
}

/*!
    \reimp
*/
void QWSTslibMouseHandler::suspend()
{
    d->suspend();
}

/*!
    \reimp
*/
void QWSTslibMouseHandler::resume()
{
    d->resume();
}

/*!
    \reimp
*/
void QWSTslibMouseHandler::clearCalibration()
{
    d->clearCalibration();
}

/*!
    \reimp
*/
void QWSTslibMouseHandler::calibrate(const QWSPointerCalibrationData *data)
{
    d->calibrate(data);
}

#include "qmousetslib_qws.moc"
#endif //QT_NO_QWS_MOUSE_TSLIB

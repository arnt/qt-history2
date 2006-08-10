/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qmousevr41xx_qws.h"

#ifndef QT_NO_QWS_MOUSE_VR41XX
#include "qwindowsystem_qws.h"
#include "qsocketnotifier.h"
#include "qtimer.h"
#include "qapplication.h"
#include "qscreen_qws.h"
#include <qstringlist.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

class QWSVr41xxMouseHandlerPrivate : public QObject
{
    Q_OBJECT
public:
    QWSVr41xxMouseHandlerPrivate(QWSVr41xxMouseHandler *, const QString &, const QString &);
    ~QWSVr41xxMouseHandlerPrivate();

    void resume();
    void suspend();

private slots:
    void sendRelease();
    void readMouseData();

private:
    enum { mouseBufSize = 128 };
    int mouseFD;
    int mouseIdx;
    QTimer *rtimer;
    uchar mouseBuf[mouseBufSize];
    QSocketNotifier *mouseNotifier;
    QWSVr41xxMouseHandler *handler;
    QPoint currPos;
    bool isPressed;
    int pressLimit;
};

QWSVr41xxMouseHandler::QWSVr41xxMouseHandler(const QString &drv, const QString &dev)
    : QWSCalibratedMouseHandler(drv, dev)
{
    d = new QWSVr41xxMouseHandlerPrivate(this, drv, dev);
}

QWSVr41xxMouseHandler::~QWSVr41xxMouseHandler()
{
    delete d;
}

void QWSVr41xxMouseHandler::resume()
{
    d->resume();
}

void QWSVr41xxMouseHandler::suspend()
{
    d->suspend();
}

QWSVr41xxMouseHandlerPrivate::QWSVr41xxMouseHandlerPrivate(QWSVr41xxMouseHandler *h, const QString &, const QString &device)
    : handler(h), isPressed(false)
{
    QStringList options = device.split(":");
    int index = -1;

    int filterSize = 3;
    QRegExp filterRegExp("filter=(\\d+)");
    index = options.indexOf(filterRegExp);
    if (index != -1) {
        filterSize = filterRegExp.cap(1).toInt();
        options.removeAt(index);
    }
    handler->setFilterSize(filterSize);

    pressLimit = 750;
    QRegExp pressRegExp("press=(\\d+)");
    index = options.indexOf(pressRegExp);
    if (index != -1) {
        pressLimit = filterRegExp.cap(1).toInt();
        options.removeAt(index);
    }

    QString dev;
    if (options.isEmpty())
        dev = QLatin1String("/dev/vrtpanel");
    else
        dev = options.first();

    if ((mouseFD = open(dev.toLocal8Bit().constData(), O_RDONLY)) < 0) {
        qWarning("Cannot open %s (%s)", qPrintable(dev), strerror(errno));
        return;
    }
    sleep(1);

    if (fcntl(mouseFD, F_SETFL, O_NONBLOCK) < 0) {
        qWarning("Error initializing touch panel.");
        return;
    }

    mouseNotifier = new QSocketNotifier(mouseFD, QSocketNotifier::Read, this);
    connect(mouseNotifier, SIGNAL(activated(int)),this, SLOT(readMouseData()));

    rtimer = new QTimer(this);
    rtimer->setSingleShot(true);
    connect(rtimer, SIGNAL(timeout()), this, SLOT(sendRelease()));
    mouseIdx = 0;

    printf("\033[?25l"); fflush(stdout); // VT100 cursor off
}

QWSVr41xxMouseHandlerPrivate::~QWSVr41xxMouseHandlerPrivate()
{
    if (mouseFD >= 0)
        close(mouseFD);
}

void QWSVr41xxMouseHandlerPrivate::suspend()
{
    mouseNotifier->setEnabled(false);
}


void QWSVr41xxMouseHandlerPrivate::resume()
{
    mouseIdx = 0;
    mouseNotifier->setEnabled(true);
}

void QWSVr41xxMouseHandlerPrivate::sendRelease()
{
    handler->sendFiltered(currPos, Qt::NoButton);
    isPressed = false;
}

void QWSVr41xxMouseHandlerPrivate::readMouseData()
{
    int n;
    do {
        n = read(mouseFD, mouseBuf+mouseIdx, mouseBufSize-mouseIdx);
        if (n > 0)
            mouseIdx += n;
    } while (n > 0 && mouseIdx < mouseBufSize);

    int idx = 0;
    while (mouseIdx-idx >= (int)sizeof(short) * 6) {
        uchar *mb = mouseBuf+idx;
        ushort *data = (ushort *) mb;
        if (data[0] & 0x8000) {
            if (data[5] >= pressLimit) {
                currPos = QPoint(data[3] - data[4], data[2] - data[1]);
                if (handler->sendFiltered(currPos, Qt::LeftButton)) {
                    isPressed = true;
                    rtimer->start(200); // release unreliable
                }
            }
        } else if (isPressed) {
            rtimer->start(50);
        }
        idx += sizeof(ushort) * 6;
    }

    int surplus = mouseIdx - idx;
    for (int i = 0; i < surplus; i++)
        mouseBuf[i] = mouseBuf[idx+i];
    mouseIdx = surplus;
}

#include "qmousevr41xx_qws.moc"
#endif //QT_NO_QWS_MOUSE_VR41

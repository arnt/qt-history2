/****************************************************************************
**
** Implementation of Qt/Embedded mouse drivers.
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

#include "qmousevr41xx_qws.h"

#ifndef QT_NO_QWS_MOUSE_VR41
#include "qwindowsystem_qws.h"
#include "qsocketnotifier.h"
#include "qtimer.h"
#include "qapplication.h"
#include "qgfx_qws.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#include <linux/tpanel.h>

class QWSVr41xxMouseHandlerPrivate : public QObject
{
    Q_OBJECT
public:
    QWSVr41xxMouseHandlerPrivate(QWSVr41xxMouseHandler *, const QString &, const QString &);
    ~QWSVr41xxMouseHandlerPrivate();

private slots:
    void sendRelease();
    void readMouseData();

private:
    enum { mouseBufSize = 128 };
    int mouseFD;
    int mouseIdx;
    int obstate;
    QTimer *rtimer;
    uchar mouseBuf[mouseBufSize];
    QWSVr41xxMouseHandler *handler;
};

QWSVr41xxMouseHandler::QWSVr41xxMouseHandler(const QString &drv, const QString &dev)
{
    d = new QWSVr41xxMouseHandlerPrivate(this, drv, dev);
    setFilterSize(3);
}

QWSVr41xxMouseHandler::~QWSVr41xxMouseHandler()
{
    delete d;
}

QWSVr41xxMouseHandlerPrivate::QWSVr41xxMouseHandlerPrivate(QWSVr41xxMouseHandler *h, const QString &, const QString &device)
    : handler(h)
{
    QString dev = device;
    if (dev.isEmpty())
        dev = "/dev/tpanel";

    if ((mouseFD = open(dev, O_RDONLY)) < 0) {
        qFatal("Cannot open %s (%s)", dev.latin1(), strerror(errno));
    } else {
        sleep(1);
    }

    struct scanparam s;
    s.interval = 20000;
    s.settletime = 480;
    if (ioctl(mouseFD, TPSETSCANPARM, &s) < 0
      || fcntl(mouseFD, F_SETFL, O_NONBLOCK) < 0)
        qWarning("Error initializing touch panel.");

    QSocketNotifier *mouseNotifier;
    mouseNotifier = new QSocketNotifier(mouseFD, QSocketNotifier::Read,
                                         this);
    connect(mouseNotifier, SIGNAL(activated(int)),this, SLOT(readMouseData()));

    rtimer = new QTimer(this);
    connect(rtimer, SIGNAL(timeout()), this, SLOT(sendRelease()));
    mouseIdx = 0;

    printf("\033[?25l"); fflush(stdout); // VT100 cursor off
}

QWSVr41xxMouseHandlerPrivate::~QWSVr41xxMouseHandlerPrivate()
{
    if (mouseFD >= 0)
        close(mouseFD);
}

void QWSVr41xxMouseHandlerPrivate::sendRelease()
{
    handler->mouseChanged(handler->pos(), 0);
}

void QWSVr41xxMouseHandlerPrivate::readMouseData()
{
    if(!qt_screen)
        return;
    static bool pressed = false;

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
            if (data[5] > 750) {
                QPoint t(data[3]-data[4],data[2]-data[1]);
                if (handler->sendFiltered(t, Qt::LeftButton))
                    pressed = true;
                if (pressed)
                    rtimer->start(200, true); // release unreliable
            }
        } else if (pressed) {
            rtimer->start(50, true);
            pressed = false;
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

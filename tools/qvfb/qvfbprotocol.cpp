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

#include "qvfbprotocol.h"
#include "qvfbhdr.h"

#include <QDebug>
#include <QTimer>

#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif


QVFbViewProtocol::QVFbViewProtocol(int display_id, QObject *parent) :
    QObject(parent), mDisplayId(display_id) { }

QVFbViewProtocol::~QVFbViewProtocol() {}

void QVFbViewProtocol::flushChanges() {}

void QVFbViewProtocol::sendKeyboardData(QString unicode, int keycode,
        int modifiers, bool press, bool repeat)
{
    if (keyHandler())
        keyHandler()->sendKeyboardData(unicode, keycode, modifiers, press, repeat);
}

void QVFbViewProtocol::sendMouseData(const QPoint &pos, int buttons, int wheel)
{
    if (mouseHandler())
        mouseHandler()->sendMouseData(pos, buttons, wheel);
}

static int openPipe(const char *fileName)
{
    unlink(fileName);

    mkfifo(fileName, 0666);
    int fd = ::open(fileName, O_RDWR | O_NDELAY);
    return fd;
}

QVFbKeyPipeProtocol::QVFbKeyPipeProtocol(int display_id)
    : QVFbKeyProtocol(display_id)
{
    fileName = QString(QT_VFB_KEYBOARD_PIPE).arg(display_id);
    fd = openPipe(fileName.toLocal8Bit().constData());

    if (fd == -1)
	qFatal("Cannot open keyboard pipe %s", fileName.toLocal8Bit().data());
}

QVFbKeyPipeProtocol::~QVFbKeyPipeProtocol()
{
    sendKeyboardData(0, 0, 0, true, false); // magic die key
    ::close(fd);
    unlink(fileName.toLocal8Bit().constData());
}

void QVFbKeyPipeProtocol::sendKeyboardData(QString unicode, int keycode,
        int modifiers, bool press, bool repeat)
{
    QVFbKeyData kd;
    kd.unicode = unicode[0].unicode();
    kd.keycode = keycode;
    kd.modifiers = static_cast<Qt::KeyboardModifier>(modifiers);
    kd.press = press;
    kd.repeat = repeat;
    write(fd, &kd, sizeof(QVFbKeyData));
}

QVFbMousePipe::QVFbMousePipe(int display_id)
    : QVFbMouseProtocol(display_id)
{
    fileName = QString(QT_VFB_MOUSE_PIPE).arg(display_id);
    fd = openPipe(fileName.toLocal8Bit().constData());

    if (fd == -1)
	qFatal("Cannot open mouse pipe %s", fileName.toLocal8Bit().data());
}

QVFbMousePipe::~QVFbMousePipe()
{
    ::close(fd);
    unlink(fileName.toLocal8Bit().constData());
}


void QVFbMousePipe::sendMouseData(const QPoint &pos, int buttons, int wheel)
{
    write(fd, &pos, sizeof(QPoint));
    write(fd, &buttons, sizeof(int));
    write(fd, &wheel, sizeof(int));
}

QVFbMouseLinuxTP::QVFbMouseLinuxTP(int display_id)
    : QObject(), QVFbMousePipe(display_id), lastPos(-1,-1)
{
    /* the timer is needed because a real touch screen send data as long as
       there is pressure.  And the linux tp driver will filter, requiring
       a minimum of 5 samples before it even registers a press.
       */
    repeater = new QTimer(this);
    connect(repeater, SIGNAL(timeout()), this, SLOT(repeatLastPress()));
}

QVFbMouseLinuxTP::~QVFbMouseLinuxTP()
{
}


void QVFbMouseLinuxTP::sendMouseData(const QPoint &pos, int buttons, int)
{
    if (buttons & Qt::LeftButton) {
        // press
        repeater->start(5);
        writeToPipe(pos, 1);
        lastPos = pos;
    } else {
        // release
        if (lastPos == QPoint(-1,-1))
            return; /* only send one release */
        repeater->stop();
        writeToPipe(pos, 0);
        lastPos = QPoint(-1,-1);
    }
}

void QVFbMouseLinuxTP::writeToPipe(const QPoint &pos, ushort pressure)
{
    ushort v;
    write(fd, &pressure, sizeof(ushort));
    v = pos.x();
    write(fd, &v, sizeof(ushort));
    v = pos.y();
    write(fd, &v, sizeof(ushort));
    v = 1; // pad
    write(fd, &v, sizeof(ushort));
}

void QVFbMouseLinuxTP::repeatLastPress()
{
    writeToPipe(lastPos, 1);
}

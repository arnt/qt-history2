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

#include "qvfbprotocol.h"
#include "qvfbhdr.h"

#include <QDebug>

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

void QVFbViewProtocol::sendKeyboardData(int unicode, int keycode,
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
    qDebug("create keyboard pipe");
    fileName = QString(QT_VFB_KEYBOARD_PIPE).arg(display_id);
    fd = openPipe(fileName.local8Bit().constData());

    if (fd == -1)
	qFatal("Cannot open keyboard pipe %s", fileName.toLocal8Bit().data());
}

QVFbKeyPipeProtocol::~QVFbKeyPipeProtocol()
{
    sendKeyboardData(0, 0, 0, true, false); // magic die key
    ::close(fd);
    unlink(fileName.local8Bit().constData());
}

void QVFbKeyPipeProtocol::sendKeyboardData(int unicode, int keycode,
        int modifiers, bool press, bool repeat)
{
    QVFbKeyData kd;
    kd.unicode = unicode;
    kd.keycode = keycode;
    kd.modifiers = static_cast<Qt::KeyboardModifier>(modifiers);
    kd.press = press;
    kd.repeat = repeat;
    write(fd, &kd, sizeof(QVFbKeyData));
}

QVFbMousePipeProtocol::QVFbMousePipeProtocol(int display_id, bool w)
    : QVFbMouseProtocol(display_id), mSupportWheelEvents(w)
{
    fileName = QString(QT_VFB_MOUSE_PIPE).arg(display_id);
    fd = openPipe(fileName.local8Bit().constData());

    if (fd == -1)
	qFatal("Cannot open mouse pipe %s", fileName.toLocal8Bit().data());
}

QVFbMousePipeProtocol::~QVFbMousePipeProtocol()
{
    ::close(fd);
    unlink(fileName.local8Bit().constData());
}


void QVFbMousePipeProtocol::sendMouseData(const QPoint &pos, int buttons, int wheel)
{
    write(fd, &pos, sizeof(QPoint));
    write(fd, &buttons, sizeof(int));
    if (mSupportWheelEvents)
        write(fd, &wheel, sizeof(int));
}

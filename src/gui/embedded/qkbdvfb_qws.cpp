/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <qvfbhdr.h>
#include <qkbdvfb_qws.h>

#ifndef QT_NO_QWS_KEYBOARD
#ifndef QT_NO_QWS_KBD_QVFB

#include <qwindowsystem_qws.h>
#include <qsocketnotifier.h>
#include <qapplication.h>

QVFbKeyboardHandler::QVFbKeyboardHandler(const QString &device)
    : QObject()
{
    terminalName = device;
    if (terminalName.isEmpty())
        terminalName = QLatin1String("/dev/vkdb");
    kbdFD = -1;
    kbdIdx = 0;
    kbdBufferLen = sizeof(QVFbKeyData) * 5;
    kbdBuffer = new unsigned char [kbdBufferLen];

    if ((kbdFD = open(terminalName.toLatin1().constData(), O_RDONLY | O_NDELAY)) < 0) {
        qWarning("Cannot open %s (%s)", terminalName.toLatin1().constData(),
        strerror(errno));
    } else {
        // Clear pending input
        char buf[2];
        while (read(kbdFD, buf, 1) > 0) { }

        notifier = new QSocketNotifier(kbdFD, QSocketNotifier::Read, this);
        connect(notifier, SIGNAL(activated(int)),this, SLOT(readKeyboardData()));
    }
}

QVFbKeyboardHandler::~QVFbKeyboardHandler()
{
    if (kbdFD >= 0)
        close(kbdFD);
    delete [] kbdBuffer;
}


void QVFbKeyboardHandler::readKeyboardData()
{
    int n;
    do {
        n  = read(kbdFD, kbdBuffer+kbdIdx, kbdBufferLen - kbdIdx);
        if (n > 0)
            kbdIdx += n;
    } while (n > 0);

    int idx = 0;
    while (kbdIdx - idx >= (int)sizeof(QVFbKeyData)) {
        QVFbKeyData *kd = (QVFbKeyData *)(kbdBuffer + idx);
        if (kd->unicode == 0 && kd->keycode == 0 && kd->modifiers == 0 && kd->press) {
            // magic exit key
            qWarning("Instructed to quit by Virtual Keyboard");
            qApp->quit();
        }
        QWSServer::processKeyEvent(kd->unicode, kd->keycode, kd->modifiers, kd->press, kd->repeat);
        idx += sizeof(QVFbKeyData);
    }

    int surplus = kbdIdx - idx;
    for (int i = 0; i < surplus; i++)
        kbdBuffer[i] = kbdBuffer[idx+i];
    kbdIdx = surplus;
}

#endif // QT_NO_QWS_KBD_QVFB
#endif // QT_NO_QWS_KEYBOARD


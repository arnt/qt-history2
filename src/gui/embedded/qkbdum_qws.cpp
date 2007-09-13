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

#include "qkbdum_qws.h"
#include "qvfbhdr.h"

#if !defined(QT_NO_QWS_KEYBOARD) && !defined(QT_NO_QWS_KBD_UM)

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <qstring.h>
#include <qwindowsystem_qws.h>
#include <qsocketnotifier.h>

QT_BEGIN_NAMESPACE

class QWSUmKeyboardHandlerPrivate : public QObject
{
    Q_OBJECT

public:
    QWSUmKeyboardHandlerPrivate(const QString&);
    ~QWSUmKeyboardHandlerPrivate();

private slots:
    void readKeyboardData();

private:
    int kbdFD;
    int kbdIdx;
    const int kbdBufferLen;
    unsigned char *kbdBuffer;
    QSocketNotifier *notifier;
};

QWSUmKeyboardHandlerPrivate::QWSUmKeyboardHandlerPrivate(const QString &device)
    : kbdFD(-1), kbdIdx(0), kbdBufferLen(sizeof(QVFbKeyData)*5)
{
    kbdBuffer = new unsigned char [kbdBufferLen];

    if ((kbdFD = open((const char *)device.toLocal8Bit(), O_RDONLY | O_NDELAY)) < 0) {
        qDebug("Cannot open %s (%s)", (const char *)device.toLocal8Bit(),
        strerror(errno));
    } else {
        // Clear pending input
        char buf[2];
        while (read(kbdFD, buf, 1) > 0) { }

        notifier = new QSocketNotifier(kbdFD, QSocketNotifier::Read, this);
        connect(notifier, SIGNAL(activated(int)),this, SLOT(readKeyboardData()));
    }
}

QWSUmKeyboardHandlerPrivate::~QWSUmKeyboardHandlerPrivate()
{
    if (kbdFD >= 0)
        close(kbdFD);
    delete [] kbdBuffer;
}


void QWSUmKeyboardHandlerPrivate::readKeyboardData()
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
        // Qtopia Key filters must still work.
        QWSServer::processKeyEvent(kd->unicode, kd->keycode, kd->modifiers, kd->press, kd->repeat);
        idx += sizeof(QVFbKeyData);
    }

    int surplus = kbdIdx - idx;
    for (int i = 0; i < surplus; i++)
        kbdBuffer[i] = kbdBuffer[idx+i];
    kbdIdx = surplus;
}

QWSUmKeyboardHandler::QWSUmKeyboardHandler(const QString &device)
    : QWSKeyboardHandler()
{
    d = new QWSUmKeyboardHandlerPrivate(device);
}

QWSUmKeyboardHandler::~QWSUmKeyboardHandler()
{
    delete d;
}

QT_END_NAMESPACE

#include "qkbdum_qws.moc"

#endif // QT_NO_QWS_KEYBOARD && QT_NO_QWS_KBD_UM

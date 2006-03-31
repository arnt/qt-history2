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

#ifndef QKBDVFB_QWS_H
#define QKBDVFB_QWS_H

#include <QtGui/qkbd_qws.h>

QT_MODULE(Gui)

#ifndef QT_NO_QWS_KEYBOARD

#ifndef QT_NO_QWS_QVFB

class QSocketNotifier;

class QVFbKeyboardHandler : public QObject, public QWSKeyboardHandler
{
    Q_OBJECT
public:
    QVFbKeyboardHandler(const QString &device);
    virtual ~QVFbKeyboardHandler();

private slots:
    void readKeyboardData();

private:
    QString terminalName;
    int kbdFD;
    int kbdIdx;
    int kbdBufferLen;
    unsigned char *kbdBuffer;
    QSocketNotifier *notifier;
};

#endif // QT_NO_QWS_QVFB

#endif // QT_NO_QWS_KEYBOARD

#endif // QKBDVFB_QWS_H

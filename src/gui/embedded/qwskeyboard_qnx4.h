/****************************************************************************
**
** Implementation of Qt/Embedded Qnx keyboard drivers.
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

#ifndef QWSKEYBOARD_QNX4_H
#define QWSKEYBOARD_QNX4_H

#ifndef QT_H
#include "qwindowsystem_qws.h"
#include "qwsutils_qws.h"
#include "qscreen_qws.h"
#endif // QT_H

#include <qapplication.h>
#include <qsocketnotifier.h>
#include <qnamespace.h>
#include <qtimer.h>

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#if defined(Q_OS_QNX4)
#ifndef QWSQNX4KB_H
#define QWSQNX4KB_H

#include <qkeyboard_qws.h>


class QWSQnx4KeyboardHandler : public QWSKeyboardHandler
{
    Q_OBJECT
public:
    QWSQnx4KeyboardHandler();
    ~QWSQnx4KeyboardHandler();

    void doKey(uchar);

    public slots:
        void readKbdData(int);

private:
    enum GuidantState {
        GuidantNone, GuidantPressed, GuidantReleased, GuidantDropped
    } gState;
    int shift;
    int alt;
    int ctrl;
    bool extended;
    bool caps;
    int modifiers;
    int prevuni;
    int prevkey;

    int kbdFD;
    QList<QSocketNotifier> notifiers;
};

#endif
#endif

#endif // QWSKEYBOARD_QNX4_H

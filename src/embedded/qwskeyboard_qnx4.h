
/****************************************************************************
** $Id$
**
** Implementation of Qt/Embedded Qnx keyboard drivers
**
** Copyright (C) 1999-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QWSKEYBOARD_QNX4_H
#define QWSKEYBOARD_QNX4_H

#ifndef QT_H
#include "qwindowsystem_qws.h"
#include "qwsutils_qws.h"
#include "qgfx_qws.h"
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

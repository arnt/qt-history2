/****************************************************************************
**
** Definition of QGuiEventLoop class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QGUIEVENTLOOP_P_H
#define QGUIEVENTLOOP_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  This header file may
// change from version to version without notice, or even be
// removed.
//
// We mean it.
//
//

#include "qguieventloop.h"
#include <private/qeventloop_p.h>

#ifdef Q_WS_X11
#include "private/qt_x11_p.h"
#elif defined(Q_WS_MAC)

#include "qt_mac.h"
#include "qlist.h"

struct timeval; //stdc struct
struct MacTimerInfo {
    int	     id;
    int  interval;
    QObject *obj;
    bool pending;
    EventLoopTimerRef mac_timer;
};
typedef QList<MacTimerInfo> MacTimerList;	// list of TimerInfo structs

struct MacSocketInfo {
    union {
	CFReadStreamRef read_not;
	CFWriteStreamRef write_not;
    };
};

#endif

class Q_GUI_EXPORT QGuiEventLoopPrivate : public QEventLoopPrivate
{
    Q_DECLARE_PUBLIC(QGuiEventLoop);
public:
#ifdef Q_WS_MAC
    int zero_timer_count;
    EventLoopTimerRef select_timer;
    MacTimerList *macTimerList;
    QHash<QSocketNotifier *, MacSocketInfo *> *macSockets;
#endif
};

#endif // QEVENTLOOP_P_H

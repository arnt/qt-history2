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

#ifndef QEVENTDISPATCHER_QWS_P_H
#define QEVENTDISPATCHER_QWS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qeventdispatcher_unix.h"

class QEventDispatcherQWSPrivate;

class QEventDispatcherQWS : public QEventDispatcherUNIX
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QEventDispatcherQWS)

public:
    QEventDispatcherQWS(QObject *parent = 0);
    ~QEventDispatcherQWS();

    bool processEvents(QEventLoop::ProcessEventsFlags flags);
    bool hasPendingEvents();

    void flush();

    void startingUp();
    void closingDown();

protected:
    int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
               timeval *timeout);
};

#endif // QEVENTDISPATCHER_QWS_P_H

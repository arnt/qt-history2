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

#include "qwineventnotifier_p.h"

#include "qeventdispatcher_win.h"
#include "qcoreapplication.h"


/*
This class is menat for use with Windows Wait functions.
witht MsgWaitForMultipleObjectsEx now used in the windows event loop it should be
possible to use this class in a similar way to QSocketNotifier. It will enable the use
 of overlapped io true async qProcess on windows

  for the now the api models QSocketNotifier and is just in a trial period.
*/

QWinEventNotifier::QWinEventNotifier(QObject *parent)
  : QObject(parent), enabled(false)
{}

QWinEventNotifier::QWinEventNotifier(HANDLE hEvent, QObject *parent)
 : QObject(parent),  handleToEvent(hEvent), enabled(false)
{
    QEventDispatcherWin32 *eventDispatcher = qt_cast<QEventDispatcherWin32 *>(QAbstractEventDispatcher::instance(thread()));
    Q_ASSERT_X(eventDispatcher, "QWinEventNotifier::QWinEventNotifier()",
               "Cannot create a win event notifier without a QEventDispatcherWin32");
    eventDispatcher->registerEventNotifier(this);
    enabled = true;
}

QWinEventNotifier::~QWinEventNotifier()
{
    setEnabled(false);
}

void QWinEventNotifier::setHandle(HANDLE hEvent)
{
    setEnabled(false);
    handleToEvent = hEvent;
}

HANDLE  QWinEventNotifier::handle() const
{
    return handleToEvent;
}

bool QWinEventNotifier::isEnabled() const
{
    return enabled;
}

void QWinEventNotifier::setEnabled(bool enable)
{
    if (enabled == enable)                        // no change
        return;
    enabled = enable;

    QEventDispatcherWin32 *eventDispatcher = qt_cast<QEventDispatcherWin32 *>(QAbstractEventDispatcher::instance(thread()));
    if (!eventDispatcher) // perhaps application is shutting down
        return;

    if (enabled)
        eventDispatcher->registerEventNotifier(this);
    else
        eventDispatcher->unregisterEventNotifier(this);
}

bool QWinEventNotifier::event(QEvent * e)
{
    // hijack the socket notifier code for now
    QObject::event(e);                        // will activate filters
    if (e->type() == QEvent::WinEventAct) {
        emit activated(handleToEvent);
        return true;
    }
    return false;
}

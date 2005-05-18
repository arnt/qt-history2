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

#ifndef QMOTIF_H
#define QMOTIF_H

#include <QtCore/qabstracteventdispatcher.h>
#include <X11/Intrinsic.h>

class QMotifPrivate;

class QMotif : public QAbstractEventDispatcher
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QMotif)

public:
    QMotif(const char *applicationClass, XtAppContext context = NULL,
           XrmOptionDescRec *options = 0, int numOptions = 0);
    ~QMotif();

    XtAppContext applicationContext() const;

    static Display *display();
    static XEvent *lastEvent();

    static void registerWidget(QWidget *);
    static void unregisterWidget(QWidget *);
    static bool redeliverEvent(XEvent *event);

    // QAbstractEventDispatcher interface
    bool processEvents(QEventLoop::ProcessEventsFlags flags);
    bool hasPendingEvents();

    void registerSocketNotifier(QSocketNotifier *);
    void unregisterSocketNotifier(QSocketNotifier *);

    void registerTimer(int timerId, int interval, QObject *object);
    bool unregisterTimer(int timerId);
    bool unregisterTimers(QObject *object);
    QList<TimerInfo> registeredTimers(QObject *object) const;

    void wakeUp();
    void interrupt();
    void flush();

    void startingUp();
    void closingDown();
};

#endif // QMOTIF_H

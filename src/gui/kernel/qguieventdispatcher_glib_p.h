#ifndef QGUIEVENTDISPATCHER_GLIB_P_H
#define QGUIEVENTDISPATCHER_GLIB_P_H

#include "../../corelib/kernel/qeventdispatcher_glib_p.h"

class QGuiEventDispatcherGlibPrivate;

class QGuiEventDispatcherGlib : public QEventDispatcherGlib
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGuiEventDispatcherGlib)

public:
    explicit QGuiEventDispatcherGlib(QObject *parent = 0);
    ~QGuiEventDispatcherGlib();

    bool processEvents(QEventLoop::ProcessEventsFlags flags);

    void startingUp();
};

#endif // QGUIEVENTDISPATCHER_GLIB_P_H

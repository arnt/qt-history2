#ifndef QABSTRACTEVENTDISPATCHER_P_H
#define QABSTRACTEVENTDISPATCHER_P_H

#include "qabstracteventdispatcher.h"
#include "qobject_p.h"

class QAbstractEventDispatcherPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QAbstractEventDispatcher)
public:
    inline QAbstractEventDispatcherPrivate()
        : event_filter(0)
    { }
    QAbstractEventDispatcher::EventFilter event_filter;
};

#endif // QABSTRACTEVENTDISPATCHER_H

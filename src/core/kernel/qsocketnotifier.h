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

#ifndef QSOCKETNOTIFIER_H
#define QSOCKETNOTIFIER_H

#include "qobject.h"


class Q_CORE_EXPORT QSocketNotifier : public QObject
{
    Q_OBJECT
public:
    enum Type { Read, Write, Exception };

    QSocketNotifier(int socket, Type, QObject *parent = 0);
#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QSocketNotifier(int socket, Type, QObject *parent, const char *name);
#endif
    ~QSocketNotifier();

    inline int socket() const { return sockfd; }
    inline Type type() const { return sntype; }

    inline bool isEnabled() const { return snenabled; }
    void setEnabled(bool);

signals:
    void activated(int socket);

protected:
    bool event(QEvent *);

private:
    Q_DISABLE_COPY(QSocketNotifier)

    int sockfd;
    Type sntype;
    bool snenabled;
};

//#define Q_WIN_EVENT_NOTIFIER
#ifdef Q_WIN_EVENT_NOTIFIER
//### for want of a better place

//### for now

class Q_CORE_EXPORT QWinEventNotifier : public QObject
{
    Q_OBJECT
public:

    QWinEventNotifier(QObject *parent = 0);
    QWinEventNotifier(long hEvent, QObject *parent = 0);
    ~QWinEventNotifier();

    void setHandle(long hEvent);
    long handle() const;

    bool isEnabled() const;
    void setEnabled(bool enable);

signals:
    void activated(long hEvent);

protected:
    bool event(QEvent * e);

private:
    Q_DISABLE_COPY(QWinEventNotifier)

    long handleToEvent;
    bool enabled;
};

#endif

#endif // QSOCKETNOTIFIER_H

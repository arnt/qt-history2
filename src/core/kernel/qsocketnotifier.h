/****************************************************************************
**
** Definition of QSocketNotifier class.
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

#ifndef QSOCKETNOTIFIER_H
#define QSOCKETNOTIFIER_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H


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

    int                 socket()        const;
    Type         type()                const;

    bool         isEnabled()        const;
    virtual void setEnabled(bool);

signals:
    void         activated(int socket);

protected:
    bool         event(QEvent *);

private:
    int                 sockfd;
    Type         sntype;
    bool         snenabled;

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSocketNotifier(const QSocketNotifier &);
    QSocketNotifier &operator=(const QSocketNotifier &);
#endif
};


inline int QSocketNotifier::socket() const
{ return sockfd; }

inline QSocketNotifier::Type QSocketNotifier::type() const
{ return sntype; }

inline bool QSocketNotifier::isEnabled() const
{ return snenabled; }


#endif // QSOCKETNOTIFIER_H

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

#include "QtCore/qobject.h"


class Q_CORE_EXPORT QSocketNotifier : public QObject
{
    Q_OBJECT
public:
    enum Type { Read, Write, Exception };

    explicit QSocketNotifier(int socket, Type, QObject *parent = 0);
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

#endif // QSOCKETNOTIFIER_H

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

#ifndef QWSSOCKET_QWS_H
#define QWSSOCKET_QWS_H

#ifndef QT_H
#include "qsocket.h"
#include "qserversocket.h"
#endif // QT_H

#ifndef QT_NO_QWS_MULTIPROCESS

class QWSSocket : public QSocket
{
    Q_OBJECT
public:
    QWSSocket(QObject *parent=0);
#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QWSSocket(QObject *parent, const char *name);
#endif
    ~QWSSocket();

    virtual void connectToLocalFile(const QString &file);

private:
    Q_DISABLE_COPY(QWSSocket)
};


class QWSServerSocket : public QServerSocket
{
    Q_OBJECT
public:
    QWSServerSocket(const QString& file, int backlog = 0,
                     QObject *parent=0);
#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QWSServerSocket(const QString& file, int backlog,
                                          QObject *parent, const char *name);
#endif
    ~QWSServerSocket();

private:
    Q_DISABLE_COPY(QWSServerSocket)

    void init(const QString &file, int backlog);
};

#endif // QT_NO_QWS_MULTIPROCESS

#endif // QWSSOCKET_QWS_H

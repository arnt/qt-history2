/****************************************************************************
**
** Definition of QWSSocket and related classes.
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
    QWSSocket(QObject *parent=0, const char *name=0);
   ~QWSSocket();

    virtual void connectToLocalFile(const QString &file);

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QWSSocket(const QWSSocket &);
    QWSSocket &operator=(const QWSSocket &);
#endif
};


class QWSServerSocket : public QServerSocket
{
    Q_OBJECT
public:
    QWSServerSocket(const QString& file, int backlog = 0,
                     QObject *parent=0, const char *name=0);
   ~QWSServerSocket();

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QWSServerSocket(const QWSServerSocket &);
    QWSServerSocket &operator=(const QWSServerSocket &);
#endif
};

#endif // QT_NO_QWS_MULTIPROCESS

#endif // QWSSOCKET_QWS_H

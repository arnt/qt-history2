/****************************************************************************
**
** Definition of QDnsHostInfo class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QDNS_P_H
#define QDNS_P_H

#include "qdns.h"
#include <qmutex.h>
#include <qobject.h>
#include <qpointer.h>

#if !defined QT_NO_THREAD
#include <qthread.h>
#    define QDnsAgentBase QThread
#else
#    define QDnsAgentBase QObject
#endif

struct QDnsQuery
{
    inline QDnsQuery() : member(0) {}

    inline QDnsQuery(const QString &name, QObject *r, const char *m)
        : hostName(name), receiver(r), member(m) {}

    QString hostName;
    QPointer<QObject> receiver;
    const char *member;
};

class QDnsAgent : public QDnsAgentBase
{
    Q_OBJECT
public:
    inline QDnsAgent() {}

    void run();
    static QDnsHostInfo getHostByName(const QString &hostName);

    inline void addHostName(const QString &name,
                            QObject *receiver, const char *member)
    {
        QMutexLocker locker(&mutex);
        queries << QDnsQuery(name, receiver, member);
    }

signals:
    void resultsReady(QDnsHostInfo);

private:
    QList<QDnsQuery> queries;
    QMutex mutex;
};

class QDnsHostInfoPrivate
{
public:
    inline QDnsHostInfoPrivate()
        : err(QDnsHostInfo::NoError),
          errorStr(QT_TRANSLATE_NOOP("QDnsHostInfo", "Unknown error"))
    {
    }

    QDnsHostInfo::Error err;
    QString errorStr;
    QList<QHostAddress> addrs;
    QString hostName;
};

#endif

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

#ifndef QDNS_P_H
#define QDNS_P_H

#include <qcoreapplication.h>
#include <private/qcoreapplication_p.h>
#include "qdns.h"
#include <qmutex.h>
#include <qwaitcondition.h>
#include <qobject.h>
#include <qpointer.h>

#if !defined QT_NO_THREAD
#include <qthread.h>
#    define QDnsAgentBase QThread
#else
#    define QDnsAgentBase QObject
#endif

class QDnsResult : public QObject
{
    Q_OBJECT
public:
    inline void emitResultsReady(const QDnsHostInfo &info)
    {
        emit resultsReady(info);
    }
signals:
    void resultsReady(const QDnsHostInfo &info);
};

struct QDnsQuery
{
    inline QDnsQuery() : object(0) {}
    inline ~QDnsQuery() { delete object; }
    inline QDnsQuery(const QString &name, QDnsResult *result)
        : hostName(name), object(result) {}

    QString hostName;
    QDnsResult *object;
};

class QDnsAgent : public QDnsAgentBase
{
    Q_OBJECT
public:
    inline QDnsAgent()
    {
        connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(cleanup()));
        connect(QCoreApplication::instance(), SIGNAL(destroyed(QObject *)), this, SLOT(cleanup()));
        QCoreApplicationPrivate::moveToMainThread(this);
        quit = false;
    }
    inline ~QDnsAgent()
    { cleanup(); }

    void run();
    static QDnsHostInfo getHostByName(const QString &hostName);

    inline void addHostName(const QString &name, QDnsResult *result)
    {
        QMutexLocker locker(&mutex);
        queries << new QDnsQuery(name, result);
        cond.wakeOne();
    }

public slots:
    inline void cleanup()
    {
        {
            QMutexLocker locker(&mutex);
            queries.clear();
            quit = true;
            cond.wakeOne();
        }
        wait();
    }

private:
    QList<QDnsQuery *> queries;
    QMutex mutex;
    QWaitCondition cond;
    bool quit;
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

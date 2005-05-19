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

#ifndef QHOSTINFO_P_H
#define QHOSTINFO_P_H

#include <qcoreapplication.h>
#include <private/qcoreapplication_p.h>
#include "qhostinfo.h"
#include <qmutex.h>
#include <qwaitcondition.h>
#include <qobject.h>
#include <qpointer.h>

#if !defined QT_NO_THREAD
#include <qthread.h>
#    define QHostInfoAgentBase QThread
#else
#    define QHostInfoAgentBase QObject
#endif

class QHostInfoResult : public QObject
{
    Q_OBJECT
public:
    inline void emitResultsReady(const QHostInfo &info)
    {
        emit resultsReady(info);
    }

    int lookupId;
signals:
    void resultsReady(const QHostInfo &info);
};

struct QHostInfoQuery
{
    inline QHostInfoQuery() : object(0) {}
    inline ~QHostInfoQuery() { delete object; }
    inline QHostInfoQuery(const QString &name, QHostInfoResult *result)
        : hostName(name), object(result) {}

    QString hostName;
    QHostInfoResult *object;
};

class QHostInfoAgent : public QHostInfoAgentBase
{
    Q_OBJECT
public:
    inline QHostInfoAgent()
    {
        connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(cleanup()));
        connect(QCoreApplication::instance(), SIGNAL(destroyed(QObject *)), this, SLOT(cleanup()));
        QCoreApplicationPrivate::moveToMainThread(this);
        quit = false;
    }
    inline ~QHostInfoAgent()
    { cleanup(); }

    void run();
    static QHostInfo fromName(const QString &hostName);

    inline void addHostName(const QString &name, QHostInfoResult *result)
    {
        QMutexLocker locker(&mutex);
        queries << new QHostInfoQuery(name, result);
        cond.wakeOne();
    }

    inline void abortLookup(int id)
    {
        QMutexLocker locker(&mutex);
        for (int i = 0; i < queries.size(); ++i) {
            QHostInfoResult *result = queries.at(i)->object;
            if (result->lookupId == id) {
                result->disconnect();
                queries.removeAt(i);
                break;
            }                
        }
    }

public slots:
    inline void cleanup()
    {
        {
            QMutexLocker locker(&mutex);
            qDeleteAll(queries);
            queries.clear();
            quit = true;
            cond.wakeOne();
        }
        wait();
    }

private:
    QList<QHostInfoQuery *> queries;
    QMutex mutex;
    QWaitCondition cond;
    bool quit;
};

class QHostInfoPrivate
{
public:
    inline QHostInfoPrivate()
        : err(QHostInfo::NoError),
          errorStr(QT_TRANSLATE_NOOP("QHostInfo", "Unknown error"))
    {
    }

    QHostInfo::HostInfoError err;
    QString errorStr;
    QList<QHostAddress> addrs;
    QString hostName;
    int lookupId;
};

#endif // QHOSTINFO_P_H

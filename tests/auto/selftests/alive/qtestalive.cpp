/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qcoreapplication.h>
#include <qcoreevent.h>
#include <qthread.h>

class QTestAliveEvent: public QEvent
{
public:

    enum { AliveEventType = QEvent::User + 422 };

    inline QTestAliveEvent(int aSequenceId)
        : QEvent(QEvent::Type(AliveEventType)), seqId(aSequenceId)
    {}
    inline int sequenceId() const { return seqId; }

private:
    int seqId;
};

class QTestAlivePinger: public QObject
{
public:
    QTestAlivePinger(QObject *receiver, QObject *parent = 0);
    bool event(QEvent *e);

protected:
    void timerEvent(QTimerEvent *event);

private:
    QObject *rec;
    int timerId;
    int currentSequenceId;
    int lastSequenceId;
};

QTestAlivePinger::QTestAlivePinger(QObject *receiver, QObject *parent)
    : QObject(parent), rec(receiver), currentSequenceId(0), lastSequenceId(0)
{
    Q_ASSERT(rec);
    timerId = startTimer(850);
}

bool QTestAlivePinger::event(QEvent *event)
{
    // pong received
    if (int(event->type()) == QTestAliveEvent::AliveEventType) {
        QTestAliveEvent *e = static_cast<QTestAliveEvent *>(event);
        //qDebug("PONG %d received", e->sequenceId());
        // if the events are not delivered in order, we don't care.
        if (e->sequenceId() > lastSequenceId)
            lastSequenceId = e->sequenceId();
        return true;
    }
    return QObject::event(event);
}

void QTestAlivePinger::timerEvent(QTimerEvent *event)
{
    if (event->timerId() != timerId)
        return;

    if (lastSequenceId < currentSequenceId - 2) {
        qWarning("TEST LAGS %d PINGS behind!", currentSequenceId - lastSequenceId);
    }
    ++currentSequenceId;
    //qDebug("PING %d", currentSequenceId);
    QCoreApplication::postEvent(rec, new QTestAliveEvent(currentSequenceId));
}

class QTestAlive: public QThread
{
public:
    QTestAlive(QObject *parent = 0);
    ~QTestAlive();
    void run();

    bool event(QEvent *e);

private:
    QTestAlivePinger *pinger;
};

QTestAlive::QTestAlive(QObject *parent)
    : QThread(parent), pinger(0)
{
}

QTestAlive::~QTestAlive()
{
    quit();
    while (isRunning());
}

bool QTestAlive::event(QEvent *e)
{
    if (int(e->type()) == QTestAliveEvent::AliveEventType && pinger) {
        // ping received, send back the pong
        //qDebug("PONG %d", static_cast<QTestAliveEvent *>(e)->sequenceId());
        QCoreApplication::postEvent(pinger,
                new QTestAliveEvent(static_cast<QTestAliveEvent *>(e)->sequenceId()));
        return true;
    }
    return QThread::event(e);
}

void QTestAlive::run()
{
    Q_ASSERT_X(QCoreApplication::instance(), "QTestAlive::run()",
               "Cannot start QTestAlive without a QCoreApplication instance.");

    QTestAlivePinger p(this);
    pinger = &p;
    exec();
    pinger = 0;
}



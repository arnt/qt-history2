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

#ifndef QTESTEVENTLOOP_H
#define QTESTEVENTLOOP_H

#include <QtCore/qcoreapplication.h>
#include <QtCore/qeventloop.h>
#include <QtCore/qobject.h>
#include <QtCore/qpointer.h>

class QTestEventLoop : public QObject
{
public:
    inline QTestEventLoop(QObject *aParent = 0)
        : QObject(aParent), inLoop(false), _timeout(false), timerId(-1), loop(0) {}
    inline void enterLoop(int secs);
    inline void exitLoop();

    inline void changeInterval(int secs)
    { killTimer(timerId); timerId = startTimer(secs * 1000); }

    inline bool timeout() const
    { return _timeout; }

    inline static QTestEventLoop &instance()
    {
        static QPointer<QTestEventLoop> testLoop;
        if (testLoop.isNull())
            testLoop = new QTestEventLoop(QCoreApplication::instance());
        return *static_cast<QTestEventLoop *>(testLoop);
    }

protected:
    inline void timerEvent(QTimerEvent *e);

private:
    bool inLoop;
    bool _timeout;
    int timerId;

    QEventLoop *loop;
};

inline void QTestEventLoop::enterLoop(int secs)
{
    Q_ASSERT(!loop);

    QEventLoop l;

    inLoop = true;
    _timeout = false;

    timerId = startTimer(secs * 1000);

    loop = &l;
    l.exec();
    loop = 0;
}

inline void QTestEventLoop::exitLoop()
{
    Q_ASSERT(loop);

    killTimer(timerId); timerId = -1;

    loop->exit();

    inLoop = false;
}

inline void QTestEventLoop::timerEvent(QTimerEvent *e)
{
    if (e->timerId() != timerId)
        return;
    _timeout = true;
    exitLoop();
}

#endif

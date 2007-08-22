/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QWINDOWSPIPEWRITER_P_H
#define QWINDOWSPIPEWRITER_P_H

#include <qdatetime.h>
#include <qthread.h>
#include <qmutex.h>
#include <qwaitcondition.h>
#include <qt_windows.h>

QT_BEGIN_HEADER

QT_MODULE(Core)

#define SLEEPMIN 10
#define SLEEPMAX 500

class QIncrementalSleepTimer
{

public:
    QIncrementalSleepTimer(int msecs)
        : totalTimeOut(msecs)
        , nextSleep(qMin(SLEEPMIN, totalTimeOut))
    {
        timer.start();
    }

    int nextSleepTime()
    {
        if (totalTimeOut == -1)
            return -1;

        int tmp = nextSleep;
        nextSleep = qMin(nextSleep * 2, qMin(SLEEPMAX, timeLeft()));
        return tmp;
    }

    int timeLeft() const
    {
        return qMax(totalTimeOut - timer.elapsed(), 0);
    }

    bool hasTimedOut() const
    {
        return timer.elapsed() >= totalTimeOut;
    }

    void resetIncrements()
    {
        nextSleep = qMin(SLEEPMIN, timeLeft());
    }

private:
    QTime timer;
    int totalTimeOut;
    int nextSleep;
};

class Q_CORE_EXPORT QWindowsPipeWriter : public QThread
{
    Q_OBJECT

Q_SIGNALS:
    void canWrite();

public:
    QWindowsPipeWriter(HANDLE writePipe, QObject * parent = 0);
    ~QWindowsPipeWriter();

    bool waitForWrite(int msecs);
    qint64 write(const char *data, qint64 maxlen);

    qint64 bytesToWrite() const
    {
        QMutexLocker locker(&lock);
        return data.size();
    }

    bool hadWritten() const
    {
        return hasWritten;
    }

protected:
   void run();

private:
    QByteArray data;
    QWaitCondition waitCondition;
    mutable QMutex lock;
    HANDLE writePipe;
    bool quitNow;
    bool hasWritten;
};

QT_END_HEADER

#endif // QT_NO_PROCESS


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

#ifndef QTHREAD_H
#define QTHREAD_H

#ifndef QT_H
#include <qobject.h>
#endif // QT_H

#include <limits.h>

class QThreadData;
class QThreadPrivate;

class Q_CORE_EXPORT QThread : public QObject
{
    friend class QThreadData;

    Q_OBJECT
    Q_DECLARE_PRIVATE(QThread)
    Q_DISABLE_COPY(QThread)

public:
    static Qt::HANDLE currentThread();
    static QThread *currentQThread(); // better name?

    static void initialize();
    static void cleanup();

    QThread();
    virtual ~QThread();

    enum Priority {
        IdlePriority,

        LowestPriority,
        LowPriority,
        NormalPriority,
        HighPriority,
        HighestPriority,

        TimeCriticalPriority,

        InheritPriority
    };

    bool isFinished() const;
    bool isRunning() const;

    void setStackSize(uint stackSize);
    uint stackSize() const;

public slots:
    void start(Priority = InheritPriority);
    void terminate();

public:
    // default argument causes thread to block indefinately
    bool wait(unsigned long time = ULONG_MAX);

signals:
    void started();
    void finished();
    void terminated();

protected:
    virtual void run() = 0;

    static void exit();

    static void sleep(unsigned long);
    static void msleep(unsigned long);
    static void usleep(unsigned long);
#ifdef QT_COMPAT
public:
    inline QT_COMPAT bool finished() const { return isFinished(); }
    inline QT_COMPAT bool running() const { return isRunning(); }
#endif
};

#endif // QTHREAD_H

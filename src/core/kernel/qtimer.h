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

#ifndef QTIMER_H
#define QTIMER_H

#ifndef QT_NO_QOBJECT

#include "QtCore/qbasictimer.h" // conceptual inheritance
#include "QtCore/qobject.h"


class Q_CORE_EXPORT QTimer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool singleShot READ isSingleShot WRITE setSingleShot)
    Q_PROPERTY(int interval READ interval WRITE setInterval)
public:
    explicit QTimer(QObject *parent = 0);
#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QTimer(QObject *parent, const char *name);
#endif
    ~QTimer();

    inline bool isActive() const { return id >= 0; }
    int timerId() const { return id; }

    void setInterval(int msec);
    int interval() const { return inter; }

    inline void setSingleShot(bool singleShot) { single = singleShot; }
    inline bool isSingleShot() const { return single; }

    static void singleShot(int msec, QObject *receiver, const char *member);

public slots:
    void start(int msec);

    void start();
    void stop();

#ifdef QT_COMPAT
    inline QT_MOC_COMPAT void changeInterval(int msec) { start(msec); };
    QT_MOC_COMPAT int start(int msec, bool sshot);
#endif

signals:
    void timeout();

protected:
    void timerEvent(QTimerEvent *);

private:
    Q_DISABLE_COPY(QTimer)

    inline int startTimer(int){ return -1;}
    inline void killTimer(int){}

    int id, inter, del;
    uint single : 1;
    uint nulltimer : 1;
};

#endif

#endif // QTIMER_H

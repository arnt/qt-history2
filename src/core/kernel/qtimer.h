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

#ifndef QT_H
#include "qbasictimer.h" // conceptual inheritance
#include "qobject.h"
#endif // QT_H


class Q_CORE_EXPORT QTimer : public QObject
{
    Q_OBJECT

public:
    QTimer(QObject *parent = 0);
#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QTimer(QObject *parent, const char *name);
#endif
    ~QTimer();

    static void singleShot(int msec, QObject *receiver, const char *member);

    inline bool isActive() const { return id >= 0; }
    int timerId() const { return id; }

public slots:
    int                start(int msec, bool sshot = false);
    void        changeInterval(int msec);
    void        stop();

signals:
    void        timeout();

protected:
    bool        event(QEvent *);

private:
    Q_DISABLE_COPY(QTimer)

    int id;
    uint single : 1;
    uint nulltimer : 1;
};

#endif // QTIMER_H

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

#ifndef QWINEVENTNOTIFIER_H
#define QWINEVENTNOTIFIER_H

#include "qobject.h"
#include "qt_windows.h"

class Q_CORE_EXPORT QWinEventNotifier : public QObject
{
    Q_OBJECT
public:

    QWinEventNotifier(QObject *parent = 0);
    QWinEventNotifier(HANDLE hEvent, QObject *parent = 0);
    ~QWinEventNotifier();

    void setHandle(HANDLE hEvent);
    HANDLE handle() const;

    bool isEnabled() const;
    void setEnabled(bool enable);

signals:
    void activated(HANDLE hEvent);

protected:
    bool event(QEvent * e);

private:
    Q_DISABLE_COPY(QWinEventNotifier)

    HANDLE handleToEvent;
    bool enabled;
};


#endif // QWINEVENTNOTIFIER_H

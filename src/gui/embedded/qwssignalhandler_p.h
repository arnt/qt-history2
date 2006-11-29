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

#ifndef QWSSIGNALHANDLER_P_H
#define QWSSIGNALHANDLER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qmap.h>
#include <QtCore/qvector.h>
#include <QtCore/qobjectcleanuphandler.h>
#include <signal.h>

class QWSSignalHandlerPrivate;

class QWSSignalHandler
{
public:
    static QWSSignalHandler* instance();

    ~QWSSignalHandler();

    inline void addSemaphore(int semno) { semaphores.append(semno); }
    void removeSemaphore(int semno);
    inline void addObject(QObject *object) { (void)objects.add(object); }

private:
    QWSSignalHandler();
    static void handleSignal(int signal);
    QMap<int, sighandler_t> oldHandlers;
    QVector<int> semaphores;
    QObjectCleanupHandler objects;

    friend class QWSSignalHandlerPrivate;
};

#endif // QWSSIGNALHANDLER_P_H

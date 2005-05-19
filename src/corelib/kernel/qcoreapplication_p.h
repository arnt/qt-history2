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

#ifndef QCOREAPPLICATION_P_H
#define QCOREAPPLICATION_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qobject_p.h"
#include "qcoreapplication.h"
#include "qtranslator.h"

typedef QList<QTranslator*> QTranslatorList;

class QAbstractEventDispatcher;

class Q_CORE_EXPORT QCoreApplicationPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QCoreApplication)
public:
    QCoreApplicationPrivate(int &aargc,  char **aargv);
    ~QCoreApplicationPrivate();

    bool notify_helper(QObject *, QEvent *);

    virtual void createEventDispatcher();
    static void moveToMainThread(QObject *o);
    static void removePostedEvent(QEvent *);

    static QThread *mainThread();
    static bool checkInstance(const char *method);

#ifdef QT3_SUPPORT
    void removePostedChildInsertedEvents(QObject *receiver, QObject *child);
#endif
    void checkReceiverThread(QObject *receiver);

    int &argc;
    char **argv;
    QString orgName, orgDomain, application;
#ifndef QT_NO_TRANSLATION
    QTranslatorList translators;
#endif
#ifndef QT_NO_COMPONENT
    QStringList *app_libpaths;
#endif

    QCoreApplication::EventFilter eventFilter;

    static QAbstractEventDispatcher *eventDispatcher;
    static bool is_app_running;
    static bool is_app_closing;
};

#endif // QCOREAPPLICATION_P_H

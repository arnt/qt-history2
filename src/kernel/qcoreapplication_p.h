#ifndef QKERNELAPPLICATION_P_H
#define QKERNELAPPLICATION_P_H

#include "qobject_p.h"
#include "qcoreapplication.h"
#include "qtranslator.h"
#include "qmetaobject.h"

#include <qvector.h>
#if defined(QT_THREAD_SUPPORT)
#  include <qmutex.h>
#endif

struct QPostEvent
{
    QObject *receiver;
    QEvent *event;
    inline QPostEvent()
	: receiver(0), event(0)
    { }
    inline QPostEvent(QObject *r, QEvent *e)
	: receiver(r), event(e)
    { }
};

class QPostEventList : public QVector<QPostEvent>
{
public:
    QEventLoop *eventloop;

    int offset;
#if defined(QT_THREAD_SUPPORT)
    QMutex mutex;
#endif

    inline QPostEventList()
	: QVector<QPostEvent>(), eventloop(0), offset(0)
    { }
    ~QPostEventList();
};

class Q_CORE_EXPORT QTranslatorList : private QList<QTranslator*>
{
public:
    using QList<QTranslator*>::prepend;
    using QList<QTranslator*>::remove;
    using QList<QTranslator*>::isEmpty;
    using QList<QTranslator*>::constBegin;
    using QList<QTranslator*>::constEnd;
};

class Q_CORE_EXPORT QCoreApplicationPrivate : public QObjectPrivate
{
    Q_DECL_PUBLIC(QCoreApplication);
public:
    QCoreApplicationPrivate(int &aargc,  char **aargv);
    ~QCoreApplicationPrivate() {}

    int &argc;
    char **argv;
#ifndef QT_NO_TRANSLATION
    QTranslatorList translators;
#endif
#ifndef QT_NO_COMPONENT
    QStringList *app_libpaths;
#endif
};


#endif

#ifndef QCOREAPPLICATION_P_H
#define QCOREAPPLICATION_P_H

#include "qobject_p.h"
#include "qcoreapplication.h"
#include "qtranslator.h"
#include "qmetaobject.h"

class Q_CORE_EXPORT QTranslatorList : private QList<QTranslator*>
{
public:
    using QList<QTranslator*>::prepend;
    using QList<QTranslator*>::removeAll;
    using QList<QTranslator*>::isEmpty;
    using QList<QTranslator*>::constBegin;
    using QList<QTranslator*>::constEnd;
};

class Q_CORE_EXPORT QCoreApplicationPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QCoreApplication)
public:
    QCoreApplicationPrivate(int &aargc,  char **aargv);
    ~QCoreApplicationPrivate() {}

    int &argc;
    char **argv;
    QEventLoop *eventLoop;
#ifndef QT_NO_TRANSLATION
    QTranslatorList translators;
#endif
#ifndef QT_NO_COMPONENT
    QStringList *app_libpaths;
#endif
};

#endif

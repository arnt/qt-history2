#ifndef QKERNELAPPLICATION_P_H
#define QKERNELAPPLICATION_P_H

#include "qobject_p.h"
#include "qkernelapplication.h"
#include "qtranslator.h"

class QKernelApplicationPrivate : public QObjectPrivate
{
    Q_DECL_PUBLIC(QKernelApplication);
public:
    QKernelApplicationPrivate(int &aargc,  char **aargv);
    ~QKernelApplicationPrivate() {}

    int &argc;
    char **argv;
#ifndef QT_NO_TRANSLATION
    QList<QTranslator*> translators;
#endif
#ifndef QT_NO_COMPONENT
    QStringList *app_libpaths;
#endif
};


#endif

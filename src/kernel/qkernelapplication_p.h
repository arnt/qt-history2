#ifndef QKERNELAPPLICATION_P_H
#define QKERNELAPPLICATION_P_H

#include "qobject_p.h"
#include "qkernelapplication.h"

class QKernelApplicationPrivate : public QObjectPrivate
{
    Q_DECL_PUBLIC(QKernelApplication);
public:
    QKernelApplicationPrivate(int &aargc,  char **aargv);
    ~QKernelApplicationPrivate() {}

    int &argc;
    char **argv;
};


#endif

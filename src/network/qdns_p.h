#ifndef QRESOLVER_P_H
#define QRESOLVER_P_H

#include "qresolver.h"
#include <qstring.h>
#include <qobject.h>

#if !defined QT_NO_THREAD
#include <qthread.h>
#    define QResolverAgentBase QThread
#else
#    define QResolverAgentBase QObject
#endif

class QResolverAgent : public QResolverAgentBase
{
    Q_OBJECT
public:
    inline QResolverAgent(const QString &name) { hostName = name; }

    void run();

signals:
    void resultsReady(QResolverHostInfo);

private:
    QString hostName;
};

#endif // QRESOLVER_P_H

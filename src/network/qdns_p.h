#ifndef QDNS_P_H
#define QDNS_P_H

#include "qdns.h"
#include <qstring.h>
#include <qobject.h>

#if !defined QT_NO_THREAD
#include <qthread.h>
#    define QDnsAgentBase QThread
#else
#    define QDnsAgentBase QObject
#endif

class QDnsAgent : public QDnsAgentBase
{
    Q_OBJECT
public:
    inline QDnsAgent(const QString &name) { hostName = name; }

    void run();

signals:
    void resultsReady(QDnsHostInfo);

private:
    QString hostName;
};

#endif // QDNS_P_H

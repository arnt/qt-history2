#include "qresolver.h"
#include "qresolver_p.h"
#include <qmetaobject.h>

void *qt_copy_QResolverHostInfo(const void *voidCopy)
{
    QResolverHostInfo *copy = (QResolverHostInfo *)voidCopy;
    QResolverHostInfo *info = new QResolverHostInfo(*copy);
    return info;
}

void qt_destroy_QResolverHostInfo(void *)
{
}

/*!
    Looks up the hostname \a name. When the results of the lookup are
    ready, the slot or singal \a resultsReady in \a receiver is invoked.
*/
void QResolver::getHostByName(const QString &name, const QObject *receiver,
                              const char *resultsReady )
{
    QResolverAgent *agent = new QResolverAgent(name);

    QMetaType::registerType("QResolverHostInfo",
                            qt_destroy_QResolverHostInfo,
                            qt_copy_QResolverHostInfo);
    
    QObject::connect(agent, SIGNAL(resultsReady(QResolverHostInfo)),
                     receiver, resultsReady);


#if !defined QT_NO_THREAD
    agent->start();
#else
    agent->run();
#endif
}

QResolverHostInfo::QResolverHostInfo()
    : error( QResolver::NoError ), errorString( "Unknown error" )
{
}

QResolverHostInfo::QResolverHostInfo(const QResolverHostInfo &d)
    : error( d.error ), errorString( d.errorString ), addresses( d.addresses )
{
}


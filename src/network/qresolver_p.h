#ifndef QRESOLVER_P_H
#define QRESOLVER_P_H

//#define QRESOLVER_DEBUG

#include "qresolver.h"
#include <qthread.h>
#include <qapplication.h>
#if defined(Q_WS_WIN)
#include <qwidget.h>
#endif

class QResolverEvent : public QCustomEvent
{
public:
    QResolverEvent()
	: QCustomEvent( ResolverEventType )
    { }

    QResolver::HostInfo resData;

    static const int ResolverEventType; // #### change this when moved into Qt
};

class QResolverDispatcher : public QObject
{
    Q_OBJECT
public:
    QResolverDispatcher( QObject *parent, const QString &name )
	: QObject( parent, "DNS dispatcher" ), hostName(name)
    { }
    ~QResolverDispatcher()
    { }

signals:
    void resultsReady( const QResolver::HostInfo &hostInfo );

private:
    QString hostName;

    friend class QResolverManager;
};

#if !( defined(Q_WS_WIN) && defined(QT_NO_IPV6) )
class QResolverSync
#ifdef QT_THREAD_SUPPORT
    : public QThread
#endif
{
public:
    QResolverSync()
    { }
    ~QResolverSync()
    { }

    void run();
    QString hostName;
};
#endif // !( defined(Q_WS_WIN) && defined(QT_NO_IPV6) )

#if defined(Q_WS_WIN)
typedef QWidget QResolverManagerBase;
#else
typedef QObject QResolverManagerBase;
#endif

class QResolverManager : public QResolverManagerBase
{
    Q_OBJECT
public:
    static QResolverManager *manager();
    void addQuery( const QString& name, const QObject * receiver, const char *resultsReady );
    ~QResolverManager();
    bool event( QEvent *e );

protected:
#if defined(Q_WS_WIN)
    bool winEvent( MSG *msg );
#endif

private:
    QResolverManager();
    void startNextQuery();

    QList<QResolverDispatcher *> queries;
#if !( defined(Q_WS_WIN) && defined(QT_NO_IPV6) )
    QResolverSync *resolverThread;
#endif

#if defined(Q_WS_WIN)
    static uint winMsg;
    HANDLE requestHandle;
    char *buf;
#endif
};

#endif // QRESOLVER_P_H

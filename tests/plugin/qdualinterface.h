#ifndef QCLIENTINTERFACE_H
#define QCLIENTINTERFACE_H

#include <qobject.h>
#include <qvariant.h>

class QDualInterface : public QObject
{
    Q_OBJECT
signals:
    void readProperty( const QCString&, QVariant& );
    void writeProperty( const QCString&, const QVariant& );
    void makeConnection( const char*, QObject*, const char* );
    void eventFilter( QObject* );

public slots:
    virtual void requestProperty( const QCString&, QVariant& ) = 0;
    virtual void requestSetProperty( const QCString&, const QVariant& ) = 0;
    virtual void requestSignal( const char*, QObject*, const char* ) = 0;
    virtual void requestEvents( QObject* ) = 0;
    
    /* 
    Some ideas for on-demand connections.
    It's quite reasonable to assume that an application won't
    provide the same interface all the time (as the accessed objects
    change).
    
    The application interface should create the interface
    if it wants to, and the plugin can use its clientInterface 
    function to access it (this way, no pointer has to be 
    passed from the application to the plugin!)

    E.g. We have a stable connection to the Designer application
    interface, and we want to access the active form -> the client
    interface sends a request to the application interface, which
    creates the forminterface for the active form. Now the plugin
    can try to get the new interface using the clientInterface()
    function!
    */
    virtual void requestInterface( const QCString& ) {}
    /*
    The other way round. The plugin doesn't need the interface
    any more, so the application can delete it.
    Hmmm... maybe the clientinterface-dict of the plugin should
    be a QDict<<QGuardedPtr<QClientInterface>>, then the application
    can cut an interface (e.g. when the form the interface accesses
    is closed) without leaving the plugin in an undefined state!
    */
    virtual void abortInterface( const QCString& ) {}
};

class QClientInterface : public QDualInterface
{
public:
    void requestProperty( const QCString& p, QVariant& v ) 
    {
	emit readProperty( p, v );
    }
    void requestSetProperty( const QCString& p, const QVariant& v )
    {
	emit writeProperty( p, v );
    }
    void requestSignal( const char* signal, QObject* target, const char* slot )
    {
	emit makeConnection( signal, target, slot );
    }
    void requestEvents( QObject* o )
    {
	emit eventFilter( o );
    }
};

#endif //QCLIENTINTERFACE_H

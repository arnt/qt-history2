#ifndef QCLIENTINTERFACE_H
#define QCLIENTINTERFACE_H

#include <qobject.h>
#include <qvariant.h>

class QClientInterface;

class QDualInterface : public QObject
{
    Q_OBJECT
signals:
    void sendRequestProperty( const QCString&, QVariant& );
    void sendRequestSetProperty( const QCString&, const QVariant& );
    void sendRequestSignal( const char*, QObject*, const char* );
    void sendRequestEvents( QObject* );
    void sendRequestConnection( const QCString&, QClientInterface* );

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
    
    The plugin can create a QClientInterface and ask the application 
    interface to connect the new client to a requested interface the
    existing ai knows about (e.g. a mainwindow knows about its menubar)
    The connection will automatically be canceled when the plugin does
    no longer need the interface and deletes it.

    I.e. We have a stable connection to the Designer application
    interface, and we want to access the active form -> the client
    interface sends a request to the application interface, which
    creates the forminterface for the active form. Now the plugin
    can try to get the new interface using the clientInterface()
    function!
    */
    virtual void requestConnection( const QCString&, QClientInterface* ) = 0;
};

class QClientInterface : public QDualInterface
{
public:
    QClientInterface()
    {
	qDebug( "Here comes the client interface!" );
    }
    ~QClientInterface()
    {
	qDebug( "There goes the client interface!" );
    }
    void requestProperty( const QCString& p, QVariant& v ) 
    {
	emit sendRequestProperty( p, v );
    }
    void requestSetProperty( const QCString& p, const QVariant& v )
    {
	emit sendRequestSetProperty( p, v );
    }
    void requestSignal( const char* signal, QObject* target, const char* slot )
    {
	emit sendRequestSignal( signal, target, slot );
    }
    void requestEvents( QObject* o )
    {
	emit sendRequestEvents( o );
    }
    void requestConnection( const QCString& request, QClientInterface* plug )
    {
	emit sendRequestConnection( request, plug );
    }
};

#endif //QCLIENTINTERFACE_H

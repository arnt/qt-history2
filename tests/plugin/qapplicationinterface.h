#ifndef QAPPLICATIONINTERFACES_H
#define QAPPLICATIONINTERFACES_H

#include "qdualinterface.h"
#include "qplugininterface.h"
#include "qguardedptr.h"

class QApplicationInterface : public QDualInterface
{
    Q_OBJECT
public:
    QApplicationInterface( QObject* o )
    {
	ref = 0;
	theObject = o;
#ifdef CHECK_RANGE
	if ( !o )
	    qWarning( "Can't create interface with null-object!" );
#endif CHECK_RANGE

	theObject->insertChild( this );
    }

    void requestProperty( const QCString& p, QVariant& v )
    {
	v = theObject->property( p );
    }
    void requestSetProperty( const QCString& p, const QVariant& v )
    {
	theObject->setProperty( p, v );
    }
    void requestSignal( const char* signal, QObject* target, const char* slot )
    {
	connect( theObject, signal, target, slot );
    }
    void requestEvents( QObject* o )
    {
	theObject->installEventFilter( o );
    }
    void requestConnection( const QCString& request, QClientInterface* pi )
    {
    }

    void connectToClient( QClientInterface* client )
    {
	ref++;
	connect( client, SIGNAL(destroyed()), this, SLOT(clientDestroyed()) );

	connect( client, SIGNAL(sendRequestProperty(const QCString&,QVariant&)), 
	    this, SLOT(requestProperty(const QCString&,QVariant&)) );
	connect( client, SIGNAL(sendRequestSetProperty(const QCString&, const QVariant&)), 
	    this, SLOT(requestSetProperty(const QCString&, const QVariant&)) );
	connect( client, SIGNAL(sendRequestSignal(const char*,QObject*,const char*)), 
	    this, SLOT(requestSignal(const char*,QObject*,const char*)) );
	connect( client, SIGNAL(sendRequestEvents(QObject*)), 
	    this, SLOT(requestEvents(QObject*)) );
	connect( client, SIGNAL(sendRequestConnection(const QCString&,QClientInterface*)), 
	    this, SLOT(requestConnection(const QCString&,QClientInterface*)) );
    }

protected:
    QObject* object() const { return theObject; }

private slots:
    void clientDestroyed()
    {
	ref--;
	if ( !ref )
	    delete this;
    }

private:
    QGuardedPtr<QObject> theObject;
    int ref;
};

#endif //QAPPLICATIONINTERFACES_H

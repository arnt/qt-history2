#ifndef QAPPLICATIONINTERFACES_H
#define QAPPLICATIONINTERFACES_H

#include <qobject.h>
#include <qguardedptr.h>
#include <qvariant.h>

class QApplicationInterface : public QObject
{
public:
    QApplicationInterface( QObject* o )
	: QObject( o )
    {
    #ifdef CHECK_RANGE
	if ( !o )
	    qWarning( "Can't create interface with null-object!" );
    #endif CHECK_RANGE
    }

    virtual QVariant requestProperty( const QCString& p )
    {
	return parent()->property( p );
    }
    virtual void requestSetProperty( const QCString& p, const QVariant& v )
    {
	parent()->setProperty( p, v );
    }
    virtual void requestConnect( const char* signal, QObject* target, const char* slot )
    {
	connect( parent(), signal, target, slot );
    }
    virtual void requestEvents( QObject* o )
    {
	parent()->installEventFilter( o );
    }
    virtual QApplicationInterface* requestInterface( const QCString& request )
    {
	return 0;
    }

protected:
    QObject* parent() { return QObject::parent(); }
};

#endif //QAPPLICATIONINTERFACES_H

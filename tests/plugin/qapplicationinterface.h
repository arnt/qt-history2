#ifndef QAPPLICATIONINTERFACES_H
#define QAPPLICATIONINTERFACES_H

#include "qdualinterface.h"

class QApplicationInterface : public QDualInterface
{
public:
    QApplicationInterface( QObject* o )
    {
	theObject = o;
#ifdef CHECK_RANGE
	if ( !o )
	    qWarning( "Can't create interface with null-object!" );
#endif CHECK_RANGE
    }

    void requestProperty( const QCString& p, QVariant& v )
    {
	if ( theObject )
	    v = theObject->property( p );
    }
    void requestSetProperty( const QCString& p, const QVariant& v )
    {
	if ( theObject )
	    theObject->setProperty( p, v );
    }
    void requestSignal( const char* signal, QObject* target, const char* slot )
    {
	if ( theObject )
	    connect( theObject, signal, target, slot );
    }

protected:
    QObject* object() const { return theObject; }

private:
    QObject* theObject;
};

#endif //QAPPLICATIONINTERFACES_H

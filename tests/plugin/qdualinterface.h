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
    
    virtual void requestConnection( const QCString&, QClientInterface* ) = 0;
};

class QClientInterface : public QDualInterface
{
public:
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

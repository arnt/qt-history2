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

#ifndef QAPPLICATIONINTERFACES_H
#define QAPPLICATIONINTERFACES_H

#include <qobject.h>
#include <qvariant.h>

class Q_EXPORT QApplicationInterface : public QObject
{
public:
    QApplicationInterface( QObject* o );

    virtual QVariant requestProperty( const QCString& p );
    virtual bool requestSetProperty( const QCString& p, const QVariant& v );
    virtual bool requestConnect( const char* signal, QObject* target, const char* slot );
    virtual bool requestEvents( QObject* o );
    virtual QApplicationInterface* requestInterface( const QCString& request );

protected:
    QObject* parent() { return QObject::parent(); }
};

#endif //QAPPLICATIONINTERFACES_H

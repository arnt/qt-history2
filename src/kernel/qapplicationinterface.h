#ifndef QAPPLICATIONINTERFACES_H
#define QAPPLICATIONINTERFACES_H

#include <qobject.h>

#ifndef QT_NO_PLUGIN

#include <qvariant.h>

class Q_EXPORT QApplicationInterface : public QObject
{
public:
    QApplicationInterface( QObject* o );

#ifndef QT_NO_PROPERTIES
    virtual QVariant requestProperty( const QCString& p );
    virtual bool requestSetProperty( const QCString& p, const QVariant& v );
#endif
    virtual bool requestConnect( const char* signal, QObject* target, const char* slot );
    virtual bool requestEvents( QObject* o );
    virtual QApplicationInterface* requestInterface( const QCString& request );

protected:
    QObject* parent() { return QObject::parent(); }
};

#endif

#endif //QAPPLICATIONINTERFACES_H

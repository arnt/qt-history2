#ifndef QCOMPONENTINTERFACE_H
#define QCOMPONENTINTERFACE_H

#ifndef QT_H
#include "qvariant.h"
#include "qobject.h"
#include "qguardedptr.h"
#endif // QT_H

#ifndef QT_NO_PLUGIN

class QApplicationInterface;

class Q_EXPORT QUnknownInterface
{
public:
    QUnknownInterface( QUnknownInterface *parent = 0 );
    virtual ~QUnknownInterface();

    virtual QString interfaceID() const;

    virtual bool connectNotify( QApplicationInterface* );
    virtual bool disconnectNotify();

    virtual QUnknownInterface* queryInterface( const QString& );
    virtual QStringList interfaceList() const;
};

class Q_EXPORT QPlugInInterface : public QUnknownInterface
{
public:
    QPlugInInterface();

    QString interfaceID() const;

    virtual QString name() const;
    virtual QString description() const;
    virtual QString author() const;

    virtual QUnknownInterface* queryInterface( const QString& ) = 0;
    virtual QStringList interfaceList() const = 0;
};

class Q_EXPORT QApplicationInterface : public QUnknownInterface
{
public:
    QApplicationInterface();
    QString interfaceID() const;

    virtual QString name() const;
    virtual QString description() const;
    virtual QString author() const;
    virtual QString workDirectory() const;
    virtual QString version() const;
    virtual QString command() const;
};

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QGuardedPtr<QObject>;
// MOC_SKIP_END
#endif

class Q_EXPORT QApplicationComponentInterface : public QUnknownInterface
{
public:
    QApplicationComponentInterface( QObject* c );

    QString interfaceID() const;

#ifndef QT_NO_PROPERTIES
    virtual QVariant requestProperty( const QCString& p );
    virtual bool requestSetProperty( const QCString& p, const QVariant& v );
#endif
    virtual bool requestConnect( const char* signal, QObject* target, const char* slot );
    virtual bool requestConnect( QObject *sender, const char* signal, const char* slot );
    virtual bool requestEvents( QObject* o );

protected:
    QObject* component() const { return comp; }

private:
    QGuardedPtr<QObject> comp;
};

#ifndef Q_EXPORT_INTERFACE
    #ifdef _WS_WIN_
	#define Q_EXPORT_INTERFACE( IMPLEMENTATION ) \
	    extern "C" __declspec(dllexport) QPlugInInterface *qt_load_interface() { return new IMPLEMENTATION; }
    #else
	#define Q_EXPORT_INTERFACE( IMPLEMENTATION ) \
	    extern "C" QPlugInInterface *qt_load_interface() { return new IMPLEMENTATION; }
    #endif
#endif

#endif

#endif //QCOMPONENTINTERFACE_H

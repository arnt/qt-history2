#ifndef QCOMPONENTINTERFACE_H
#define QCOMPONENTINTERFACE_H

#ifndef QT_H
#include "qvariant.h"
#include "qobject.h"
#include "qguardedptr.h"
#endif // QT_H

#ifndef QT_NO_PLUGIN

class QApplicationInterface;
class QInterfaceList;
class QRegExp;

class Q_EXPORT QUnknownInterface
{
    friend class QPlugIn;
public:
    QUnknownInterface( QUnknownInterface *parent = 0, const char* name = 0 );
    virtual ~QUnknownInterface();

    virtual QString interfaceID() const;
    QString demangledID( QString *unique = 0, QString *hierarchy = 0 ) const;

    virtual bool initialize( QApplicationInterface* = 0 );
    virtual bool cleanUp( QApplicationInterface* = 0 );

    virtual bool hasInterface( const QRegExp&, bool rec = TRUE ) const;
    virtual QUnknownInterface* queryInterface( const QRegExp&, bool rec = TRUE );
    virtual QStringList interfaceList( bool rec = TRUE) const;

    bool release();

    QApplicationInterface *applicationInterface() const;

    QUnknownInterface *parent() const;

    const char *name() const;   

protected:
    void insertChild( QUnknownInterface * );
    void removeChild( QUnknownInterface * );
    bool ref();

private:
    QInterfaceList* children;
    QUnknownInterface* par;
    int refcount;
    QApplicationInterface *appInterface;
    const char* objname;
};

class Q_EXPORT QPlugInInterface : public QUnknownInterface
{
public:
    QPlugInInterface( const char* name = 0 );

    QString interfaceID() const;

    virtual QString name() const;
    virtual QString description() const;
    virtual QString author() const;
    virtual QString version() const;
};

class Q_EXPORT QApplicationInterface : public QPlugInInterface
{
public:
    QApplicationInterface( const char* name = 0 );
    QString interfaceID() const;

    QString name() const;

    QString workDirectory() const;
    QString command() const;
};

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QGuardedPtr<QObject>;
// MOC_SKIP_END
#endif

class Q_EXPORT QApplicationComponentInterface : public QUnknownInterface
{
public:
    QApplicationComponentInterface( QObject* c, QUnknownInterface *parent, const char* name = 0 );

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

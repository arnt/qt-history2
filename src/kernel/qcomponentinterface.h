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
    friend class QLibrary;
    friend class QApplicationInterface;
public:
    QUnknownInterface( QUnknownInterface *parent = 0, const char* name = 0 );
    virtual ~QUnknownInterface();

    virtual QString interfaceId() const;
    QString Id() const;

    virtual bool initialize();
    virtual bool cleanup();

    virtual bool hasInterface( const QString&, bool recursive = TRUE, bool regexp = TRUE ) const;
    virtual QUnknownInterface* queryInterface( const QString&, bool recursive = TRUE, bool regexp = TRUE );
    virtual QStringList interfaceList( bool recursive = TRUE ) const;

    virtual bool addRef();
    virtual bool release();

    QApplicationInterface *applicationInterface() const;

    QUnknownInterface *parent() const;

    const char *name() const;   

protected:
    virtual void insertChild( QUnknownInterface * );
    virtual void removeChild( QUnknownInterface * );
    QUnknownInterface *child( const QString & ) const;
    QString createId( const QString& parent, const QString& that ) const;

private:
    QInterfaceList* children;
    QUnknownInterface* par;
    int refcount;
    QApplicationInterface *appInterface;
    const char* objname;
};

class Q_EXPORT QComponentInterface : public QUnknownInterface
{
public:
    QComponentInterface( const char* name = 0 );

    QString interfaceId() const;

    virtual QString brief() const;
    virtual QString description() const;
    virtual QString author() const;
    virtual QString version() const;
};

class Q_EXPORT QApplicationInterface : public QComponentInterface
{
public:
    QApplicationInterface( const char* name = 0 );
    QString interfaceId() const;

    QString brief() const;

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

    QString interfaceId() const;

#ifndef QT_NO_PROPERTIES
    virtual QVariant requestProperty( const QCString& p );
    virtual bool requestSetProperty( const QCString& p, const QVariant& v );
#endif
    virtual bool requestConnect( const char* signal, QObject* target, const char* slot );
    virtual bool requestConnect( QObject *sender, const char* signal, const char* slot );
    virtual bool requestEvents( QObject* o );

protected:
    QObject* component() const { return comp; }
    void setComponent( QObject* c ) { comp = c; }

private:
    QGuardedPtr<QObject> comp;
};

#ifndef Q_EXPORT_INTERFACE
    #ifdef Q_WS_WIN
	#define Q_EXPORT_INTERFACE( IMPLEMENTATION ) \
	    extern "C" __declspec(dllexport) QComponentInterface *qt_load_interface() { return new IMPLEMENTATION; }
    #else
	#define Q_EXPORT_INTERFACE( IMPLEMENTATION ) \
	    extern "C" QComponentInterface *qt_load_interface() { return new IMPLEMENTATION; }
    #endif
#endif

#endif

#endif //QCOMPONENTINTERFACE_H

#ifndef QCOMPONENTINTERFACE_H
#define QCOMPONENTINTERFACE_H

#ifndef QT_H
#include "qvariant.h"
#include "qobject.h"
#include "qguardedptr.h"
#endif // QT_H

#ifndef QT_NO_PLUGIN

class QInterfaceList;
class QRegExp;

class Q_EXPORT QUnknownInterface
{
public:
    QUnknownInterface( QUnknownInterface *parent = 0 );
    virtual ~QUnknownInterface();

    virtual QString interfaceId() const;

    virtual QUnknownInterface* queryInterface( const QString& );
    virtual unsigned long addRef();
    virtual unsigned long release();

    QUnknownInterface *parent() const;

protected:
    QString createId( const QString& parent, const QString& that ) const;

private:
    void insertChild( QUnknownInterface * );
    void removeChild( QUnknownInterface * );

    QInterfaceList* children;
    QUnknownInterface* par;
    unsigned long refcount;
};

class Q_EXPORT QComponentInterface : public QUnknownInterface
{
public:
    QString interfaceId() const;

    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual QString author() const = 0;
    virtual QString version() const = 0;
};

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QGuardedPtr<QObject>;
// MOC_SKIP_END
#endif

class Q_EXPORT QApplicationComponentInterface : public QUnknownInterface
{
public:
    QApplicationComponentInterface( QObject* c, QUnknownInterface *parent = 0);

    QString interfaceId() const;

#ifndef QT_NO_PROPERTIES
    virtual QVariant requestProperty( const QCString& p );
    virtual bool requestSetProperty( const QCString& p, const QVariant& v );
#endif
    virtual bool requestConnect( const char* signal, QObject* target, const char* slot );
    virtual bool requestConnect( QObject *sender, const char* signal, const char* slot );
    virtual bool requestEvents( QObject* o );

protected:
    virtual QObject* component() const { return comp; }
    void setComponent( QObject* c ) { comp = c; }

private:
    QGuardedPtr<QObject> comp;
};

#ifndef Q_CREATE_INSTANCE
    #define Q_CREATE_INSTANCE( IMPLEMENTATION )		\
	QUnknownInterface *i = new IMPLEMENTATION;	\
	i->addRef();					\
	return i;
#endif

#ifndef Q_EXPORT_INTERFACE
    #ifdef Q_WS_WIN
	#define Q_EXPORT_INTERFACE( IMPLEMENTATION ) \
	    extern "C" __declspec(dllexport) QUnknownInterface *qt_load_interface() { Q_CREATE_INSTANCE( IMPLEMENTATION ) }
    #else
	#define Q_EXPORT_INTERFACE( IMPLEMENTATION ) \
	    extern "C" QUnknownInterface *qt_load_interface() { Q_CREATE_INSTANCE( IMPLEMENTATION ) }
    #endif
#endif

#endif

#endif //QCOMPONENTINTERFACE_H

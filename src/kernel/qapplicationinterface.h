#ifndef QAPPLICATIONINTERFACES_H
#define QAPPLICATIONINTERFACES_H

#ifndef QT_H
#include "qvariant.h"
#include "qobject.h"
#endif // QT_H

#ifndef QT_NO_PLUGIN

class QApplicationInterface;

class Q_EXPORT QUnknownInterface
{
public:
    QUnknownInterface() {}
    virtual ~QUnknownInterface() {}

    static QString interfaceID() { return "QUnknownInterface"; }

    virtual bool connectNotify( QApplicationInterface* ) { return TRUE; }
    virtual bool disconnectNotify() { return TRUE; }

    virtual QUnknownInterface* queryInterface( const QString& ) { return 0; }
    virtual QStringList interfaceList() { return QStringList(); }
};

class Q_EXPORT QPlugInInterface : public QUnknownInterface
{
public:
    QPlugInInterface() {}

    static QString interfaceID() { return "QPlugInInterface"; }

    virtual QString name() { return QString::null; }
    virtual QString description() { return QString::null; }
    virtual QString author() { return QString::null; }

    virtual QUnknownInterface* queryInterface( const QString& ) = 0;
    virtual QStringList interfaceList() = 0;
};

class Q_EXPORT QApplicationInterface : public QObject, public QUnknownInterface
{
    Q_OBJECT

public:
    QApplicationInterface();
    ~QApplicationInterface() {}

    static QString interfaceID() { return "QApplicationInterface"; }
};

class Q_EXPORT QApplicationComponentInterface : public QObject, public QUnknownInterface
{
    Q_OBJECT

public:
    QApplicationComponentInterface( QObject* o );
    ~QApplicationComponentInterface() {}

    static QString interfaceID() { return "QApplicationComponentInterface"; }

#ifndef QT_NO_PROPERTIES
    virtual QVariant requestProperty( const QCString& p );
    virtual bool requestSetProperty( const QCString& p, const QVariant& v );
#endif
    virtual bool requestConnect( const char* signal, QObject* target, const char* slot );
    virtual bool requestConnect( QObject *sender, const char* signal, const char* slot );
    virtual bool requestEvents( QObject* o );

protected:
    QObject* object() { return QObject::parent(); }

private:
    QObject* parent() { return QObject::parent(); }
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

#endif //QAPPLICATIONINTERFACES_H

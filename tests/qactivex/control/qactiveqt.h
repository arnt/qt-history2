#ifndef QACTIVEQT_H
#define QACTIVEQT_H

#include <qwidget.h>
#include <quuid.h>
#include <private/qwidgetinterface_p.h>

// {22B230F6-8722-4051-ADCB-E7C9CE872EB3}
#ifndef IID_QActiveQtFactory
#define IID_QActiveQtFactory QUuid( 0x22b230f6, 0x8722, 0x4051, 0xad, 0xcb, 0xe7, 0xc9, 0xce, 0x87, 0x2e, 0xb3 )
#endif

struct QActiveQtFactoryInterface : public QFeatureListInterface
{
public:
    virtual QWidget *create( const QString &key, QWidget *parent = 0, const char *name = 0 ) = 0;
    virtual QMetaObject *metaObject( const QString &key ) const = 0;

    virtual QUuid appID() const = 0;
    virtual QUuid typeLibID() const = 0;
    virtual QUuid interfaceID( const QString &key ) const = 0;
    virtual QUuid eventsID( const QString &key ) const = 0;    
};

#ifndef NOQT_ACTIVEX
#ifndef __IID_DEFINED__
#define __IID_DEFINED__
typedef GUID IID;
#endif

#define __IID_DEFINED__

#if defined QT_ACTIVEXIMPL
#define QT_ACTIVEX( Class, IIDClass, IIDInterface, IIDEvents, IIDTypeLib, IIDApp ) \
    class QActiveQtFactory : public QActiveQtFactoryInterface \
    { \
    public: \
	QActiveQtFactory() {} \
	Q_REFCOUNT \
	QRESULT queryInterface( const QUuid &iid, QUnknownInterface **iface ) \
	{ \
	    *iface = 0; \
	    if ( iid == IID_QUnknown ) \
		*iface = this; \
	    else if ( iid == IID_QFeatureList ) \
		*iface = this; \
	    else if ( iid == IID_QActiveQtFactory ) \
		*iface = this; \
	    else \
		return QE_NOINTERFACE; \
	    addRef(); \
	    return QS_OK; \
	} \
	QStringList featureList() const \
	{ \
	    QStringList list; \
	    list << IIDClass; \
	    return list; \
	} \
	QWidget *create( const QString &key, QWidget *parent, const char *name ) \
	{ \
	    if ( QUuid(key) == QUuid(IIDClass) ) \
		return new Class( parent, name ); \
	    return 0; \
	} \
	QMetaObject *metaObject( const QString &key ) const \
	{ \
	    if ( QUuid(key) == QUuid(IIDClass) ) \
		return Class::staticMetaObject(); \
	    return 0; \
	} \
	QUuid appID() const \
	{ \
	    return QUuid( IIDApp ); \
	} \
	QUuid typeLibID() const \
	{ \
	    return QUuid( IIDTypeLib ); \
	} \
	QUuid interfaceID( const QString &key ) const \
	{ \
	    if ( QUuid(key) == QUuid(IIDClass) ) \
		return QUuid( IIDInterface ); \
	    return QUuid(); \
	} \
	QUuid eventsID( const QString &key ) const \
	{ \
	    if ( QUuid(key) == QUuid(IIDClass) ) \
		return QUuid( IIDEvents ); \
	    return QUuid(); \
	} \
    }; \
    Q_EXPORT_COMPONENT() \
    { \
	Q_CREATE_INSTANCE( QActiveQtFactory ) \
    } \

#else
#define QT_ACTIVEX( Class, IIDClass, IIDInterface, IIDEvents, IIDTypeLib, IIDApp )
#endif
#endif

class QActiveQtBase;

class QActiveQt
{
    friend class QActiveQtBase;
public:
    QActiveQt();

protected:
    bool requestPropertyChange( const char *property );
    void propertyChanged( const char *property );

private:
    QActiveQtBase *activex;
};

#endif // QACTIVEQT_H

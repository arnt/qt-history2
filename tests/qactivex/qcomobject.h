#ifndef QCOMOBJECT_H
#define QCOMOBJECT_H

#include <qobject.h>
#include <qvariant.h>

struct IUnknown;
class QAxEventSink;
struct QUuid;

#if defined(QT_PLUGIN)
#define QCOM_EXPORT __declspec(dllexport)
#else
#define QCOM_EXPORT __declspec(dllimport)
#endif

class QCOM_EXPORT QComBase
{
public:
    QComBase( IUnknown *iface = 0 );
    virtual ~QComBase();

    QString control() const;

    long queryInterface( const QUuid &, void** ) const;
    IUnknown *iface() const { return ptr; }

    QVariant dynamicCall( const QCString&, const QVariant &v1 = QVariant(), 
					   const QVariant &v2 = QVariant(),
					   const QVariant &v3 = QVariant(),
					   const QVariant &v4 = QVariant(),
					   const QVariant &v5 = QVariant(),
					   const QVariant &v6 = QVariant(),
					   const QVariant &v7 = QVariant(),
					   const QVariant &v8 = QVariant() );

    virtual QMetaObject *metaObject() const;
    virtual bool qt_invoke( int, QUObject* );
    virtual bool qt_property( int, int, QVariant* );
    virtual bool qt_emit( int, QUObject* ) = 0;
    virtual const char *className() const = 0;

    bool isNull() const { return !ptr; }

public:
    virtual void clear();
    void setControl( const QString& );

protected:
    QMetaObject *metaobj;

private:
    virtual bool initialize( IUnknown** ptr ) = 0;
    virtual QMetaObject *parentMetaObject() const = 0;

    IUnknown *ptr;
    QAxEventSink *eventSink;
    QString ctrl;
};

class QCOM_EXPORT QComObject : public QObject, public QComBase
{
public:
    QMetaObject *metaObject() const;
    const char *className() const;
    void* qt_cast( const char* );
    bool qt_invoke( int, QUObject* );
    bool qt_emit( int, QUObject* );
    bool qt_property( int, int, QVariant* );
    QObject* qObject() { return (QObject*)this; }

    QComObject( QObject *parent = 0, const char *name = 0 );
    QComObject( const QString &c, QObject *parent = 0, const char *name = 0 );
    QComObject( IUnknown *iface, QObject *parent = 0, const char *name = 0 );
    ~QComObject();

private:
    bool initialize( IUnknown** );
    QMetaObject *parentMetaObject() const;
};

#endif //QCOMOBJECT_H

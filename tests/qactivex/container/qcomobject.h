/****************************************************************************
** $Id: $
**
** Declaration of the QComBase and QComObject classes
**
** Copyright (C) 2001-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Active Qt integration.
**
** Licensees holding valid Qt Enterprise Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef UNICODE
#define UNICODE
#endif

#ifndef QCOMOBJECT_H
#define QCOMOBJECT_H

#include <qobject.h>
#include <qvariant.h>

struct IUnknown;
class QAxEventSink;
struct QUuid;

#if defined(QT_DLL)
#if defined(QT_PLUGIN)
#define QCOM_EXPORT __declspec(dllexport)
#else
#define QCOM_EXPORT __declspec(dllimport)
#endif
#else
#define QCOM_EXPORT
#endif

class QCOM_EXPORT QComBase
{
#ifdef Q_QDOC
#error "The Symbol Q_QDOC is reserved for documentation purposes."
    Q_PROPERTY( QString control READ control WRITE setControl )
#endif
public:
    typedef QMap<QCString, QVariant> PropertyBag;

    QComBase( IUnknown *iface = 0 );
    virtual ~QComBase();

    QString control() const;

    long queryInterface( const QUuid &, void** ) const;

    QVariant dynamicCall( const QCString&, const QVariant &v1 = QVariant(), 
					   const QVariant &v2 = QVariant(),
					   const QVariant &v3 = QVariant(),
					   const QVariant &v4 = QVariant(),
					   const QVariant &v5 = QVariant(),
					   const QVariant &v6 = QVariant(),
					   const QVariant &v7 = QVariant(),
					   const QVariant &v8 = QVariant() );
    QVariant dynamicCall( int ID, const QVariant &v1 = QVariant(), 
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
    virtual QObject *qObject() = 0;

    PropertyBag propertyBag() const;
    void setPropertyBag( const PropertyBag& );

    virtual bool propertyWritable( const char* ) const;
    virtual void setPropertyWritable( const char*, bool );

    bool isNull() const { return !ptr; }

#ifdef Q_QDOC
#error "The Symbol Q_QDOC is reserved for documentation purposes."
signals:
    void signal(const QString&,int,void*);
    void propertyChanged(const QString&);
#endif

public:
    virtual void clear();
    bool setControl( const QString& );

protected:
    QMetaObject *metaobj;
    virtual bool initialize( IUnknown** ptr ) = 0;

private:
    static QMetaObject *staticMetaObject() { return 0; }
    virtual QMetaObject *parentMetaObject() const = 0;

    IUnknown *ptr;
    QAxEventSink *eventSink;
    QString ctrl;
    QMap<QCString, bool> *propWritable;
};

#ifndef QT_NO_DATASTREAM
inline QDataStream &operator >>( QDataStream &s, QComBase &c )
{
    QComBase::PropertyBag bag;
    c.qObject()->blockSignals( TRUE );
    QString control;
    s >> control;
    c.setControl( control );
    s >> bag;
    c.setPropertyBag( bag );
    c.qObject()->blockSignals( FALSE );

    return s;
}

inline QDataStream &operator <<( QDataStream &s, const QComBase &c )
{
    QComBase::PropertyBag bag = c.propertyBag();
    s << c.control();
    s << bag;

    return s;
}
#endif

class QCOM_EXPORT QComObject : public QObject, public QComBase
{
    friend class QAxEventSink;
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

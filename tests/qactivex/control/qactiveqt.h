/****************************************************************************
** $Id: $
**
** Declaration of the QActiveQt and QActiveQtFactory classes
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

#ifndef QACTIVEQT_H
#define QACTIVEQT_H

#include <qwidget.h>
#include <quuid.h>
#include <private/qcom_p.h>

// {22B230F6-8722-4051-ADCB-E7C9CE872EB3}
#ifndef IID_QActiveQtFactory
#define IID_QActiveQtFactory QUuid( 0x22b230f6, 0x8722, 0x4051, 0xad, 0xcb, 0xe7, 0xc9, 0xce, 0x87, 0x2e, 0xb3 )
#endif

struct QActiveQtFactoryInterface : public QFeatureListInterface
{
public:
    virtual QWidget *create( const QString &key, QWidget *parent = 0, const char *name = 0 ) = 0;
    virtual QMetaObject *metaObject( const QString &key ) const = 0;

    virtual QUuid interfaceID( const QString &key ) const = 0;
    virtual QUuid eventsID( const QString &key ) const = 0;    
    virtual QUuid typeLibID() const = 0;
    virtual QUuid appID() const = 0;
};

class QActiveQtFactory : public QActiveQtFactoryInterface
{
public:
    QActiveQtFactory( const QUuid &, const QUuid &);
    Q_REFCOUNT

    QRESULT queryInterface( const QUuid &iid, QUnknownInterface **iface );

    QUuid typeLibID() const;
    QUuid appID() const;

private:
    QUuid typelib;
    QUuid app;
};

#define Q_EXPORT_ACTIVEX( IMPL, TYPELIB, APPID )	\
    Q_EXPORT_COMPONENT()				\
    {							\
	IMPL *impl = new IMPL( QUuid(TYPELIB), QUuid(APPID) );	\
	QUnknownInterface* iface = 0; 			\
	impl->queryInterface( IID_QUnknown, &iface );	\
	return iface;					\
    }

#ifndef NOQT_ACTIVEX

#ifndef __IID_DEFINED__
#define __IID_DEFINED__
typedef GUID IID;
#endif

#define __IID_DEFINED__

#if defined QT_ACTIVEX_DEFAULT
#define QT_ACTIVEX( Class, IIDClass, IIDInterface, IIDEvents, IIDTypeLib, IIDApp ) \
    class QDefaultActiveQtFactory : public QActiveQtFactory \
    { \
    public: \
	QDefaultActiveQtFactory( const QUuid &app, const QUuid &lib) \
	: QActiveQtFactory( app, lib ) {} \
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
    Q_EXPORT_ACTIVEX( QDefaultActiveQtFactory, IIDTypeLib, IIDApp ) \

#elif defined QT_ACTIVEX_IMPL
#define QT_ACTIVEX( Class, IIDClass, IIDInterface, IIDEvents, IIDTypeLib, IIDApp ) \
    const IID CLSID_##Class = QUuid(IIDClass); \
    const IID IID_I##Class = QUuid(IIDInterface); \
    const IID IID_I##Class##Events = QUuid(IIDEvents); \
    const IID IID_##Class##Lib = QUuid(IIDTypeLib); \
    const IID IID_##Class##App = QUuid(IIDApp); \

#else
#define QT_ACTIVEX( Class, IIDClass, IIDInterface, IIDEvents, IIDTypeLib, IIDApp )

#endif

#endif //NOQT_ACTIVEX

class QActiveQtBase;

class QActiveQt
{
    friend class QActiveQtBase;
public:
    QActiveQt();

    virtual QRESULT queryInterface( const QUuid&, void** );
    long addRef();
    long release();

    static bool isServer();

protected:
    bool requestPropertyChange( const char *property );
    void propertyChanged( const char *property );

private:
    QActiveQtBase *activex;
};

#endif // QACTIVEQT_H

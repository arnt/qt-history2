/****************************************************************************
** $Id: $
**
** Declaration of the QAxFactory class
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

#ifndef QAXFACTORY_H
#define QAXFACTORY_H

#include <quuid.h>
#include <private/qcom_p.h>

// {22B230F6-8722-4051-ADCB-E7C9CE872EB3}
#ifndef IID_QAxFactory
#define IID_QAxFactory QUuid( 0x22b230f6, 0x8722, 0x4051, 0xad, 0xcb, 0xe7, 0xc9, 0xce, 0x87, 0x2e, 0xb3 )
#endif

class QWidget;
class QMetaObject;
class QSettings;

struct QAxFactoryInterface : public QFeatureListInterface
{
public:
#ifndef Q_QDOC
    virtual QWidget *create( const QString &key, QWidget *parent = 0, const char *name = 0 ) = 0;

    virtual QUuid classID( const QString &key ) const = 0;
    virtual QUuid interfaceID( const QString &key ) const = 0;
    virtual QUuid eventsID( const QString &key ) const = 0;    
    virtual QUuid typeLibID() const = 0;
    virtual QUuid appID() const = 0;

    virtual void registerClass( const QString &key, QSettings * ) const = 0;
    virtual void unregisterClass( const QString &key, QSettings * ) const = 0;

    virtual QString exposeToSuperClass( const QString &key ) const = 0;
    virtual bool stayTopLevel( const QString &key ) const = 0;
    virtual bool hasStockEvents( const QString &key ) const = 0;
    virtual bool isService() const = 0;
#endif
};

class QAxFactory : public QAxFactoryInterface
{
public:
    QAxFactory( const QUuid &, const QUuid &);
    virtual ~QAxFactory();
    Q_REFCOUNT;

    QRESULT queryInterface( const QUuid &iid, QUnknownInterface **iface );

#ifdef Q_QDOC
    virtual QStringList featureList() const = 0;

    virtual QWidget *create( const QString &key, QWidget *parent = 0, const char *name = 0 ) = 0;

    virtual QUuid classID( const QString &key ) const = 0;
    virtual QUuid interfaceID( const QString &key ) const = 0;
    virtual QUuid eventsID( const QString &key ) const = 0;
#endif

    QUuid typeLibID() const;
    QUuid appID() const;

    void registerClass( const QString &key, QSettings * ) const;
    void unregisterClass( const QString &key, QSettings * ) const;

    QString exposeToSuperClass( const QString &key ) const;
    bool stayTopLevel( const QString &key ) const;
    bool hasStockEvents( const QString &key ) const;
    bool isService() const;

    static bool isServer();

private:
    QUuid typelib;
    QUuid app;
};

#define QAXFACTORY_EXPORT( IMPL, TYPELIB, APPID )	\
    QUnknownInterface *ucm_instantiate()		\
    {							\
	IMPL *impl = new IMPL( QUuid(TYPELIB), QUuid(APPID) );	\
	QUnknownInterface* iface = 0; 			\
	impl->queryInterface( IID_QUnknown, &iface );	\
	return iface;					\
    }

#define QAXFACTORY_DEFAULT( Class, IIDClass, IIDInterface, IIDEvents, IIDTypeLib, IIDApp ) \
    class QAxDefaultFactory : public QAxFactory \
    { \
    public: \
	QAxDefaultFactory( const QUuid &app, const QUuid &lib) \
	: QAxFactory( app, lib ) {} \
	QStringList featureList() const \
	{ \
	    QStringList list; \
	    list << #Class; \
	    return list; \
	} \
	QWidget *create( const QString &key, QWidget *parent, const char *name ) \
	{ \
	    if ( key == #Class ) \
		return new Class( parent, name ); \
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
	QUuid classID( const QString &key ) const \
	{ \
	    if ( key == #Class ) \
		return QUuid( IIDClass ); \
	    return QUuid(); \
	} \
	QUuid interfaceID( const QString &key ) const \
	{ \
	    if ( key == #Class ) \
		return QUuid( IIDInterface ); \
	    return QUuid(); \
	} \
	QUuid eventsID( const QString &key ) const \
	{ \
	    if ( key == #Class ) \
		return QUuid( IIDEvents ); \
	    return QUuid(); \
	} \
    }; \
    QAXFACTORY_EXPORT( QAxDefaultFactory, IIDTypeLib, IIDApp ) \


#endif // QAXFACTORY_H

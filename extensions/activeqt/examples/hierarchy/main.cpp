/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for the ActiveQt integration.
** EDITIONS: UNKNOWN
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "objects.h"
#include <qaxfactory.h>

class ActiveQtFactory : public QAxFactory
{
public:
    ActiveQtFactory( const QUuid &lib, const QUuid &app )
	: QAxFactory( lib, app )
    {}
    QStringList featureList() const
    {
	QStringList list;
	list << "QParentWidget";
	list << "QSubWidget";
	return list;
    }
    QWidget *create( const QString &key, QWidget *parent, const char *name )
    {
	if ( key == "QParentWidget" )
	    return new QParentWidget( parent, name );

	return 0;
    }
    QUuid classID( const QString &key ) const
    {
	if ( key == "QParentWidget" )
	    return QUuid( "{d574a747-8016-46db-a07c-b2b4854ee75c}" );
	if ( key == "QSubWidget" )
	    return QUuid( "{850652f4-8f71-4f69-b745-bce241ccdc30}" );

	return QUuid();
    }
    QUuid interfaceID( const QString &key ) const
    {
	if ( key == "QParentWidget" )
	    return QUuid( "{4a30719d-d9c2-4659-9d16-67378209f822}" );
	if ( key == "QSubWidget" )
	    return QUuid( "{2d76cc2f-3488-417a-83d6-debff88b3c3f}" );

	return QUuid();
    }
    QUuid eventsID( const QString &key ) const
    {
	if ( key == "QParentWidget" )
	    return QUuid( "{aac9f855-c3dc-4cae-b747-c77f4d509f4c}" );
	if ( key == "QSubWidget" )
	    return QUuid( "{25fac47e-c723-4696-8c74-6185903bdf65}" );

	return QUuid();
    }

    QString exposeToSuperClass( const QString &key ) const
    {
	if ( key == "QSubWidget" )
	    return key;
	return QAxFactory::exposeToSuperClass(key);
    }
};

QAXFACTORY_EXPORT( ActiveQtFactory, "{9e626211-be62-4d18-9483-9419358fbb03}", "{75c276de-1df5-451f-a004-e4fa1a587df1}" )
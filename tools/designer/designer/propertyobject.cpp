/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "propertyobject.h"
#include "metadatabase.h"
#include <qvector.h>
#include <qlist.h>
#include <qmetaobject.h>
#include <qvariant.h>

PropertyObject::PropertyObject( const QWidgetList &objs )
    : QObject(), objects( objs ), mobj( 0 )
{
    QVector<QList<QMetaObject*> > v;

    for (int i = 0; i < objects.size(); ++i) {
	QObject *o = objects.at(i);
	const QMetaObject *m = o->metaObject();
	QList<QMetaObject*> mol;
	while ( m ) {
	    mol.insert( 0, (QMetaObject*)m );
	    m = m->superClass();
	}
	v.insert( v.count(), mol );
    }

    int numObjects = objects.count();
    int minDepth = v[0].count();
    int depth = minDepth;

    for ( int i = 0; i < numObjects; ++i ) {
	depth = (int)v[i].count();
	if ( depth < minDepth )
	    minDepth = depth;
    }

    const QMetaObject *m = v[0].at( --minDepth );

    for ( int j = 0; j < numObjects; ++j ) {
	if ( v[j].at( minDepth ) != m ) {
	    m = v[0].at( --minDepth );
	    j = 0;
	}
    }

    mobj = m;

    Q_ASSERT( mobj );
}

bool PropertyObject::setProperty( const char *name, const QVariant& value )
{
    for (int i = 0; i < objects.size(); ++i) {
	QObject *o = objects.at(i);
	o->setProperty( name, value );
    }

    return TRUE;
}

QVariant PropertyObject::property( const char *name ) const
{
    return ( (PropertyObject*)this )->objects.first()->property( name );
}

void PropertyObject::mdPropertyChanged( const QString &property, bool changed )
{
    for (int i = 0; i < objects.size(); ++i) {
	QObject *o = objects.at(i);
	MetaDataBase::setPropertyChanged( o, property, changed );
    }
}

bool PropertyObject::mdIsPropertyChanged( const QString &property )
{
    for (int i = 0; i < objects.size(); ++i) {
	QObject *o = objects.at(i);
	if ( MetaDataBase::isPropertyChanged( o, property ) )
	    return TRUE;
    }
    return FALSE;
}

void PropertyObject::mdSetPropertyComment( const QString &property, const QString &comment )
{
    for (int i = 0; i < objects.size(); ++i) {
	QObject *o = objects.at(i);
	MetaDataBase::setPropertyComment( o, property, comment );
    }
}

QString PropertyObject::mdPropertyComment( const QString &property )
{
    return MetaDataBase::propertyComment( objects.first(), property );
}

void PropertyObject::mdSetFakeProperty( const QString &property, const QVariant &value )
{
    for (int i = 0; i < objects.size(); ++i) {
	QObject *o = objects.at(i);
	MetaDataBase::setFakeProperty( o, property, value );
    }
}

QVariant PropertyObject::mdFakeProperty( const QString &property )
{
    return MetaDataBase::fakeProperty( objects.first(), property );
}

void PropertyObject::mdSetCursor( const QCursor &c )
{
    for (int i = 0; i < objects.size(); ++i) {
	QObject *o = objects.at(i);
	if ( o->isWidgetType() )
	    MetaDataBase::setCursor( (QWidget*)o, c );
    }
}

QCursor PropertyObject::mdCursor()
{
    return MetaDataBase::cursor( objects.first() );
}

void PropertyObject::mdSetPixmapKey( int pixmap, const QString &arg )
{
    for (int i = 0; i < objects.size(); ++i) {
	QObject *o = objects.at(i);
	MetaDataBase::setPixmapKey( o, pixmap, arg );
    }
}

QString PropertyObject::mdPixmapKey( int pixmap )
{
    return MetaDataBase::pixmapKey( objects.first(), pixmap );
}

void PropertyObject::mdSetExportMacro( const QString &macro )
{
    for (int i = 0; i < objects.size(); ++i) {
	QObject *o = objects.at(i);
	MetaDataBase::setExportMacro( o, macro );
    }
}

QString PropertyObject::mdExportMacro()
{
    return MetaDataBase::exportMacro( objects.first() );
}

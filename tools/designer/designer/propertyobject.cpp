#include "propertyobject.h"
#include <qvector.h>
#include <qptrlist.h>
#include <qmetaobject.h>
#include <qvariant.h>
#include "metadatabase.h"

PropertyObject::PropertyObject( const QWidgetList &objs )
    : QObject(), objects( objs ), mobj( 0 )
{
    QVector<QPtrList<QMetaObject> > v;
    v.resize( objects.count() );
    v.setAutoDelete( TRUE );

    for ( QObject *o = objects.first(); o; o = objects.next() ) {
	const QMetaObject *m = o->metaObject();
	QPtrList<QMetaObject> *mol = new QPtrList<QMetaObject>;
	while ( m ) {
	    mol->insert( 0, m );
	    m = m->superClass();
	}
	v.insert( v.count(), mol );
    }

    int i = 0;
    int numObjects = objects.count();
    while ( TRUE ) {
	bool same = FALSE;
	const QMetaObject *m = 0;
	for ( int j = 0; j < numObjects; ++j ) {
	    if ( i >= (int)v[j]->count() || j > 0 && v[j]->at( i ) != m ) {
		same = FALSE;
		break;
	    } else {
		m = v[j]->at( i );
		same = TRUE;
	    }
	}
	if ( same )
	    mobj = m;
	else
	    break;
	++i;
    }

    Q_ASSERT( mobj );
}

bool PropertyObject::setProperty( const char *name, const QVariant& value )
{
    for ( QObject *o = objects.first(); o; o = objects.next() )
	o->setProperty( name, value );

    return TRUE;
}

QVariant PropertyObject::property( const char *name ) const
{
    return ( (PropertyObject*)this )->objects.first()->property( name );
}

void PropertyObject::mdPropertyChanged( const QString &property, bool changed )
{
    for ( QObject *o = objects.first(); o; o = objects.next() )
	MetaDataBase::setPropertyChanged( o, property, changed );
}

bool PropertyObject::mdIsPropertyChanged( const QString &property )
{
    for ( QObject *o = objects.first(); o; o = objects.next() ) {
	if ( MetaDataBase::isPropertyChanged( o, property ) )
	    return TRUE;
    }
    return FALSE;
}

void PropertyObject::mdSetPropertyComment( const QString &property, const QString &comment )
{
    for ( QObject *o = objects.first(); o; o = objects.next() )
	MetaDataBase::setPropertyComment( o, property, comment );
}

QString PropertyObject::mdPropertyComment( const QString &property )
{
    return MetaDataBase::propertyComment( objects.first(), property );
}

QVariant PropertyObject::mdFakeProperty( const QString &property )
{
    return MetaDataBase::fakeProperty( objects.first(), property );
}

void PropertyObject::mdSetCursor( const QCursor &c )
{
    for ( QObject *o = objects.first(); o; o = objects.next() ) {
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
    for ( QObject *o = objects.first(); o; o = objects.next() )
	MetaDataBase::setPixmapKey( o, pixmap, arg );
}

QString PropertyObject::mdPixmapKey( int pixmap )
{
    return MetaDataBase::pixmapKey( objects.first(), pixmap );
}

void PropertyObject::mdSetExportMacro( const QString &macro )
{
    for ( QObject *o = objects.first(); o; o = objects.next() )
	MetaDataBase::setExportMacro( o, macro );
}

QString PropertyObject::mdExportMacro()
{
    return MetaDataBase::exportMacro( objects.first() );
}

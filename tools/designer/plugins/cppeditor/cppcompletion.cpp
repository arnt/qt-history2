/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "cppcompletion.h"
#include <qobject.h>
#include <qmetaobject.h>
#include <qregexp.h>

CppEditorCompletion::CppEditorCompletion( Editor *e )
    : EditorCompletion( e )
{
}

bool CppEditorCompletion::doObjectCompletion( const QString &objName )
{
    if ( !ths )
	return FALSE;
    QString object( objName );
    int i = -1;
    if ( ( i = object.findRev( "->" ) ) != -1 )
	object = object.mid( i + 2 );
    if ( ( i = object.findRev( "." ) ) != -1 )
	object = object.mid( i + 1 );
    object = object.simplifyWhiteSpace();
    QObject *obj = 0;
    if ( ths->name() == object || object == "this" ) {
	obj = ths;
    } else {
	obj = ths->child( object );
    }

    if ( !obj )
	return FALSE;

    QList<CompletionEntry> lst;

    QObjectList ol = obj->children();
    for (int i = 0; i < ol.size(); ++i) {
	QString s( ol.at(i)->name() );
	if ( s.find( " " ) == -1 && s.find( "qt_" ) == -1 &&
	     s.find( "unnamed" ) == -1 ) {
	    CompletionEntry c;
	    c.type = "variable";
	    c.text = s;
	    c.prefix = "";
	    lst << c;
	}
    }

    int propertyCount = obj->metaObject()->propertyCount();
    for (int i = 0; i < propertyCount; ++i) {
	QMetaProperty p = obj->metaObject()->property(i);
	QString f( p.name() );
	QChar c = f[ 0 ];
	f.remove( (uint)0, 1 );
	f.prepend( c.upper() );
	f.prepend( "set" );

	CompletionEntry ce;
	ce.type = "property";
	ce.text = f;
	ce.postfix = "()";

	if ( lst.find( ce ) == lst.end() )
	    lst << ce;
    }

    int slotCount = obj->metaObject()->slotCount();
    for (int i = 0; i < slotCount; ++i) {
	QMetaMember mm = obj->metaObject()->slot(i);
	QString f( mm.signature() );
	f = f.left( f.find( "(" ) );
	CompletionEntry c;
	c.type = "slot";
	c.text = f;
	c.postfix = "()";
	if ( lst.find( c ) == lst.end() )
	    lst << c;
    }

    if ( lst.isEmpty() )
	return FALSE;

    showCompletion( lst );
    return TRUE;
}

QList<QStringList> CppEditorCompletion::functionParameters( const QString &expr, QChar &separator,
						     QString &prefix, QString &postfix )
{
    Q_UNUSED( prefix );
    Q_UNUSED( postfix );
    separator = ',';
    if ( !ths )
	return QList<QStringList>();
    QString func;
    QString objName;
    int i = -1;

    i = expr.findRev( "->" );
    if ( i == -1 )
	i = expr.findRev( "." );
    else
	++i;
    if ( i == -1 ) {
	i = expr.findRev( " " );

	if ( i == -1 )
	    i = expr.findRev( "\t" );
	else
	    objName = ths->name();

	if ( i == -1 && expr[ 0 ] != ' ' && expr[ 0 ] != '\t' )
	    objName = ths->name();
    }

    if ( !objName.isEmpty() ) {
	func = expr.mid( i + 1 );
	func = func.simplifyWhiteSpace();
    } else {
	func = expr.mid( i + 1 );
	func = func.simplifyWhiteSpace();
	QString ex( expr );
	ex.remove( i, 0xFFFFFF );
	if ( ex[ (int)ex.length() - 1 ] == '-' )
	    ex.remove( ex.length() - 1, 1 );
	int j = -1;
	j = ex.findRev( "->" );
	if ( j == -1 )
	    j = ex.findRev( "." );
	else
	    ++j;
	if ( j == -1 ) {
	    j = ex.findRev( " " );

	    if ( j == -1 )
		j = ex.findRev( "\t" );
	    else
		objName = ths->name();

	    if ( j == -1 )
		objName = ths->name();
	}
	objName = ex.mid( j + 1 );
	objName = objName.simplifyWhiteSpace();
    }

    QObject *obj = 0;
    if ( ths->name() == objName || objName == "this" ) {
	obj = ths;
    } else {
	obj = ths->child( objName );
    }

    if ( !obj )
	return QList<QStringList>();

    int slotCount = obj->metaObject()->slotCount();
    for (int i = 0; i < slotCount; ++i) {
	QMetaMember mm = obj->metaObject()->slot(i);
	QString f( mm.signature() );
	f = f.left( f.find( "(" ) );
	if ( f == func ) {
	    f = QString( mm.signature() );
	    f.remove( (uint)0, f.find( "(" ) + 1 );
	    f = f.left( f.find( ")" ) );
	    QStringList lst = QStringList::split( ',', f );
	    if ( !lst.isEmpty() ) {
		QList<QStringList> l;
		l << lst;
		return l;
	    }
	}
    }

    QMetaProperty prop =
	obj->metaObject()->
	property( obj->metaObject()->indexOfProperty( func[ 3 ].lower() + func.mid( 4 ) ) );
    if ( prop ) {
	QList<QStringList> l;
	l << QStringList( prop.type() );
	return l;
    }

    return QList<QStringList>();
}

void CppEditorCompletion::setContext( QObject *this_ )
{
    ths = this_;
}

#include "cppcompletion.h"
#include <qobject.h>
#include <qmetaobject.h>
#include <qobjectlist.h>
#include <qregexp.h>

CppEditorCompletion::CppEditorCompletion( Editor *e )
    : EditorCompletion( e )
{
}

bool CppEditorCompletion::doObjectCompletion( const QString &object )
{
    if ( !ths )
	return FALSE;
    QObject *obj = 0;
    if ( ths->name() == object ) {
	obj = ths;
    } else {
	obj = ths->child( object );
    }

    if ( !obj )
	return FALSE;

    QStringList lst;
    QStrList slts = obj->metaObject()->slotNames( TRUE );
    for ( QListIterator<char> sit( slts ); sit.current(); ++sit ) {
	QString f( sit.current() );
	f = f.left( f.find( "(" ) ) + "()";
	if ( lst.find( f ) == lst.end() )
	    lst << f;
    }
    QStrList props = obj->metaObject()->propertyNames( TRUE );
    for ( QListIterator<char> pit( props ); pit.current(); ++pit ) {
	QString f( pit.current() );
	QChar c = f[ 0 ];
	f.remove( 0, 1 );
	f.prepend( c.upper() );
	f.prepend( "set" );
	f += "()";
	if ( lst.find( f ) == lst.end() )
	    lst << f;
    }

    for ( QObjectListIt cit( *obj->children() ); cit.current(); ++cit )
	lst << cit.current()->name();
    lst = lst.grep( QRegExp( "[^unnamed]" ) ); // filter out unnamed objects
    if ( lst.isEmpty() )
	return FALSE;

    showCompletion( lst );
    return TRUE;
}

QStringList CppEditorCompletion::functionParameters( const QString &expr, QChar &separator )
{
    separator = ',';
    if ( !ths )
	return QStringList();
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

    qDebug( objName );

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
    if ( ths->name() == objName ) {
	obj = ths;
    } else {
	obj = ths->child( objName );
    }

    if ( !obj )
	return QStringList();

    QStrList slts = obj->metaObject()->slotNames( TRUE );
    for ( QListIterator<char> sit( slts ); sit.current(); ++sit ) {
	QString f( sit.current() );
	f = f.left( f.find( "(" ) );
	if ( f == func ) {
	    f = QString( sit.current() );
	    f.remove( 0, f.find( "(" ) + 1 );
	    f = f.left( f.find( ")" ) );
	    QStringList lst = QStringList::split( ',', f );
	    if ( !lst.isEmpty() )
		return lst;
	}
    }

    return QStringList();
}

void CppEditorCompletion::setContext( QObjectList *, QObject *this_ )
{
    ths = this_;
}

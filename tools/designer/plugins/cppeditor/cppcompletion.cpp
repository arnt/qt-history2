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
    QObject *obj;
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
	if ( lst.find( f ) != lst.end() )
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
	if ( lst.find( f ) != lst.end() )
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

QStringList CppEditorCompletion::functionParameters( const QString &, QChar & )
{
    return QStringList();
}

void CppEditorCompletion::setContext( QObjectList *, QObject *this_ )
{
    ths = this_;
}

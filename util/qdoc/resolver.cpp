/*
  resolver.cpp
*/

#include "resolver.h"

static QString *opParenParen = 0;
static QString *aHrefEq = 0;
static QString *rAngle = 0;
static QString *slashA = 0;

Resolver::Resolver()
{
    if ( opParenParen == 0 ) {
	opParenParen = new QString( "operator()" );
	aHrefEq = new QString( "<a href=\"" );
	rAngle = new QString( "\">" );
	slashA = new QString( "</a>" );
    }
}

QString Resolver::resolve( const QString& /* name */ ) const
{
    return QString::null;
}

/*
  By default, functions are resolved as anything else.
*/
QString Resolver::resolvefn( const QString& name ) const
{
    return resolve( name );
}

bool Resolver::changedSinceLastRun( const Location& /* loc */,
				    const QString& /* link */,
				    const QString& /* html */ ) const
{
    return FALSE;
}

QString Resolver::href( const QString& name, const QString& text ) const
{
    int k = name.find( QChar('(') );
    if ( k == -1 || (name.length() >= 10 &&
		     name[ (int)name.length() - 3] == QChar('r') &&
		     name.right(10) == *opParenParen) )
	k = name.length();

    QString left;
    QString right;
    QString link;

    if ( text.isEmpty() ) {
	left = name.left( k );
	right = name.mid( k );
    } else {
	left = text;
    }

    if ( k < (int)name.length() )
	link = resolvefn( name.left(k) );
    else
	link = resolve( name );

    if ( link.isEmpty() )
	return left + right;
    else
	return *aHrefEq + link + *rAngle + left + *slashA + right;
}

/*
  resolver.cpp
*/

#include "resolver.h"

QString Resolver::resolve( const QString& /* name */ ) const
{
    return QString::null;
}

QString Resolver::resolvefn( const QString& name ) const
{
    return resolve( name );
}

void Resolver::compare( const Location& /* loc */, const QString& /* link */,
			const QString& /* html */ ) const
{
}

QString Resolver::href( const QString& name, const QString& text ) const
{
    // ### useful?
    static QString *opParenParen = 0;
    static QString *aHrefEq = 0;
    static QString *rAngle = 0;
    static QString *slashA = 0;

    if ( opParenParen == 0 ) {
	opParenParen = new QString( "operator()" );
	aHrefEq = new QString( "<a href=" );
	rAngle = new QString( ">" );
	slashA = new QString( "</a>" );
    }

    int k = name.find( QChar('(') );
    if ( k == -1 || name.right(10) == *opParenParen )
	k = name.length();

    QString left = text;
    QString right;
    QString link;

    if ( left.isEmpty() ) {
	left = name.left( k );
	right = name.mid( k );
    }
    link = resolve( name.left(k) );

    if ( link.isEmpty() )
	return left + right;
    else
	return *aHrefEq + link + *rAngle + left + *slashA + right;
}

/*
  resolver.cpp
*/

#include "resolver.h"

QString Resolver::resolve( const QString& /* name */ ) const
{
    return QString::null;
}

bool Resolver::changedSinceLastRun( const QString& /* link */,
				    const QString& /* html */ ) const
{
    return FALSE;
}

bool Resolver::warnChangedSinceLastRun( const Location& /* loc */,
					const QString& /* link */,
					const QString& /* html */ ) const
{
    return FALSE;
}

QString Resolver::href( const QString& name, const QString& text ) const
{
    static const QString opParenParen( "operator()" );
    static const QString aHrefEq( "<a href=\"" );
    static const QString rAngle( "\">" );
    static const QString slashA( "</a>" );

    int k = name.find( QChar('(') );
    if ( k == -1 || (name.length() >= 10 &&
		     name[(int) name.length() - 3] == QChar('r') &&
		     name.right(10) == *opParenParen) )
	k = name.length();

    QString left, right, link;

    if ( text.isEmpty() ) {
	left = name.left( k );
	right = name.mid( k );
    } else {
	left = text;
    }

    link = resolve( name );
    if ( link.isEmpty() )
	return left + right;
    else
	return aHrefEq + link + rAngle + left + slashA + right;
}

/*
  english.cpp
*/

#include "config.h"
#include "english.h"

/*
  Returns a stack of separators in an English enumeration.
*/
QValueStack<QString> separators( int n, const QString& terminator )
{
    QValueStack<QString> seps;

    if ( n >= 1 )
	seps.push( terminator );

    if ( n >= 3 && config->serialComma() )
	seps.push( QString(", and ") );
    else if ( n >= 2 )
	seps.push( QString(" and ") );

    for ( int i = 0; i < n - 2; i++ )
	seps.push( QString(", ") );

    return seps;
}

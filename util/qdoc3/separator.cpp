/*
  separator.cpp
*/

#include "separator.h"
#include "tr.h"

QString separator( int index, int count )
{
    if ( index == count - 1 )
	return tr( ".", "terminator" );

    if ( count == 2 ) {
	return tr( " and ", "separator when N = 2" );
    } else {
	if ( index == 0 ) {
	    return tr( ", ", "first separator when N > 2" );
	} else if ( index < count - 2 ) {
	    return tr( ", ", "general separator when N > 2" );
	} else {
	    return tr( ", and ", "last separator when N > 2" );
	}
    }
}

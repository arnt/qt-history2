/*
  separator.cpp
*/

#include "qdoc.h"
#include "separator.h"

QString separator( int index, int count )
{
    if ( index == count - 1 )
	return Qdoc::tr( ".", "terminator" );

    if ( count == 2 ) {
	return Qdoc::tr( " and ", "separator when N = 2" );
    } else {
	if ( index == 0 ) {
	    return Qdoc::tr( ", ", "first separator when N > 2" );
	} else if ( index < count - 2 ) {
	    return Qdoc::tr( ", ", "general separator when N > 2" );
	} else {
	    return Qdoc::tr( ", and ", "last separator when N > 2" );
	}
    }
}

/*
  separator.cpp
*/

#include "separator.h"

QString separator( int index, int count )
{
    if ( index == count - 1 )
	return ".";

    if ( count == 2 ) {
	return " and ";
    } else {
	if ( index == 0 ) {
	    return ", ";
	} else if ( index < count - 2 ) {
	    return ", ";
	} else {
	    return " and ";
	}
    }
}

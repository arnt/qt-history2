/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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

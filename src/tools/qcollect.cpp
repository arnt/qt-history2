/****************************************************************************
** $Id: //depot/qt/main/src/tools/qcollect.cpp#1 $
**
** Implementation of base class for all collection classes
**
** Author  : Haavard Nord
** Created : 920820
**
** Copyright (C) 1992,1993 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#include "qcollect.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/tools/qcollect.cpp#1 $";
#endif


// Default implementation of virtual functions

GCI QCollection::newItem( GCI d )		// allocate object
{
    return d;					// just return reference
}

void QCollection::deleteItem( GCI d )		// delete object
{
    if ( del_item )
	delete d;				// default operation
}

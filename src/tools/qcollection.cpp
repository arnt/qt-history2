/****************************************************************************
** $Id: //depot/qt/main/src/tools/qcollection.cpp#3 $
**
** Implementation of base class for all collection classes
**
** Author  : Haavard Nord
** Created : 920820
**
** Copyright (C) 1992-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qcollect.h"

RCSTAG("$Id: //depot/qt/main/src/tools/qcollection.cpp#3 $")


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

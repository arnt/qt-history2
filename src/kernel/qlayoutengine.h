/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qlayoutengine.h#1 $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

/* WARNING! THIS IS NOT YET PART OF THE Qt API, AND MAY BE CHANGED WITHOUT
   NOTICE */


#ifndef QLAYOUTENGINE_H
#define QLAYOUTENGINE_H

#include "qabstractlayout.h"

struct QLayoutStruct
{
    void init() { stretch = 0; initParameters(); }
    void initParameters() { minimumSize = sizeHint = 0;
    maximumSize = QCOORD_MAX; expansive = FALSE; empty = TRUE; }
    //permanent storage:
    int stretch;
    //parameters:
    QCOORD sizeHint;
    QCOORD maximumSize;
    QCOORD minimumSize;
    bool expansive;
    bool empty;
    //temporary storage:
    bool done;
    //result:
    int pos;
    int size;
};


void qGeomCalc( QArray<QLayoutStruct> &chain, int count, int pos,
		      int space, int spacer );


#endif

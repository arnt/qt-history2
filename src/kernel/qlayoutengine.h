/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qlayoutengine.h#4 $
**
** Internal header file. 
**
** Created : 981027
**
** Copyright (C) 1998-99 by Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

/* WARNING! THIS IS NOT YET PART OF THE Qt API, AND MAY BE CHANGED WITHOUT
   NOTICE */


#ifndef QLAYOUTENGINE_H
#define QLAYOUTENGINE_H

#include "qabstractlayout.h"

struct QLayoutStruct
{
    void init() { stretch = 0; initParameters(); }
    void initParameters() { minimumSize = sizeHint = 0;
    maximumSize = QWIDGETSIZE_MAX; expansive = FALSE; empty = TRUE; }
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


void qGeomCalc( QArray<QLayoutStruct> &chain, int start, int count, int pos,
		      int space, int spacer );


#endif

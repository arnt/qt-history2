/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qlayoutengine_p.h#1 $
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QLAYOUTENGINE_P_H
#define QLAYOUTENGINE_P_H


//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qlayout.cpp, qlayoutengine.cpp, qmainwindow.cpp and qsplitter.cpp.
// This header file may change from version to version without notice,
// or even be removed.
//
//


#ifndef QT_H
#include "qabstractlayout.h"
#endif // QT_H
#ifndef QT_NO_LAYOUT
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



/*
  Modify total maximum (max) and total expansion (exp)
  when adding boxmax/boxexp.

  Expansive boxes win over non-expansive boxes.
*/
static inline void qMaxExpCalc( QCOORD & max, bool &exp,
			       QCOORD boxmax, bool boxexp )
{
    if ( exp ) {
	if ( boxexp )
	    max = QMAX( max, boxmax );
    } else {
	if ( boxexp )
	    max = boxmax;
	else
	    max = QMIN( max, boxmax );
    }
    exp = exp || boxexp;
}

#endif //QT_NO_LAYOUT
#endif

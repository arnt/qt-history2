/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QLAYOUTENGINE_P_H
#define QLAYOUTENGINE_P_H

#ifndef QLAYOUT_H
    #error "Need to include qlayout.h before including qlayoutengine_p.h"
#endif

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qlayout.cpp, qlayoutengine.cpp, qmainwindow.cpp and qsplitter.cpp.
// This header file may change from version to version without notice,
// or even be removed.
//
// We mean it.
//
//


#ifndef QT_H
#include "qabstractlayout.h"
#endif // QT_H

#ifndef QT_NO_LAYOUT

template <typename T> class QVector;

struct QLayoutStruct
{
    inline void init( int stretchFactor = 0, int spacing = 0 ) {
	stretch = stretchFactor;
	minimumSize = sizeHint = spacing;
	maximumSize = QLAYOUTSIZE_MAX;
	expansive = FALSE;
	empty = TRUE;
    }

    QCOORD smartSizeHint() {
	return ( stretch > 0 ) ? minimumSize : sizeHint;
    }

    // parameters
    int stretch;
    QCOORD sizeHint;
    QCOORD maximumSize;
    QCOORD minimumSize;
    bool expansive;
    bool empty;

    // temporary storage
    bool done;

    // result
    int pos;
    int size;
};


void qGeomCalc( QVector<QLayoutStruct> &chain, int start, int count,
			 int pos, int space, int spacer );
QSize qSmartMinSize( const QWidgetItem *i );
QSize qSmartMinSize( QWidget *w );
QSize qSmartMaxSize( const QWidgetItem *i, Qt::Alignment align = 0 );
QSize qSmartMaxSize( QWidget *w, Qt::Alignment align = 0 );


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
	    max = qMax( max, boxmax );
    } else {
	if ( boxexp )
	    max = boxmax;
	else
	    max = qMin( max, boxmax );
    }
    exp = exp || boxexp;
}

#endif //QT_NO_LAYOUT
#endif

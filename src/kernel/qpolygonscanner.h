/****************************************************************************
**
** Definition of QPolygonScanner class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPOLYGONSCANNER_H
#define QPOLYGONSCANNER_H

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H

class QPointArray;
class QPoint;

class Q_GUI_EXPORT QPolygonScanner {
public:
    // BIC: fix for 3.0
    void scan( const QPointArray& pa, bool winding, int index=0, int npoints=-1 );
    void scan( const QPointArray& pa, bool winding, int index, int npoints, bool stitchable );
    enum Edge { Left=1, Right=2, Top=4, Bottom=8 };
    void scan( const QPointArray& pa, bool winding, int index, int npoints, Edge edges );
    virtual void processSpans( int n, QPoint* point, int* width )=0;
};

#endif // QPOLYGONSCANNER_H

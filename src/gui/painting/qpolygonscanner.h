/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPOLYGONSCANNER_H
#define QPOLYGONSCANNER_H

#include "qglobal.h"

class QPointArray;
class QPoint;

class Q_GUI_EXPORT QPolygonScanner {
public:
    // BIC: fix for 3.0
    void scan(const QPointArray& pa, bool winding, int index=0, int npoints=-1);
    void scan(const QPointArray& pa, bool winding, int index, int npoints, bool stitchable);
    enum Edge { Left=1, Right=2, Top=4, Bottom=8 };
    void scan(const QPointArray& pa, bool winding, int index, int npoints, Edge edges);
    virtual void processSpans(int n, QPoint* point, int* width)=0;
};

#endif // QPOLYGONSCANNER_H

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

#ifndef Q3POLYGONSCANNER_H
#define Q3POLYGONSCANNER_H

#include "QtCore/qglobal.h"

QT_MODULE(Qt3SupportLight)

class Q3PointArray;
class QPoint;

class Q_COMPAT_EXPORT Q3PolygonScanner {
public:
    virtual ~Q3PolygonScanner() {}
    void scan(const Q3PointArray& pa, bool winding, int index=0, int npoints=-1);
    void scan(const Q3PointArray& pa, bool winding, int index, int npoints, bool stitchable);
    enum Edge { Left=1, Right=2, Top=4, Bottom=8 };
    void scan(const Q3PointArray& pa, bool winding, int index, int npoints, Edge edges);
    virtual void processSpans(int n, QPoint* point, int* width)=0;
};

#endif // Q3POLYGONSCANNER_H

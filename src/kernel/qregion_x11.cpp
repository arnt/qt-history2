/****************************************************************************
**
** Implementation of X11 region translation.
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

#include "qt_x11_p.h"

#include <limits.h>

QRegion::QRegionData QRegion::shared_empty = {Q_ATOMIC_INIT(1), 0, 0, 0};

/*
  This is how X represents regions internally.
*/

struct BOX {
    short x1, x2, y1, y2;
};

struct _XRegion {
    long size;
    long numRects;
    BOX *rects;
    BOX  extents;
};


void QRegion::updateX11Region() const
{
    d->rgn = XCreateRegion();
    for(int i = 0; i < d->qt_rgn->numRects; ++i) {
        XRectangle r;
        const QRect &rect = d->qt_rgn->rects[i];
        r.x = qMax(SHRT_MIN, rect.x());
        r.y = qMax(SHRT_MIN, rect.y());
        r.width = qMin(USHRT_MAX, rect.width());
        r.height = qMin(USHRT_MAX, rect.height());
        XUnionRectWithRegion(&r, d->rgn, d->rgn);
    }
}

void *QRegion::clipRectangles(int &num) const
{
    if (!d->xrectangles && !isNull()) {
        XRectangle *r = static_cast<XRectangle*>(malloc(d->qt_rgn->numRects * sizeof(XRectangle)));
        d->xrectangles = r;
        for(int i = 0; i < d->qt_rgn->numRects; ++i) {
            const QRect &rect = d->qt_rgn->rects[i];
            r->x = qMax(SHRT_MIN, rect.x());
            r->y = qMax(SHRT_MIN, rect.y());
            r->width = qMin(USHRT_MAX, rect.width());
            r->height = qMin(USHRT_MAX, rect.height());
            ++r;
        }
    }
    if (isNull())
        num = 0;
    else
        num = d->qt_rgn->numRects;
    return d->xrectangles;
}

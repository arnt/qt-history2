/****************************************************************************
**
** Implementation of X11 region translation.
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

#include <private/qt_x11_p.h>

#include <limits.h>

QRegion::QRegionData QRegion::shared_empty = {Q_ATOMIC_INIT(1), 0, 0, 0};

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
    if (!d->xrectangles && !(d == &shared_empty || d->qt_rgn->numRects == 0)) {
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
    if (d == &shared_empty || d->qt_rgn->numRects == 0)
        num = 0;
    else
        num = d->qt_rgn->numRects;
    return d->xrectangles;
}

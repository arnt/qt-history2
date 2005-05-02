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

#include <private/qt_mac_p.h>
#include "qcoreapplication.h"

QRegion::QRegionData QRegion::shared_empty = { Q_ATOMIC_INIT(1), 0, 0 };

#define RGN_CACHE_SIZE 200
#ifdef RGN_CACHE_SIZE
static bool rgncache_init = false;
static int rgncache_used;
static RgnHandle rgncache[RGN_CACHE_SIZE];
static void qt_mac_cleanup_rgncache()
{
    rgncache_init = false;
    for(int i = 0; i < RGN_CACHE_SIZE; ++i) {
        if(rgncache[i]) {
            --rgncache_used;
            DisposeRgn(rgncache[i]);
            rgncache[i] = 0;
        }
    }
}
#endif

Q_GUI_EXPORT RgnHandle qt_mac_get_rgn()
{
#ifdef RGN_CACHE_SIZE
    if(!rgncache_init) {
        rgncache_used = 0;
        rgncache_init = true;
        for(int i = 0; i < RGN_CACHE_SIZE; ++i)
            rgncache[i] = 0;
        qAddPostRoutine(qt_mac_cleanup_rgncache);
    } else if(rgncache_used) {
        for(int i = 0; i < RGN_CACHE_SIZE; ++i) {
            if(rgncache[i]) {
                RgnHandle ret = rgncache[i];
                SetEmptyRgn(ret);
                rgncache[i] = 0;
                --rgncache_used;
                return ret;
            }
        }
    }
#endif
    return NewRgn();
}

Q_GUI_EXPORT void qt_mac_dispose_rgn(RgnHandle r)
{
#ifdef RGN_CACHE_SIZE
    if(rgncache_init && rgncache_used < RGN_CACHE_SIZE) {
        for(int i = 0; i < RGN_CACHE_SIZE; ++i) {
            if(!rgncache[i]) {
                ++rgncache_used;
                rgncache[i] = r;
                return;
            }
        }
    }
#endif
    DisposeRgn(r);
}

static OSStatus qt_mac_get_rgn_rect(UInt16 msg, RgnHandle, const Rect *rect, void *reg)
{
    if(msg == kQDRegionToRectsMsgParse) {
        QRect rct(rect->left, rect->top, (rect->right - rect->left), (rect->bottom - rect->top));
        if(!rct.isEmpty())
            *((QRegion *)reg) += rct;
    }
    return noErr;
}

Q_GUI_EXPORT QRegion qt_mac_convert_mac_region(RgnHandle rgn)
{
    QRegion ret;
    ret.detach();
    RegionToRectsUPP cbk = NewRegionToRectsUPP(qt_mac_get_rgn_rect);
    OSStatus oss = QDRegionToRects(rgn, kQDParseRegionFromTopLeft, cbk, (void *)&ret);
    DisposeRegionToRectsUPP(cbk);
    if(oss != noErr)
        return QRegion();
    return ret;
}

QRegion qt_mac_convert_mac_region(HIShapeRef shape)
{
    RgnHandle rgn = qt_mac_get_rgn();
    HIShapeGetAsQDRgn(shape, rgn);
    QRegion ret = qt_mac_convert_mac_region(rgn);
    qt_mac_dispose_rgn(rgn);
    return ret;
}

/*!
    \internal
*/
RgnHandle QRegion::handle(bool require_rgn) const
{
    if(!d->rgn && (require_rgn || (d->qt_rgn && d->qt_rgn->numRects > 1))) {
        d->rgn = qt_mac_get_rgn();
        if(d->qt_rgn && d->qt_rgn->numRects) {
            RgnHandle tmp_rgn = qt_mac_get_rgn();
            for(int i = 0; i < d->qt_rgn->numRects; ++i) {
                const QRect &qt_r = d->qt_rgn->rects[i];
                SetRectRgn(tmp_rgn, qMax(SHRT_MIN, qt_r.x()), qMax(SHRT_MIN, qt_r.y()),
                           qMin(SHRT_MAX, qt_r.right() + 1),
                           qMin(SHRT_MAX, qt_r.bottom() + 1));
                UnionRgn(d->rgn, tmp_rgn, d->rgn);
            }
            qt_mac_dispose_rgn(tmp_rgn);
        }
    }
    return d->rgn;
}

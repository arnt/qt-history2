/****************************************************************************
**
** Implementation of Mac region translation.
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

#include "qt_mac.h"
#include "qcoreapplication.h"

static QRegionPrivate qrp; //### make it work now without checking for null pointers all the time
QRegion::QRegionData QRegion::shared_empty = { Q_ATOMIC_INIT(1), 0, &qrp };

#define RGN_CACHE_SIZE 200
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

RgnHandle qt_mac_get_rgn()
{
    RgnHandle ret = 0;
    if(!rgncache_init) {
        rgncache_used = 0;
        rgncache_init = true;
        for(int i = 0; i < RGN_CACHE_SIZE; ++i)
            rgncache[i] = 0;
        qAddPostRoutine(qt_mac_cleanup_rgncache);
    } else if(rgncache_used) {
        for(int i = 0; i < RGN_CACHE_SIZE; ++i) {
            if(rgncache[i]) {
                ret = rgncache[i];
                SetEmptyRgn(ret);
                rgncache[i] = 0;
                --rgncache_used;
                break;
            }
        }
    }
    if(!ret)
        ret = NewRgn();
    return ret;
}

void qt_mac_dispose_rgn(RgnHandle r)
{
    if(rgncache_init && rgncache_used < RGN_CACHE_SIZE) {
        for(int i = 0; i < RGN_CACHE_SIZE; ++i) {
            if(!rgncache[i]) {
                ++rgncache_used;
                rgncache[i] = r;
                break;
            }
        }
    } else {
        DisposeRgn(r);
    }
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

QRegion qt_mac_convert_mac_region(RgnHandle rgn)
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
    if(!d->rgn && (require_rgn || d->qt_rgn->numRects > 1)) {
        d->rgn = qt_mac_get_rgn();
        if(d->qt_rgn->numRects) {
            RgnHandle tmp_rgn = qt_mac_get_rgn();
            for(int i = 0; i < d->qt_rgn->numRects; ++i) {
                QRect qt_r = d->qt_rgn->rects[i];
                SetRectRgn(tmp_rgn, qMax(SHRT_MIN, qt_r.x()), qMax(SHRT_MIN, qt_r.y()),
                           qMin(SHRT_MAX, qt_r.right() + 1), qMin(SHRT_MAX, qt_r.bottom() + 1));
                UnionRgn(d->rgn, tmp_rgn, d->rgn);
            }
            qt_mac_dispose_rgn(tmp_rgn);
        }
    }
    return d->rgn;
}

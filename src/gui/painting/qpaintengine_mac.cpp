/****************************************************************************
**
** Definition of QQuickDrawPaintEngine/QCoreGraphicsPaintEngine  class.
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

#include <qbitmap.h>
#include <qpaintdevice.h>
#include <qpaintdevicemetrics.h>
#include <qpaintengine_mac.h>
#include <qpixmapcache.h>
#include <qprinter.h>
#include <qstack.h>
#include <private/qt_mac_p.h>
#include <qtextcodec.h>
#include <qtextcodec.h>
#include <qwidget.h>

#include <private/qfontdata_p.h>
#include <private/qfontengine_p.h>
#include <private/qpaintengine_mac_p.h>
#include <private/qpainter_p.h>
#include <private/qwidget_p.h>

#include <string.h>

#define d d_func()
#define q q_func()

/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/

/*****************************************************************************
  External functions
 *****************************************************************************/
extern QPoint posInWindow(QWidget *w); //qwidget_mac.cpp
extern QRegion make_region(RgnHandle handle);
extern WindowPtr qt_mac_window_for(const QWidget *); //qwidget_mac.cpp
extern void qt_mac_clip_cg(CGContextRef, const QRegion &, const QPoint *); //qpaintdevice_mac.cpp
extern void qt_mac_clip_cg_reset(CGContextRef); //qpaintdevice_mac.cpp
extern GrafPtr qt_macQDHandle(const QPaintDevice *); //qpaintdevice_mac.cpp
extern CGContextRef qt_macCGHandle(const QPaintDevice *); //qpaintdevice_mac.cpp
extern RgnHandle qt_mac_get_rgn(); //qregion_mac.cpp
CGImageRef qt_mac_create_cgimage(const QPixmap &, bool); //qpixmap_mac.cpp
extern void qt_mac_dispose_rgn(RgnHandle r); //qregion_mac.cpp
extern const uchar *qt_patternForBrush(int, bool); //qbrush.cpp
extern QPixmap qt_pixmapForBrush(int, bool); //qbrush.cpp

// paintevent magic to provide Windows semantics on Qt/Mac
class paintevent_item
{
    QPaintDevice* dev;
    QRegion clipRegion;
public:
    paintevent_item(QPaintDevice *dv, QRegion r) : dev(dv), clipRegion(r) { }
    inline bool operator==(const QPaintDevice *rhs) const { return rhs == dev; }
    inline bool operator!=(const QPaintDevice *rhs) const { return !(this->operator==(rhs)); }
    inline QPaintDevice *device() const { return dev; }
    inline QRegion region() const { return clipRegion; }
};
QStack<paintevent_item*> paintevents;
static paintevent_item *qt_mac_get_paintevent() { return paintevents.isEmpty() ? 0 : paintevents.top(); }

void qt_set_paintevent_clipping(QPaintDevice* dev, const QRegion& region)
{
    QRegion r = region;
    if(dev && dev->devType() == QInternal::Widget) {
        QWidget *w = (QWidget *)dev;
        QPoint mp(posInWindow(w));
        r.translate(mp.x(), mp.y());
    }
    if(paintevent_item *curr = qt_mac_get_paintevent()) {
        if(curr->device() == dev)
            r &= curr->region();
    }
    paintevents.push(new paintevent_item(dev, r));
}

void qt_clear_paintevent_clipping(QPaintDevice *dev)
{
    if(paintevents.isEmpty() || !((*paintevents.top()) == dev)) {
        qDebug("Qt: internal: WH0A, qt_clear_paintevent_clipping mismatch.");
        return;
    }
    delete paintevents.pop();
}

/*****************************************************************************
  QQuickDrawPaintEngine member functions
 *****************************************************************************/
QQuickDrawPaintEngine::QQuickDrawPaintEngine()
    : QPaintEngine(*(new QQuickDrawPaintEnginePrivate), PaintEngineFeatures(UsesFontEngine|PixmapScale))
{
}

QQuickDrawPaintEngine::QQuickDrawPaintEngine(QPaintEnginePrivate &dptr, PaintEngineFeatures devcaps)
    : QPaintEngine(dptr, devcaps)
{
}

QQuickDrawPaintEngine::~QQuickDrawPaintEngine()
{
}

bool
QQuickDrawPaintEngine::begin(QPaintDevice *pdev)
{
    if(isActive()) {                         // already active painting
        qWarning("QQuickDrawPaintEngine::begin: Painter is already active."
                  "\n\tYou must end() the painter before a second begin()");
//        return false;
    }

    //save the gworld now, we'll reset it in end()
    d->saved = new QMacSavedPortInfo;

    d->pdev = pdev;
    setActive(true);
    assignf(IsActive | DirtyFont);

    if(d->pdev->devType() == QInternal::Pixmap)         // device is a pixmap
        ((QPixmap*)d->pdev)->detach();             // will modify it

    d->clip.serial = 0;
    d->paintevent = 0;
    d->clip.dirty = false;
    d->offx = d->offy = 0;

    bool unclipped = false;
    if(d->pdev->devType() == QInternal::Widget) {                    // device is a widget
        QWidget *w = (QWidget*)d->pdev;
        { //offset painting in widget relative the tld
            QPoint wp = posInWindow(w);
            d->offx = wp.x();
            d->offy = wp.y();
        }
        bool unclipped = w->testAttribute(Qt::WA_PaintUnclipped);

        if(!d->locked) {
            LockPortBits(GetWindowPort(qt_mac_window_for(w)));
            d->locked = true;
        }

        if(w->isDesktop()) {
            if(!unclipped)
                qWarning("QQuickDrawPaintEngine::begin: Does not support clipped desktop on MacOSX");
            ShowWindow(qt_mac_window_for(w));
        } else if(unclipped) {
            qWarning("QQuickDrawPaintEngine::begin: Does not support unclipped painting");
        }
    } else if(d->pdev->devType() == QInternal::Pixmap) {             // device is a pixmap
        QPixmap *pm = (QPixmap*)d->pdev;
        if(pm->isNull()) {
            qWarning("QQuickDrawPaintEngine::begin: Cannot paint null pixmap");
            end();
            return false;
        }
    }
    d->unclipped = unclipped;
    if(type() != CoreGraphics)
        setupQDPort(true); //force setting paint device, this does unclipped fu
    return true;
}

bool
QQuickDrawPaintEngine::end()
{
    setActive(false);

    if(d->locked) {
        if(d->pdev->devType() == QInternal::Widget)
            UnlockPortBits(GetWindowPort(qt_mac_window_for(static_cast<QWidget*>(d->pdev))));
        d->locked = false;
    }

    delete d->saved;
    d->saved = 0;
    if(d->pdev->devType() == QInternal::Widget && ((QWidget*)d->pdev)->isDesktop())
        HideWindow(qt_mac_window_for(static_cast<QWidget*>(d->pdev)));

    d->pdev = 0;
    return true;
}

void
QQuickDrawPaintEngine::updatePen(const QPen &pen)
{
    d->current.pen = pen;
}


void
QQuickDrawPaintEngine::updateBrush(const QBrush &brush, const QPoint &origin)
{
    d->current.brush = brush;
    d->current.bg.origin = origin;
}

void
QQuickDrawPaintEngine::updateFont(const QFont &)
{
    clearf(DirtyFont);
    updatePen(d->current.pen);
}

void
QQuickDrawPaintEngine::updateBackground(Qt::BGMode mode, const QBrush &bgBrush)
{
    Q_ASSERT(isActive());
    d->current.bg.mode = mode;
    d->current.bg.brush = bgBrush;
}

void
QQuickDrawPaintEngine::updateXForm(const QWMatrix &)
{
}

void
QQuickDrawPaintEngine::setClippedRegionInternal(QRegion *rgn)
{
    if(rgn)
        setf(ClipOn);
    else
        clearf(ClipOn);
    if(rgn)
        d->current.clip = *rgn;
    d->clip.dirty = 1;
}

void
QQuickDrawPaintEngine::updateClipRegion(const QRegion &region, bool enable)
{
    Q_ASSERT(isActive());
    setClippedRegionInternal(enable ? const_cast<QRegion *>(&region) : 0);
}

void
QQuickDrawPaintEngine::drawLine(const QPoint &pt1, const QPoint &pt2)
{
    Q_ASSERT(isActive());
    setupQDPort();
    if(d->clip.paintable.isEmpty())
        return;
    setupQDPen();
    MoveTo(pt1.x()+d->offx,pt1.y()+d->offy);
    LineTo(pt2.x()+d->offx,pt2.y()+d->offy);
}

void
QQuickDrawPaintEngine::drawRect(const QRect &r)
{
    Q_ASSERT(isActive());
    setupQDPort();
    if(d->clip.paintable.isEmpty())
        return;

    Rect rect;
    SetRect(&rect, r.x()+d->offx, r.y()+d->offy, r.right()+d->offx+1, r.bottom()+d->offy+1);
    if(d->current.brush.style() != Qt::NoBrush) {
        setupQDBrush();
        if(d->current.brush.style() == Qt::SolidPattern) {
            PaintRect(&rect);
        } else {
            QPixmap *pm = 0;
            if(d->brush_style_pix) {
                pm = d->brush_style_pix;
                if(d->current.bg.mode == Qt::OpaqueMode) {
                    ::RGBColor f;
                    f.red = d->current.bg.brush.color().red()*256;
                    f.green = d->current.bg.brush.color().green()*256;
                    f.blue = d->current.bg.brush.color().blue()*256;
                    RGBForeColor(&f);
                    PaintRect(&rect);
                }
            } else {
                pm = d->current.brush.pixmap();
            }
            if(pm && !pm->isNull()) {
                //save the clip
                bool clipon = testf(ClipOn);
                QRegion clip = d->current.clip;

                //create the region
                QRegion newclip(r);
                if(clipon)
                    newclip &= clip;
                setClippedRegionInternal(&newclip);

                //draw the brush
                drawTiledPixmap(r, *pm, r.topLeft() - d->current.bg.origin);

                //restore the clip
                setClippedRegionInternal(clipon ? &clip : 0);
            }
        }
    }
    if(d->current.pen.style() != Qt::NoPen) {
        setupQDPen();
        FrameRect(&rect);
    }
}

void
QQuickDrawPaintEngine::drawPoint(const QPoint &pt)
{
    Q_ASSERT(isActive());
    if(d->current.pen.style() != Qt::NoPen) {
        setupQDPort();
        if(d->clip.paintable.isEmpty())
            return;
        setupQDPen();
        MoveTo(pt.x() + d->offx, pt.y() + d->offy);
        Line(0,1);
    }
}

void
QQuickDrawPaintEngine::drawPoints(const QPointArray &pa, int index, int npoints)
{
    Q_ASSERT(isActive());

    if(d->current.pen.style() != Qt::NoPen) {
        setupQDPort();
        if(d->clip.paintable.isEmpty())
            return;
        setupQDPen();
        for(int i=0; i<npoints; i++) {
            MoveTo(pa[index+i].x()+d->offx, pa[index+i].y()+d->offy);
            Line(0,1);
        }
    }
}

void
QQuickDrawPaintEngine::drawRoundRect(const QRect &r, int xRnd, int yRnd)
{
    Q_ASSERT(isActive());

    setupQDPort();
    if(d->clip.paintable.isEmpty())
        return;

    Rect rect;
    SetRect(&rect, r.x()+d->offx, r.y()+d->offy, r.right()+d->offx+1, r.bottom()+d->offy+1);
    if(d->current.brush.style() == Qt::SolidPattern) {
        setupQDBrush();
        PaintRoundRect(&rect, r.width()*xRnd/100, r.height()*yRnd/100);
    }
    if(d->current.pen.style() != Qt::NoPen) {
        setupQDPen();
        FrameRoundRect(&rect, r.width()*xRnd/100, r.height()*yRnd/100);
    }
}

void
QQuickDrawPaintEngine::drawPolyInternal(const QPointArray &pa, bool close)
{
    Q_ASSERT(isActive());

    setupQDPort();
    if(d->clip.paintable.isEmpty())
        return;

    PolyHandle polyHandle = OpenPoly();
    MoveTo(pa[0].x()+d->offx, pa[0].y()+d->offy);
    for(int x = 1; x < pa.size(); x++) 
        LineTo(pa[x].x()+d->offx, pa[x].y()+d->offy);
    if(close) 
        LineTo(pa[0].x()+d->offx, pa[0].y()+d->offy);
    ClosePoly();

    if(close && d->current.brush.style() != Qt::NoBrush) {
        setupQDBrush();
        if(d->current.brush.style() == Qt::SolidPattern) {
            PaintPoly(polyHandle);
        } else {
            QPixmap *pm = 0;
            if(d->brush_style_pix) {
                pm = d->brush_style_pix;
                if(d->current.bg.mode == Qt::OpaqueMode) {
                    ::RGBColor f;
                    f.red = d->current.bg.brush.color().red()*256;
                    f.green = d->current.bg.brush.color().green()*256;
                    f.blue = d->current.bg.brush.color().blue()*256;
                    RGBForeColor(&f);
                    PaintPoly(polyHandle);
                }
            } else {
                pm = d->current.brush.pixmap();
            }

            if(pm && !pm->isNull()) {
                //save the clip
                bool clipon = testf(ClipOn);
                QRegion clip = d->current.clip;

                //create the region
                QRegion newclip(pa);
                if(clipon)
                    newclip &= clip;
                setClippedRegionInternal(&newclip);

                //draw the brush
                QRect r(pa.boundingRect());
                drawTiledPixmap(r, *pm, r.topLeft() - d->current.bg.origin);

                //restore the clip
                setClippedRegionInternal(clipon ? &clip : 0);
            }
        }
    }
    if(d->current.pen.style() != Qt::NoPen) {
        setupQDPen();
        FramePoly(polyHandle);
    }
    KillPoly(polyHandle);
}

void
QQuickDrawPaintEngine::drawEllipse(const QRect &r)
{
    Q_ASSERT(isActive());

    setupQDPort();
    if(d->clip.paintable.isEmpty())
        return;

    Rect mac_r;
    SetRect(&mac_r, r.x()+d->offx, r.y()+d->offy, r.right()+d->offx+1, r.bottom()+d->offy+1);
    if(d->current.brush.style() != Qt::NoBrush) {
        setupQDBrush();
        if(d->current.brush.style() == Qt::SolidPattern) {
            PaintOval(&mac_r);
        } else {
            QPixmap *pm = 0;
            if(d->brush_style_pix) {
                pm = d->brush_style_pix;
                if(d->current.bg.mode == Qt::OpaqueMode) {
                    ::RGBColor f;
                    f.red = d->current.bg.brush.color().red()*256;
                    f.green = d->current.bg.brush.color().green()*256;
                    f.blue = d->current.bg.brush.color().blue()*256;
                    RGBForeColor(&f);
                    PaintOval(&mac_r);
                }
            } else {
                pm = d->current.brush.pixmap();
            }
            if(pm && !pm->isNull()) {
                //save the clip
                bool clipon = testf(ClipOn);
                QRegion clip = d->current.clip;

                //create the region
                QRegion newclip(r, QRegion::Ellipse);
                if(clipon)
                    newclip &= clip;
                setClippedRegionInternal(&newclip);

                //draw the brush
                drawTiledPixmap(r, *pm, r.topLeft() - d->current.bg.origin);

                //restore the clip
                setClippedRegionInternal(clipon ? &clip : 0);
            }
        }
    }

    if(d->current.pen.style() != Qt::NoPen) {
        setupQDPen();
        FrameOval(&mac_r);
    }
}

void
QQuickDrawPaintEngine::drawChord(const QRect &r, int a, int alen)
{
    Q_ASSERT(isActive());

    QPointArray pa;
    pa.makeArc(r.x(), r.y(), r.width()-1, r.height()-1, a, alen);
    int n = pa.size();
    pa.resize(n + 1);
    pa.setPoint(n, pa.at(0));
    drawPolyInternal(pa);
}

void
QQuickDrawPaintEngine::drawLineSegments(const QPointArray &pa, int index, int nlines)
{
    Q_ASSERT(isActive());

    setupQDPort();
    if(d->clip.paintable.isEmpty())
        return;

    setupQDPen();
    for(int i = index, x1, x2, y1, y2; nlines; nlines--) {
        pa.point(i++, &x1, &y1);
        pa.point(i++, &x2, &y2);
        MoveTo(x1 + d->offx, y1 + d->offy);
        LineTo(x2 + d->offx, y2 + d->offy);
    }
}

void
QQuickDrawPaintEngine::drawPolyline(const QPointArray &pa, int index, int npoints)
{
    Q_ASSERT(isActive());
    if(npoints == -1)
        npoints = pa.size()-index;
    if(pa.size() < index+npoints || npoints < 2)
        return;

    int x1, y1, x2, y2, xsave, ysave;
    pa.point(index+npoints-2, &x1, &y1);      // last line segment
    pa.point(index+npoints-1, &x2, &y2);
    xsave = x2; ysave = y2;
    bool plot_pixel = false;
    if(x1 == x2) {                           // vertical
        if(y1 < y2)
            y2++;
        else
            y2--;
    } else if(y1 == y2) {                    // horizontal
        if(x1 < x2)
            x2++;
        else
            x2--;
    } else {
        plot_pixel = d->current.pen.style() == Qt::SolidLine; // plot last pixel
    }
    setupQDPort();
    if(d->clip.paintable.isEmpty())
        return;

    setupQDPen();
    /* We draw 5000 chunks at a time because of limitations in QD */
    for(int chunk = index; chunk < npoints;) {
        //make a region of it
        PolyHandle poly = OpenPoly();
        MoveTo(pa[chunk].x()+d->offx, pa[chunk].y()+d->offy);
        for(int last_chunk=chunk+5000; chunk < last_chunk; chunk++) {
            if(chunk == npoints)
                break;
            LineTo(pa[chunk].x()+d->offx, pa[chunk].y()+d->offy);
        }
        ClosePoly();
        //now draw it
        FramePoly(poly);
        KillPoly(poly);
    }
}

void
QQuickDrawPaintEngine::drawPolygon(const QPointArray &a, bool /*winding*/, int index, int npoints)
{
    Q_ASSERT(isActive());
    QPointArray pa;
    if(index != 0 || npoints != (int)a.size()) {
        pa = QPointArray(npoints);
        for(int i=0; i<npoints; i++)
            pa.setPoint(i, a.point(index+i));
        index = 0;
    } else {
        pa = a;
    }
    drawPolyInternal(pa, true);
}

void
QQuickDrawPaintEngine::drawConvexPolygon(const QPointArray &pa, int index, int npoints)
{
    // Implemented in terms of drawPolygon() [no optimization]
    drawPolygon(pa, false, index, npoints);
}

#ifndef QT_NO_BEZIER
void
QQuickDrawPaintEngine::drawCubicBezier(const QPointArray &pa, int index)
{
    Q_ASSERT(isActive());
    QPointArray a(pa);
    if(index != 0 || a.size() > 4) {
        a = QPointArray(4);
        for(int i=0; i<4; i++)
            a.setPoint(i, pa.point(index+i));
    }
    drawPolyline(a.cubicBezier(), index);
}
#endif

void
QQuickDrawPaintEngine::drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &p)
{
    int yPos=r.y(), xPos, drawH, drawW, yOff=p.y(), xOff;
    while(yPos < r.bottom()) {
        drawH = pixmap.height() - yOff;    // Cropping first row
        if(yPos + drawH > r.bottom())        // Cropping last row
            drawH = r.bottom() - yPos;
        xPos = r.x();
        xOff = p.x();
        while(xPos < r.right()) {
            drawW = pixmap.width() - xOff; // Cropping first column
            if(xPos + drawW > r.right())    // Cropping last column
                drawW = r.right() - xPos;
            drawPixmap(QRect(xPos, yPos, drawW, drawH), pixmap, QRect(xOff, yOff, drawW, drawH),
                       Qt::AlphaBlend);
            xPos += drawW;
            xOff = 0;
        }
        yPos += drawH;
        yOff = 0;
    }
}

void
QQuickDrawPaintEngine::drawPie(const QRect &r, int a, int alen)
{
    Q_ASSERT(isActive());

    QPointArray pa;
    pa.makeArc(r.x(), r.y(), r.width(), r.height(), a, alen); // arc polyline
    int n = pa.size();
    pa.resize(n+2);
    pa.setPoint(n, r.x()+(r.width()/2), r.y()+(r.height()/2));        // add legs
    pa.setPoint(n+1, pa.at(0));
    drawPolyInternal(pa, true);
}

void
QQuickDrawPaintEngine::drawArc(const QRect &r, int a, int alen)
{
    Q_ASSERT(isActive());

    QPointArray pa;
    pa.makeArc(r.x(), r.y(), r.width(), r.height(), a, alen); // arc polyline
    drawPolyline(pa);
}

void
QQuickDrawPaintEngine::drawPixmap(const QRect &r, const QPixmap &pixmap, const QRect &sr,
                                  Qt::BlendMode mode)
{
    Q_ASSERT(isActive());
    if(pixmap.isNull())
        return;

    setupQDPort();
    if(d->clip.paintable.isEmpty())
        return;

    //setup port
    ::RGBColor f;
    if(pixmap.depth() == 1) {
        f.red = d->current.pen.color().red()*256;
        f.green = d->current.pen.color().green()*256;
        f.blue = d->current.pen.color().blue()*256;
    } else {
        f.red = f.green = f.blue = 0;
    }
    RGBForeColor(&f);
    f.red = f.green = f.blue = ~0;
    RGBBackColor(&f);

    //get pixmap bits
    const BitMap *srcbitmap = GetPortBitMapForCopyBits(qt_macQDHandle(&pixmap));
    const QPixmap *srcmask=0;
    if(mode == Qt::AlphaBlend) {
        if(pixmap.data->alphapm)
            srcmask = pixmap.data->alphapm;
        else
            srcmask = pixmap.mask();
    }

    //get pdev bits
    const BitMap *dstbitmap=0;
    switch(d->pdev->devType()) {
    case QInternal::Widget: {
        QWidget *w = (QWidget *)d->pdev;
        dstbitmap = GetPortBitMapForCopyBits(GetWindowPort(qt_mac_window_for(w)));
        break; }
    case QInternal::Printer:
    case QInternal::Pixmap: {
        dstbitmap = GetPortBitMapForCopyBits(qt_macQDHandle(d->pdev));
        break; }
    }

    //get copy mode
    short copymode = srcCopy;
    if(srcmask && srcmask->depth() > 1)
        copymode = ditherCopy;

    //do the blt
    Rect srcr;
    SetRect(&srcr, sr.x(), sr.y(), sr.x()+sr.width()+1, sr.y()+sr.height()+1);
    Rect dstr;
    SetRect(&dstr, d->offx+r.x(), d->offy+r.y(), d->offx+r.x()+r.width()+1, d->offy+r.y()+r.height()+1);
    if(srcmask) {
        const BitMap *maskbits = GetPortBitMapForCopyBits(qt_macQDHandle(srcmask));
        if(d->pdev->devType() == QInternal::Printer) { //can't use CopyDeepMask on a printer
            QPixmap tmppix(r.width(), r.height(), pixmap.depth());
            Rect pixr;
            SetRect(&pixr, 0, 0, r.width()+1, r.height()+1);
            const BitMap *pixbits = GetPortBitMapForCopyBits((GWorldPtr)tmppix.handle());
            {
                QMacSavedPortInfo pi(&tmppix);
                EraseRect(&pixr);
                CopyDeepMask(srcbitmap, maskbits, pixbits, &srcr, &srcr, &pixr, copymode, 0);
            }
            setupQDPort(true);
            CopyBits(pixbits, dstbitmap, &pixr, &dstr, srcOr, 0); //use srcOr transfer, to "emulate" the mask
        } else {
            CopyDeepMask(srcbitmap, maskbits, dstbitmap, &srcr, &srcr, &dstr, copymode, 0);
        }
    } else {
        CopyBits(srcbitmap, dstbitmap, &srcr, &dstr, copymode, 0);
    }
}

void
QQuickDrawPaintEngine::initialize()
{
}

void
QQuickDrawPaintEngine::cleanup()
{
}

/*!
    \internal
*/
void
QQuickDrawPaintEngine::setupQDPen()
{
    //pen size
    int dot = d->current.pen.width();
    if(dot < 1)
        dot = 1;
    PenSize(dot, dot);

    //forecolor
    ::RGBColor f;
    f.red = d->current.pen.color().red()*256;
    f.green = d->current.pen.color().green()*256;
    f.blue = d->current.pen.color().blue()*256;
    Pattern pat;
    GetQDGlobalsBlack(&pat);
    PenPat(&pat);
    RGBForeColor(&f);

    //backcolor
    ::RGBColor b;
    b.red = d->current.bg.brush.color().red()*256;
    b.green = d->current.bg.brush.color().green()*256;
    b.blue = d->current.bg.brush.color().blue()*256;
    RGBBackColor(&b);

    //penmodes
    PenMode(patCopy);
}

/*!
    \internal
*/
void
QQuickDrawPaintEngine::setupQDBrush()
{
    //pattern
    delete d->brush_style_pix;
    d->brush_style_pix = 0;
    int bs = d->current.brush.style();
    if(bs >= Qt::Dense1Pattern && bs <= Qt::DiagCrossPattern) {
        d->brush_style_pix = new QPixmap(8, 8);
        d->brush_style_pix->setMask(qt_pixmapForBrush(bs, true));
        d->brush_style_pix->fill(d->current.brush.color());
    } else if(bs == Qt::CustomPattern) {
        if(d->current.brush.pixmap()->isQBitmap()) {
            d->brush_style_pix = new QPixmap(d->current.brush.pixmap()->width(),
                                             d->current.brush.pixmap()->height());
            d->brush_style_pix->setMask(*((QBitmap*)d->current.brush.pixmap()));
            d->brush_style_pix->fill(d->current.brush.color());
        }
    }

    //forecolor
    ::RGBColor f;
    f.red = d->current.brush.color().red()*256;
    f.green = d->current.brush.color().green()*256;
    f.blue = d->current.brush.color().blue()*256;
    Pattern pat;
    GetQDGlobalsBlack(&pat);
    PenPat(&pat);
    RGBForeColor(&f);

    //backcolor
    ::RGBColor b;
    b.red = d->current.bg.brush.color().red()*256;
    b.green = d->current.bg.brush.color().green()*256;
    b.blue = d->current.bg.brush.color().blue()*256;
    RGBBackColor(&b);

    //penmodes
    PenMode(patCopy);
}

/*!
    \internal
*/
void
QQuickDrawPaintEngine::setupQDFont()
{
    setupQDPen();
}

/*!
    \internal
*/
void QQuickDrawPaintEngine::setupQDPort(bool force, QPoint *off, QRegion *rgn)
{
    bool remade_clip = false;
    if(d->pdev->devType() == QInternal::Printer) {
        if(force) {
            remade_clip = true;
            d->clip.pdev = QRegion(0, 0, d->pdev->metric(QPaintDeviceMetrics::PdmWidth),
                                          d->pdev->metric(QPaintDeviceMetrics::PdmHeight));
        }
    } else if(d->pdev->devType() == QInternal::Widget) {                    // device is a widget
        paintevent_item *pevent = qt_mac_get_paintevent();
        if(pevent && (*pevent) != d->pdev)
            pevent = 0;
        QWidget *w = (QWidget*)d->pdev;
        if(!(remade_clip = force)) {
            if(pevent != d->paintevent)
                remade_clip = true;
            else if(!w->isVisible())
                remade_clip = d->clip.serial;
            else
                remade_clip = (d->clip.serial != w->d_func()->clippedSerial(!d->unclipped));
        }
        if(remade_clip) {
            //offset painting in widget relative the tld
            QPoint wp = posInWindow(w);
            d->offx = wp.x();
            d->offy = wp.y();

            if(!w->isVisible()) {
                d->clip.pdev = QRegion(0, 0, 0, 0); //make the clipped reg empty if not visible!!!
                d->clip.serial = 0;
            } else {
                d->clip.pdev = w->d_func()->clippedRegion(!d->unclipped);
                d->clip.serial = w->d_func()->clippedSerial(!d->unclipped);
            }
            if(pevent)
                d->clip.pdev &= pevent->region();
            d->paintevent = pevent;
        }
    } else if(d->pdev->devType() == QInternal::Pixmap) {             // device is a pixmap
        QPixmap *pm = (QPixmap*)d->pdev;
        if(force) {//clip out my bounding rect
            remade_clip = true;
            d->clip.pdev = QRegion(0, 0, pm->width(), pm->height());
        }
    }
    if(remade_clip || d->clip.dirty) {         //update clipped region
        remade_clip = true;
        if(!d->clip.pdev.isEmpty() && testf(ClipOn)) {
            d->clip.paintable = d->current.clip;
            d->clip.paintable.translate(d->offx, d->offy);
            d->clip.paintable &= d->clip.pdev;
        } else {
            d->clip.paintable = d->clip.pdev;
        }

        CGrafPtr ptr = qt_macQDHandle(d->pdev);
        if(RgnHandle rgn = d->clip.paintable.handle()) {
            QDAddRegionToDirtyRegion(ptr, rgn);
        } else {
            QRect qr = d->clip.paintable.boundingRect();
            Rect mr; SetRect(&mr, qr.x(), qr.y(), qr.right()+1, qr.bottom()+1);
            QDAddRectToDirtyRegion(ptr, &mr);
        }
        d->clip.dirty = false;
    }
    { //setup the port
        QMacSavedPortInfo::setPaintDevice(d->pdev);
        if(type() != CoreGraphics)
            QMacSavedPortInfo::setClipRegion(d->clip.paintable);
    }
    if(off)
        *off = QPoint(d->offx, d->offy);
    if(rgn)
        *rgn = d->clip.paintable;
}

/*****************************************************************************
  QCoreGraphicsPaintEngine utility functions
 *****************************************************************************/

//colour conversion
inline static float qt_mac_convert_color_to_cg(int c) { return ((float)c * 1000 / 255) / 1000; }

//pattern handling (tiling)
struct QMacPattern {
    QMacPattern() : as_mask(false), image(0) { data.pixmap = 0; }
    //input
    bool as_mask;
    union {
        const QPixmap *pixmap;
        const uchar *bytes;
    } data;
    //output
    CGImageRef image;
};
static void qt_mac_draw_pattern(void *info, CGContextRef c)
{
    QMacPattern *pat = (QMacPattern*)info;
    int w = 0, h = 0;
    if (!pat->image) {
        if (pat->as_mask) {
            w = h = 8;
            CGDataProviderRef provider = CGDataProviderCreateWithData(0, pat->data.bytes, 64, 0);
            pat->image = CGImageMaskCreate(w, h, 1, 1, 1, provider, 0, false);
            CGDataProviderRelease(provider);
        } else {
            w = pat->data.pixmap->width();
            h = pat->data.pixmap->height();
            pat->image = qt_mac_create_cgimage(*pat->data.pixmap, false);
        }
    }
    CGRect rect = CGRectMake(0, 0, w, h);
    HIViewDrawCGImage(c, &rect, pat->image); //HIViews render the way we want anyway, so just use the convenience..
}
static void qt_mac_dispose_pattern(void *info)
{
    QMacPattern *pat = (QMacPattern*)info;
    if(pat->image)
        CGImageRelease(pat->image);
    delete pat;
}

//gradiant callback
static void qt_mac_color_gradient_function(void *info, const float *in, float *out)
{
    QBrush *brush = (QBrush*)info;
    const float red = qt_mac_convert_color_to_cg(brush->color().red());
    out[0] = red + in[0] * (qt_mac_convert_color_to_cg(brush->gradientColor().red())-red);
    const float green = qt_mac_convert_color_to_cg(brush->color().green());
    out[1] = green + in[0] * (qt_mac_convert_color_to_cg(brush->gradientColor().green())-green);
    const float blue = qt_mac_convert_color_to_cg(brush->color().blue());
    out[2] = blue + in[0] * (qt_mac_convert_color_to_cg(brush->gradientColor().blue())-blue);
    out[3] = 1; //100%
}

/*****************************************************************************
  QCoreGraphicsPaintEngine member functions
 *****************************************************************************/

QCoreGraphicsPaintEngine::QCoreGraphicsPaintEngine()
    : QQuickDrawPaintEngine(*(new QCoreGraphicsPaintEnginePrivate),
                            PaintEngineFeatures(/*CoordTransform|PenWidthTransform|PixmapTransform|*/PixmapScale|UsesFontEngine|LinearGradients|SolidAlphaFill))
{
}

QCoreGraphicsPaintEngine::QCoreGraphicsPaintEngine(QPaintEnginePrivate &dptr)
    : QQuickDrawPaintEngine(dptr, PaintEngineFeatures(/*CoordTransform|PenWidthTransform|PixmapTransform|*/PixmapScale|UsesFontEngine|LinearGradients|SolidAlphaFill))
{
}

QCoreGraphicsPaintEngine::~QCoreGraphicsPaintEngine()
{
}

bool
QCoreGraphicsPaintEngine::begin(QPaintDevice *pdev)
{
    if(isActive()) {                         // already active painting
        qWarning("QCoreGraphicsPaintEngine::begin: Painter is already active."
                  "\n\tYou must end() the painter before a second begin()");
//        return false;
    }

    d->pdev = pdev;
    d->hd = qt_macCGHandle(d->pdev);
    if(d->shading) {
        CGShadingRelease(d->shading);
        d->shading = 0;
    }
#if 0
    CGContextSetShadowWithColor(d->hd, CGSizeMake(0, 0), 0, 0);
#endif
    d->offx = d->offy = 0; // (quickdraw compat!!)

    setupCGClip(0); //get handle to drawable
    setActive(true);
    assignf(IsActive | DirtyFont);

    if(d->pdev->devType() == QInternal::Pixmap)         // device is a pixmap
        ((QPixmap*)d->pdev)->detach();             // will modify it

    if(d->pdev->devType() == QInternal::Widget) {                    // device is a widget
        QWidget *w = (QWidget*)d->pdev;
        { //offset painting in widget relative the tld (quickdraw compat!!!)
            QPoint wp = posInWindow(w);
            d->offx = wp.x();
            d->offy = wp.y();
        }
	bool unclipped = w->testAttribute(Qt::WA_PaintUnclipped);

        if(w->isDesktop()) {
            if(!unclipped)
                qWarning("QCoreGraphicsPaintEngine::begin: Does not support clipped desktop on MacOSX");
            ShowWindow(qt_mac_window_for(w));
        } else if(unclipped) {
            qWarning("QCoreGraphicsPaintEngine::begin: Does not support unclipped painting");
        }
    } else if(d->pdev->devType() == QInternal::Pixmap) {             // device is a pixmap
        QPixmap *pm = (QPixmap*)d->pdev;
        if(pm->depth() == 1)
            clearRenderHints(QPainter::RenderHints(QPainter::LineAntialiasing
                                                   | QPainter::TextAntialiasing));
        if(pm->isNull()) {
            qWarning("QCoreGraphicsPaintEngine::begin: Cannot paint null pixmap");
            end();
            return false;
        }
    }
    return true;
}

bool
QCoreGraphicsPaintEngine::end()
{
    setActive(false);
    if(d->hd) {
        CGContextSynchronize(d->hd);
        qt_mac_clip_cg_reset(d->hd);
        d->hd = 0;
    }
    if(d->pdev->devType() == QInternal::Widget && ((QWidget*)d->pdev)->isDesktop())
        HideWindow(qt_mac_window_for(static_cast<QWidget*>(d->pdev)));
    if(d->shading) {
        CGShadingRelease(d->shading);
        d->shading = 0;
    }
    d->pdev = 0;
    return true;
}

void
QCoreGraphicsPaintEngine::updatePen(const QPen &pen)
{
    Q_ASSERT(isActive());
    d->current.pen = pen;

    //pen style
    float *lengths = 0;
    int count = 0;
    if(pen.style() == Qt::DashLine) {
        static float inner_lengths[] = { 3, 1 };
        lengths = inner_lengths;
        count = sizeof(sizeof(inner_lengths) / sizeof(inner_lengths[0]));
    } else if(pen.style() == Qt::DotLine) {
        static float inner_lengths[] = { 1, 1 };
        lengths = inner_lengths;
        count = sizeof(sizeof(inner_lengths) / sizeof(inner_lengths[0]));
    } else if(pen.style() == Qt::DashDotLine) {
        static float inner_lengths[] = { 3, 1, 1, 1 };
        lengths = inner_lengths;
        count = sizeof(sizeof(inner_lengths) / sizeof(inner_lengths[0]));
    } else if(pen.style() == Qt::DashDotDotLine) {
        static float inner_lengths[] = { 3, 1, 1, 1, 1, 1 };
        lengths = inner_lengths;
        count = sizeof(sizeof(inner_lengths) / sizeof(inner_lengths[0]));
    }
    CGContextSetLineDash(d->hd, 0, lengths, count);

    //pencap
    CGLineCap cglinecap = kCGLineCapButt;
    if(pen.capStyle() == Qt::SquareCap)
        cglinecap = kCGLineCapSquare;
    else if(pen.capStyle() == Qt::RoundCap)
        cglinecap = kCGLineCapRound;
    CGContextSetLineCap(d->hd, cglinecap);

    //penwidth
    CGContextSetLineWidth(d->hd, pen.width() < 1 ? 1 : pen.width());

    //join
    CGLineJoin cglinejoin = kCGLineJoinMiter;
    if(pen.joinStyle() == Qt::BevelJoin)
        cglinejoin = kCGLineJoinBevel;
    else if(pen.joinStyle() == Qt::RoundJoin)
        cglinejoin = kCGLineJoinRound;
    CGContextSetLineJoin(d->hd, cglinejoin);

    //color
    const QColor &col = pen.color();
    CGContextSetRGBStrokeColor(d->hd, qt_mac_convert_color_to_cg(col.red()),
                               qt_mac_convert_color_to_cg(col.green()),
                               qt_mac_convert_color_to_cg(col.blue()),
                               qt_mac_convert_color_to_cg(col.alpha()));
}

void
QCoreGraphicsPaintEngine::updateBrush(const QBrush &brush, const QPoint &brushOrigin)
{
    Q_ASSERT(isActive());
    d->current.brush = brush;
    d->current.bg.origin = brushOrigin;

    if(d->shading) {
        CGShadingRelease(d->shading);
        d->shading = 0;
    }

    //pattern
    Qt::BrushStyle bs = brush.style();
    if(bs == Qt::LinearGradientPattern) {
        CGFunctionCallbacks callbacks = { 0, qt_mac_color_gradient_function, 0 };
        CGFunctionRef fill_func = CGFunctionCreate(const_cast<void *>(reinterpret_cast<const void *>(&brush)), 1, 0, 4, 0, &callbacks);
        CGColorSpaceRef grad_colorspace = CGColorSpaceCreateDeviceRGB();
        const QPoint start = painter()->xForm(brush.gradientStart()),
                      stop =  painter()->xForm(brush.gradientStop());
        d->shading = CGShadingCreateAxial(grad_colorspace, CGPointMake(start.x(), start.y()),
                                          CGPointMake(stop.x(), stop.y()), fill_func, true, true);
        CGFunctionRelease(fill_func);
        CGColorSpaceRelease(grad_colorspace);
    } else if(bs != Qt::SolidPattern && bs != Qt::NoBrush) {
        int width = 0, height = 0;
        QMacPattern *qpattern = new QMacPattern;
        float components[4] = { 1.0, 1.0, 1.0, 1.0 };
        CGColorSpaceRef base_colorspace = 0;
        if (bs == Qt::CustomPattern) {
            qpattern->data.pixmap = brush.pixmap();
            if(qpattern->data.pixmap->isQBitmap()) {
                const QColor &col = brush.color();
                components[0] = qt_mac_convert_color_to_cg(col.red());
                components[1] = qt_mac_convert_color_to_cg(col.green());
                components[2] = qt_mac_convert_color_to_cg(col.blue());
                base_colorspace = CGColorSpaceCreateDeviceRGB();
            }
            width = qpattern->data.pixmap->width();
            height = qpattern->data.pixmap->height();
        } else {
            qpattern->as_mask = true;
            qpattern->data.bytes = qt_patternForBrush(bs, false);
            width = height = 8;
            const QColor &col = brush.color();
            components[0] = qt_mac_convert_color_to_cg(col.red());
            components[1] = qt_mac_convert_color_to_cg(col.green());
            components[2] = qt_mac_convert_color_to_cg(col.blue());
            base_colorspace = CGColorSpaceCreateDeviceRGB();
        }
        CGColorSpaceRef fill_colorspace = CGColorSpaceCreatePattern(base_colorspace);
        CGContextSetFillColorSpace(d->hd, fill_colorspace);

        CGPatternCallbacks callbks;
        callbks.version = 0;
        callbks.drawPattern = qt_mac_draw_pattern;
        callbks.releaseInfo = qt_mac_dispose_pattern;
        CGPatternRef fill_pattern = CGPatternCreate(qpattern, CGRectMake(0, 0, width, height),
                                                    CGContextGetCTM(d->hd), width, height,
                                                    kCGPatternTilingNoDistortion, !base_colorspace,
                                                    &callbks);
        CGContextSetFillPattern(d->hd, fill_pattern, components);

        CGPatternRelease(fill_pattern);
        CGColorSpaceRelease(fill_colorspace);
        if(base_colorspace)
            CGColorSpaceRelease(base_colorspace);
    } else if(bs != Qt::NoBrush) {
        const QColor &col = brush.color();
        CGContextSetRGBFillColor(d->hd, qt_mac_convert_color_to_cg(col.red()),
                                 qt_mac_convert_color_to_cg(col.green()),
                                 qt_mac_convert_color_to_cg(col.blue()),
                                 qt_mac_convert_color_to_cg(col.alpha()));
    }
}

void
QCoreGraphicsPaintEngine::updateFont(const QFont &)
{
    Q_ASSERT(isActive());
    clearf(DirtyFont);
    updatePen(d->current.pen);
}

void
QCoreGraphicsPaintEngine::updateBackground(Qt::BGMode mode, const QBrush &brush)
{
    Q_ASSERT(isActive());
    d->current.bg.mode = mode;
    d->current.bg.brush = brush;
}

void
QCoreGraphicsPaintEngine::updateXForm(const QWMatrix &matrix)
{
    Q_ASSERT(isActive());
#if 0
    CGAffineTransform xf = CGAffineTransformInvert(CGContextGetCTM(d->hd));
    xf = CGAffineTransformConcat(xf, CGAffineTransformMake(matrix.m11(), matrix.m12(),
                                                           matrix.m21(), matrix.m22(),
                                                           matrix.dx(),  matrix.dy()));
    CGContextConcatCTM(d->hd, xf);
//    CGContextSetTextMatrix(d->hd, xf);
#else
    Q_UNUSED(matrix);
#endif
}

void
QCoreGraphicsPaintEngine::updateClipRegion(const QRegion &clipRegion, bool clipEnabled)
{
    Q_ASSERT(isActive());
    bool old_clipEnabled = testf(ClipOn);
    if(clipEnabled) {
        d->current.clip = clipRegion;
        setf(ClipOn);
    } else {
        clearf(ClipOn);
    }
    if(clipEnabled || old_clipEnabled)
        setupCGClip(clipEnabled ? &clipRegion : 0);
}

void
QCoreGraphicsPaintEngine::drawLine(const QPoint &p1, const QPoint &p2)
{
    Q_ASSERT(isActive());

    CGContextBeginPath(d->hd);
    CGContextMoveToPoint(d->hd, p1.x(), p1.y());
    CGContextAddLineToPoint(d->hd, p2.x(), p2.y());
    d->drawPath(QCoreGraphicsPaintEnginePrivate::CGStroke);
}

void
QCoreGraphicsPaintEngine::drawRect(const QRect &r)
{
    Q_ASSERT(isActive());

    CGContextBeginPath(d->hd);
    CGRect mac_rect = CGRectMake(r.x(), r.y(), r.width(), r.height());
    CGContextAddRect(d->hd, mac_rect);
    d->drawPath(QCoreGraphicsPaintEnginePrivate::CGFill|QCoreGraphicsPaintEnginePrivate::CGStroke);
}

void
QCoreGraphicsPaintEngine::drawPoint(const QPoint &p)
{
    Q_ASSERT(isActive());

    CGContextBeginPath(d->hd);
    CGContextMoveToPoint(d->hd, p.x(), p.y());
    CGContextAddLineToPoint(d->hd, p.x(), p.y()+1);
    d->drawPath(QCoreGraphicsPaintEnginePrivate::CGStroke);
}

void
QCoreGraphicsPaintEngine::drawPoints(const QPointArray &pa, int index, int npoints)
{
    Q_ASSERT(isActive());

    CGContextBeginPath(d->hd);
    for(int i=0; i<npoints; i++) {
        float x = pa[index+i].x(), y = pa[index+i].y();
        CGContextMoveToPoint(d->hd, x, y);
        CGContextAddLineToPoint(d->hd, x, y+1);
        d->drawPath(QCoreGraphicsPaintEnginePrivate::CGStroke);
    }
}

void
QCoreGraphicsPaintEngine::drawRoundRect(const QRect &r, int xRnd, int yRnd)
{
    Q_ASSERT(isActive());

    CGMutablePathRef path = CGPathCreateMutable();
    const float ow = r.width()*xRnd/200, oh = r.height()*yRnd/200;
    CGAffineTransform transform = CGAffineTransformMake(ow, 0, 0, oh, r.left(), r.top());
    float fw = r.width() / ow, fh = r.height() / oh;
    CGPathMoveToPoint(path, &transform, fw, fh/2);
    CGPathAddArcToPoint(path, &transform, fw, fh, fw/2, fh, 1);
    CGPathAddArcToPoint(path, &transform, 0, fh, 0, fh/2, 1);
    CGPathAddArcToPoint(path, &transform, 0, 0, fw/2, 0, 1);
    CGPathAddArcToPoint(path, &transform, fw, 0, fw, fh/2, 1);
    CGPathCloseSubpath(path);
    CGContextBeginPath(d->hd);
    CGContextAddPath(d->hd, path);
    d->drawPath(QCoreGraphicsPaintEnginePrivate::CGFill|QCoreGraphicsPaintEnginePrivate::CGStroke);
    CGPathRelease(path);
}

void
QCoreGraphicsPaintEngine::drawEllipse(const QRect &r)
{
    Q_ASSERT(isActive());

    CGMutablePathRef path = CGPathCreateMutable();
    CGAffineTransform transform;
    if(r.width() != r.height())
        transform = CGAffineTransformMakeScale(((float)r.width())/r.height(), 1);
    CGPathAddArc(path, r.width() == r.height() ? 0 : &transform,
                 (r.x()+(r.width()/2))/((float)r.width()/r.height()),
                 r.y() + (r.height()/2), r.height()/2, 0, 360, false);
    CGContextBeginPath(d->hd);
    CGContextAddPath(d->hd, path);
    d->drawPath(QCoreGraphicsPaintEnginePrivate::CGFill|QCoreGraphicsPaintEnginePrivate::CGStroke);
    CGPathRelease(path);
}

void
QCoreGraphicsPaintEngine::drawArc(const QRect &r, int a, int alen)
{
    Q_ASSERT(isActive());

    CGMutablePathRef path = CGPathCreateMutable();
    CGAffineTransform transform = CGAffineTransformMake(((float)r.width())/r.height(), 0, 0, -1, 1, (r.y()*2)+r.height());
    float begin_radians = ((float)a/16) * (M_PI/180), end_radians = ((float)((a+alen)/16)) * (M_PI/180);
    CGPathAddArc(path, &transform, (r.x()+(r.width()/2))/((float)r.width()/r.height()), r.y() + (r.height()/2),
		 r.height()/2, begin_radians, end_radians, a < 0 || alen < 0);
    CGContextBeginPath(d->hd);
    CGContextAddPath(d->hd, path);
    d->drawPath(QCoreGraphicsPaintEnginePrivate::CGStroke);
    CGPathRelease(path);
}

void
QCoreGraphicsPaintEngine::drawPie(const QRect &r, int a, int alen)
{
    Q_ASSERT(isActive());

    CGMutablePathRef path = CGPathCreateMutable();
    CGAffineTransform transform = CGAffineTransformMake(((float)r.width())/r.height(), 0, 0, -1, 1, (r.y()*2)+r.height());
    float begin_radians = ((float)a/16) * (M_PI/180), end_radians = ((float)((a+alen)/16)) * (M_PI/180);
    CGPathMoveToPoint(path, 0, r.x() + (r.width()/2), r.y() + (r.height()/2));
    CGPathAddArc(path, &transform, (r.x()+(r.width()/2))/((float)r.width()/r.height()),
                 r.y() + (r.height()/2), r.height()/2, begin_radians, end_radians, a < 0 || alen < 0);
    CGPathAddLineToPoint(path, 0, r.x() + (r.width()/2), r.y() + (r.height()/2));
    CGContextBeginPath(d->hd);
    CGContextAddPath(d->hd, path);
    d->drawPath(QCoreGraphicsPaintEnginePrivate::CGFill|QCoreGraphicsPaintEnginePrivate::CGStroke);
    CGPathRelease(path);
}

void
QCoreGraphicsPaintEngine::drawPolyInternal(const QPointArray &a, bool close)
{
    Q_ASSERT(isActive());

    CGContextMoveToPoint(d->hd, a[0].x(), a[0].y());
    for(int x = 1; x < a.size(); x++)
        CGContextAddLineToPoint(d->hd, a[x].x(), a[x].y());
    if(close)
        CGContextAddLineToPoint(d->hd, a[0].x(), a[0].y());
    d->drawPath(QCoreGraphicsPaintEnginePrivate::CGFill|QCoreGraphicsPaintEnginePrivate::CGStroke);
}

void
QCoreGraphicsPaintEngine::drawChord(const QRect &r, int a, int alen)
{
    Q_ASSERT(isActive());

    CGMutablePathRef path = CGPathCreateMutable();
    CGAffineTransform transform = CGAffineTransformMake(((float)r.width())/r.height(), 0, 0, -1, 1, (r.y()*2)+r.height());
    float begin_radians = ((float)a/16) * (M_PI/180), end_radians = ((float)((a+alen)/16)) * (M_PI/180);
    //We draw twice because the first draw will set the point to the end of arc, and the second pass will draw the line to the first point
    for(int i = 0; i < 2; i++)
        CGPathAddArc(path, &transform, (r.x()+(r.width()/2))/((float)r.width()/r.height()),
                     r.y()+(r.height()/2), r.height()/2, begin_radians, end_radians, a < 0 || alen < 0);
    CGContextBeginPath(d->hd);
    CGContextAddPath(d->hd, path);
    d->drawPath(QCoreGraphicsPaintEnginePrivate::CGFill|QCoreGraphicsPaintEnginePrivate::CGStroke);
    CGPathRelease(path);
}

void
QCoreGraphicsPaintEngine::drawLineSegments(const QPointArray &pa, int index, int nlines)
{
    Q_ASSERT(isActive());

    int  x, y;
    uint i = index;
    while(nlines--) {
        pa.point(i++, &x, &y);
        CGContextMoveToPoint(d->hd, x, y);
        pa.point(i++, &x, &y);
        CGContextAddLineToPoint(d->hd, x, y);
        d->drawPath(QCoreGraphicsPaintEnginePrivate::CGStroke);
    }
}

void
QCoreGraphicsPaintEngine::drawPolyline(const QPointArray &pa, int index, int npoints)
{
    Q_ASSERT(isActive());

    CGContextMoveToPoint(d->hd, pa[index].x(), pa[index].y());
    for(int x = index+1; x < index+npoints; x++)
        CGContextAddLineToPoint(d->hd, pa[x].x(), pa[x].y());
    d->drawPath(QCoreGraphicsPaintEnginePrivate::CGStroke);
}

void
QCoreGraphicsPaintEngine::drawPolygon(const QPointArray &a, bool, int index, int npoints)
{
    Q_ASSERT(isActive());

    QPointArray pa;
    if(index != 0 || npoints != (int)a.size()) {
        pa = QPointArray(npoints);
        for(int i=0; i<npoints; i++)
            pa.setPoint(i, a.point(index+i));
        index = 0;
    } else {
        pa = a;
    }
    drawPolyInternal(pa, true);
}

void
QCoreGraphicsPaintEngine::drawConvexPolygon(const QPointArray &pa, int index, int npoints)
{
    // Implemented in terms of drawPolygon() [no optimization]
    drawPolygon(pa,false,index,npoints);
}

#ifndef QT_NO_BEZIER
void
QCoreGraphicsPaintEngine::drawCubicBezier(const QPointArray &pa, int index)
{
    Q_ASSERT(isActive());

    CGContextMoveToPoint(d->hd, pa[index].x(), pa[index].y());
    CGContextAddCurveToPoint(d->hd, pa[index+1].x(), pa[index+1].y(),
                             pa[index+2].x(), pa[index+2].y(), pa[index+3].x(), pa[index+3].y());
    d->drawPath(QCoreGraphicsPaintEnginePrivate::CGStroke);
}
#endif

void
QCoreGraphicsPaintEngine::drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr, Qt::BlendMode mode)
{
    Q_ASSERT(isActive());
    if(pm.isNull())
        return;

    CGContextSaveGState(d->hd);
    if(pm.isQBitmap() || pm.depth() == 1) {
        const QColor &col = d->current.pen.color();
        CGContextSetRGBFillColor(d->hd, qt_mac_convert_color_to_cg(col.red()),
                                 qt_mac_convert_color_to_cg(col.green()),
                                 qt_mac_convert_color_to_cg(col.blue()),
                                 qt_mac_convert_color_to_cg(col.alpha()));
    }
    QRegion rgn(r); 
    qt_mac_clip_cg(d->hd, rgn, 0);

    const float sx = ((float)r.width())/sr.width(), sy = ((float)r.height())/sr.height();
    CGRect rect = CGRectMake(r.x()-(sr.x()*sx), r.y()-(sr.y()*sy), pm.width()*sx, pm.height()*sy);
    CGImageRef image = qt_mac_create_cgimage(pm, mode);
    HIViewDrawCGImage(d->hd, &rect, image); //HIViews render the way we want anyway, so just use the convenience..
    CGImageRelease(image);

    CGContextRestoreGState(d->hd);
}

void
QCoreGraphicsPaintEngine::initialize()
{
}

void
QCoreGraphicsPaintEngine::cleanup()
{
}

void QCoreGraphicsPaintEngine::setupCGClip(const QRegion *rgn)
{
    if(d->hd) {
        qt_mac_clip_cg_reset(d->hd);
        QPoint mp(0, 0);
        if(d->pdev->devType() == QInternal::Widget) {
            QWidget *w = static_cast<QWidget*>(d->pdev);
            mp = posInWindow(w);
            qt_mac_clip_cg(d->hd, w->d->clippedRegion(), &mp);
        }
        if(paintevent_item *pevent = qt_mac_get_paintevent()) {
            if((*pevent) == d->pdev)
                qt_mac_clip_cg(d->hd, pevent->region(), &mp);
        }
        if(rgn)
            qt_mac_clip_cg(d->hd, *rgn, 0); //already widget relative
    }
}


void
QCoreGraphicsPaintEngine::drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &p)
{
    Q_ASSERT(isActive());

    //save the old state
    CGContextSaveGState(d->hd);
    //setup the pattern
    QMacPattern *qpattern = new QMacPattern;
    qpattern->data.pixmap = &pixmap;
    CGPatternCallbacks callbks;
    callbks.version = 0;
    callbks.drawPattern = qt_mac_draw_pattern;
    callbks.releaseInfo = qt_mac_dispose_pattern;
    const int width = pixmap.width(), height = pixmap.height();
    CGPatternRef pat = CGPatternCreate(qpattern, CGRectMake(0, 0, width, height), CGContextGetCTM(d->hd), width, height,
                                       kCGPatternTilingNoDistortion, true, &callbks);
    CGColorSpaceRef cs = CGColorSpaceCreatePattern(0);
    CGContextSetFillColorSpace(d->hd, cs);
    float component = 1.0; //just one
    CGContextSetFillPattern(d->hd, pat, &component);
    CGContextSetPatternPhase(d->hd, CGSizeMake(p.x()-r.x(), p.y()-r.y()));
    //fill the rectangle
    CGRect mac_rect = CGRectMake(r.x(), r.y(), r.width(), r.height());
    CGContextFillRect(d->hd, mac_rect);
    //restore the state
    CGContextRestoreGState(d->hd);
    //cleanup
    CGColorSpaceRelease(cs);
    CGPatternRelease(pat);
}

QPainter::RenderHints QCoreGraphicsPaintEngine::supportedRenderHints() const
{
    return QPainter::RenderHints(QPainter::LineAntialiasing | QPainter::TextAntialiasing);
}

void QCoreGraphicsPaintEngine::updateRenderHints(QPainter::RenderHints hints)
{
    CGContextSetShouldAntialias(d->hd, hints & QPainter::LineAntialiasing);
    CGContextSetShouldSmoothFonts(d->hd, hints & QPainter::TextAntialiasing);
}

/****************************************************************************
**
** Definition of QQuickDrawGC/QCoreGraphicsGC  class.
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

#include "qgc_mac.h"
#include "qwidget.h"
#include "qbitmap.h"
#include "qpixmapcache.h"
#include "qfontdata_p.h"
#include "qtextcodec.h"
#include "qpaintdevicemetrics.h"
#include "qpaintdevice.h"
#include "qt_mac.h"
#include <qtextcodec.h>
#include <qstack.h>
#include <qprinter.h>
#include <private/qgc_mac_p.h>
#include <private/qpainter_p.h>
#include <private/qfontengine_p.h>
#include <private/qtextlayout_p.h>
#include <string.h>

/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/
QAbstractGC *qt_mac_current_gc = 0; //Current "active" QGC


/*****************************************************************************
  External functions
 *****************************************************************************/
QPoint posInWindow(QWidget *w); //qwidget_mac.cpp
bool qt_recreate_root_win(); //qwidget_mac.cpp
QRegion make_region(RgnHandle handle);
void qt_mac_clip_cg_handle(CGContextRef, const QRegion &, const QPoint &, bool); //qpaintdevice_mac.cpp
void unclippedBitBlt(QPaintDevice *dst, int dx, int dy, const QPaintDevice *src, int sx, int sy, int sw, int sh,
		     Qt::RasterOp rop, bool imask, bool set_fore_colour); //qpaintdevice_mac.cpp
RgnHandle qt_mac_get_rgn(); //qregion_mac.cpp
void qt_mac_dispose_rgn(RgnHandle r); //qregion_mac.cpp

// paintevent magic to provide Windows semantics on Qt/Mac
class paintevent_item
{
    QWidget *clippedTo;
    QPaintDevice* dev;
    QRegion clipRegion;
public:
    paintevent_item(QPaintDevice *d, QRegion r, QWidget *c) : clippedTo(c), dev(d), clipRegion(r) { }
    inline bool operator==(const QPaintDevice *rhs) const { return rhs == dev; }
    inline bool operator!=(const QPaintDevice *rhs) const { return !(this->operator==(rhs)); }
    inline QWidget *clip() const { return clippedTo; }
    inline QPaintDevice *device() const { return dev; }
    inline QRegion region() const { return clipRegion; }
};
QStack<paintevent_item*> paintevents;
static paintevent_item *qt_mac_get_paintevent() { return paintevents.isEmpty() ? 0 : paintevents.top(); }

void qt_set_paintevent_clipping(QPaintDevice* dev, const QRegion& region, QWidget *clip)
{
    QRegion r = region;
    if(dev && dev->devType() == QInternal::Widget) {
	QWidget *w = (QWidget *)dev;
	QPoint mp(posInWindow(w));
	r.translate(mp.x(), mp.y());
    }
    if(paintevent_item *curr = qt_mac_get_paintevent()) {
	if(curr->device() == dev || curr->clip() == dev || clip == curr->clip()
	   || curr->device() == clip)
	    r &= curr->region();
    }
    paintevents.push(new paintevent_item(dev, r, clip));
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
  QQuickDrawGC member functions
 *****************************************************************************/
QQuickDrawGC::QQuickDrawGC(const QPaintDevice *pdev)
{
    d = new QQuickDrawGCPrivate;
    d->pdev = const_cast<QPaintDevice*>(pdev);
}

QQuickDrawGC::~QQuickDrawGC()
{
    delete d;
}

bool
QQuickDrawGC::begin(const QPaintDevice *pdev, QPainterState *ps, bool unclipped)
{
    if(isActive()) {                         // already active painting
        qWarning( "QQuickDrawGC::begin: Painter is already active."
                  "\n\tYou must end() the painter before a second begin()" );
	return false;
    }
    if(pdev->devType() == QInternal::Widget &&
       !static_cast<const QWidget*>(pdev)->testWState(WState_InPaintEvent)) {
	qWarning("QQuickDrawGC::begin: Widget painting can only begin as a "
		 "result of a paintEvent");
//	return false;
    }

    //save the gworld now, we'll reset it in end()
    d->saved = new QMacSavedPortInfo;

    d->pdev = const_cast<QPaintDevice*>(pdev);
    setActive(true);
    assignf(IsActive | DirtyFont);

    if(d->pdev->devType() == QInternal::Pixmap)         // device is a pixmap
	((QPixmap*)d->pdev)->detach();             // will modify it

    d->clip.serial = 0;
    d->paintevent = 0;
    d->clip.dirty = false;
    d->offx = d->offy = 0;

    if(d->pdev->devType() == QInternal::Widget) {                    // device is a widget
	QWidget *w = (QWidget*)d->pdev;
	{ //offset painting in widget relative the tld
	    QPoint wp(posInWindow(w));
	    d->offx = wp.x();
	    d->offy = wp.y();
	}
	if(!unclipped)
	    unclipped = (bool)w->testWFlags(WPaintUnclipped);

	if(!d->locked) {
	    LockPortBits(GetWindowPort((WindowPtr)w->handle()));
	    d->locked = true;
	}

	if(w->isDesktop()) {
	    if(!unclipped)
		qWarning("QPainter::begin: Does not support clipped desktop on MacOSX");
	    ShowWindow((WindowPtr)w->handle());
	}
    } else if(d->pdev->devType() == QInternal::Pixmap) {             // device is a pixmap
	QPixmap *pm = (QPixmap*)d->pdev;
	if(pm->isNull()) {
	    qWarning("QPainter::begin: Cannot paint null pixmap");
	    end();
	    return false;
	}
#ifndef QMAC_ONE_PIXEL_LOCK
	if(!d->locked) {
	    bool locked = LockPixels(GetGWorldPixMap((GWorldPtr)pm->handle()));
	    Q_ASSERT(locked);
	    d->locked = true;
	}
#endif
    }
    d->unclipped = unclipped;
#ifndef USE_CORE_GRAPHICS
    setupQDPort(true); //force setting paint device, this does unclipped fu
#endif

    updateXForm(ps);
    updateBrush(ps);
    updatePen(ps);
    updateClipRegion(ps);
    return true;
}

bool
QQuickDrawGC::end()
{
    setActive(false);

    if(d->locked) {
	if(d->pdev->devType() == QInternal::Widget)
	    UnlockPortBits(GetWindowPort((WindowPtr)d->pdev->handle()));
#ifndef QMAC_ONE_PIXEL_LOCK
	else
	    UnlockPixels(GetGWorldPixMap((GWorldPtr)d->pdev->handle()));
#endif
	d->locked = false;
    }

    delete d->saved;
    d->saved = 0;
    if(qt_mac_current_gc == this)
	qt_mac_current_gc = 0;

    if(d->pdev->devType() == QInternal::Widget && ((QWidget*)d->pdev)->isDesktop())
	HideWindow((WindowPtr)d->pdev->handle());

    d->pdev = 0;
    return true;
}

void
QQuickDrawGC::updatePen(QPainterState *ps)
{
    d->current.pen = ps->pen;
}


void
QQuickDrawGC::updateBrush(QPainterState *ps)
{
    d->current.brush = ps->brush;
}

void
QQuickDrawGC::updateFont(QPainterState *ps)
{
    d->current.font = ps->font;
    clearf(DirtyFont);
}


void
QQuickDrawGC::updateRasterOp(QPainterState *ps)
{
    Q_ASSERT(isActive());
    d->current.rop = ps->rasterOp;
}

void
QQuickDrawGC::updateBackground(QPainterState *ps)
{
    Q_ASSERT(isActive());
    d->current.bg.origin = ps->bgOrigin;
    d->current.bg.mode = ps->bgMode;
    d->current.bg.color = ps->bgColor;
}

void
QQuickDrawGC::updateXForm(QPainterState */*ps*/)
{
}

void
QQuickDrawGC::setClippedRegionInternal(QRegion *rgn)
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
QQuickDrawGC::updateClipRegion(QPainterState *ps)
{
    Q_ASSERT(isActive());
    setClippedRegionInternal(ps->clipEnabled ? &ps->clipRegion : 0);
}

void QQuickDrawGC::setRasterOp(RasterOp r)
{
    Q_ASSERT(isActive());
    if ((uint)r > LastROP) {
        qWarning("QX11GC::setRasterOp: Invalid ROP code");
        return;
    }
    d->current.rop = r;
}

void
QQuickDrawGC::drawLine(const QPoint &pt1, const QPoint &pt2)
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
QQuickDrawGC::drawRect(const QRect &r)
{
    Q_ASSERT(isActive());
    setupQDPort();
    if(d->clip.paintable.isEmpty())
	return;

    Rect rect;
    SetRect(&rect, r.x()+d->offx, r.y()+d->offy, r.right()+d->offx, r.bottom()+d->offy);
    if(d->current.brush.style() != NoBrush) {
	setupQDBrush();
	if(d->current.brush.style() == SolidPattern) {
	    PaintRect(&rect);
	} else {
	    QPixmap *pm = 0;
	    if(d->current.brush.style() == QBrush::CustomPattern) {
		pm = d->current.brush.pixmap();
	    } else {
		qDebug("%d: a brush style pix..", __LINE__);
		pm = d->brush_style_pix;
		if(d->current.bg.mode == OpaqueMode) {
		    ::RGBColor f;
		    f.red = d->current.bg.color.red()*256;
		    f.green = d->current.bg.color.green()*256;
		    f.blue = d->current.bg.color.blue()*256;
		    RGBForeColor(&f);
		    PaintRect(&rect);
		}
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
		drawTiledPixmap(r, *pm, r.topLeft() - d->current.bg.origin, true);

		//restore the clip
		setClippedRegionInternal(clipon ? &clip : 0);
	    }
	}
    }
    if(d->current.pen.style() != NoPen) {
	setupQDPen();
	FrameRect(&rect);
    }
}

void
QQuickDrawGC::drawPoint(const QPoint &pt)
{
    Q_ASSERT(isActive());
    if(d->current.pen.style() != NoPen) {
	setupQDPort();
	if(d->clip.paintable.isEmpty())
	    return;
	setupQDPen();
	MoveTo(pt.x() + d->offx, pt.y() + d->offy);
	Line(0,1);
    }
}

void
QQuickDrawGC::drawPoints(const QPointArray &pa, int index, int npoints)
{
    Q_ASSERT(isActive());

    if(d->current.pen.style() != NoPen) {
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
QQuickDrawGC::drawWinFocusRect(const QRect &fr, bool xorPaint, const QColor &bgColor)
{
    Q_ASSERT(isActive());

    //save
    QPen    old_pen = d->current.pen;
    RasterOp old_rop = (RasterOp)d->current.rop;

    //setup
    if(xorPaint) {
	if(QColor::numBitPlanes() <= 8)
	    d->current.pen = QPen(color1);
	else
	    d->current.pen = QPen(white);
	d->current.rop = XorROP;
    } else {
	if(qGray(bgColor.rgb()) < 128)
	    d->current.pen = QPen(white);
	else
	    d->current.pen = QPen(black);
    }
    d->current.pen.setStyle(DashLine);

    //draw
    setupQDPen();
    if(d->current.pen.style() != NoPen)
	drawRect(fr);

    //restore
    d->current.rop = old_rop;
    d->current.pen = old_pen;
}

void
QQuickDrawGC::drawRoundRect(const QRect &r, int xRnd, int yRnd)
{
    Q_ASSERT(isActive());

    setupQDPort();
    if(d->clip.paintable.isEmpty())
	return;

    Rect rect;
    SetRect(&rect, r.x()+d->offx, r.y()+d->offy, r.right()+d->offx, r.bottom()+d->offy);
    if(d->current.brush.style() == SolidPattern) {
	setupQDBrush();
	PaintRoundRect(&rect, r.width()*xRnd/100, r.height()*yRnd/100);
    }
    if(d->current.pen.style() != NoPen) {
	setupQDPen();
	FrameRoundRect(&rect, r.width()*xRnd/100, r.height()*yRnd/100);
    }
}

void
QQuickDrawGC::drawPolyInternal(const QPointArray &pa, bool close, bool inset)
{
    Q_ASSERT(isActive());

    setupQDPort();
    if(d->clip.paintable.isEmpty())
	return;

    RgnHandle polyRegion = qt_mac_get_rgn();
    OpenRgn();
    MoveTo(pa[0].x()+d->offx, pa[0].y()+d->offy);
    for(int x = 1; x < pa.size(); x++)
	LineTo(pa[x].x()+d->offx, pa[x].y()+d->offy);
    if(close)
	LineTo(pa[0].x()+d->offx, pa[0].y()+d->offy);
    CloseRgn(polyRegion);

    if(close && d->current.brush.style() != NoBrush) {
	setupQDBrush();
	if(inset && d->current.pen.style() == NoPen)  // Inset all points, no frame will be painted.
	    InsetRgn(polyRegion, 1, 1);
	if(d->current.brush.style() == SolidPattern) {
	    PaintRgn(polyRegion);
	} else {
	    QPixmap *pm = 0;
	    if(d->current.brush.style() == QBrush::CustomPattern) {
		pm = d->current.brush.pixmap();
	    } else {
		qDebug("%d: a brush style pix..", __LINE__);
		pm = d->brush_style_pix;
		if(d->current.bg.mode == OpaqueMode) {
		    ::RGBColor f;
		    f.red = d->current.bg.color.red()*256;
		    f.green = d->current.bg.color.green()*256;
		    f.blue = d->current.bg.color.blue()*256;
		    RGBForeColor(&f);
		    PaintRgn(polyRegion);
		}
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
		drawTiledPixmap(r, *pm, r.topLeft() - d->current.bg.origin, true);
		
		//restore the clip
		setClippedRegionInternal(clipon ? &clip : 0);
	    }
	}
    }
    if(d->current.pen.style() != NoPen) {
	setupQDPen();
	FrameRgn(polyRegion);
    }
    qt_mac_dispose_rgn(polyRegion);
}

void
QQuickDrawGC::drawEllipse(const QRect &r)
{
    Q_ASSERT(isActive());

    setupQDPort();
    if(d->clip.paintable.isEmpty())
	return;

    Rect mac_r;
    SetRect(&mac_r, r.x()+d->offx, r.y()+d->offy, r.right()+d->offx, r.bottom()+d->offy);
    if(d->current.brush.style() != NoBrush) {
	setupQDBrush();
	if(d->current.brush.style() == SolidPattern) {
	    PaintOval(&mac_r);
	} else {
	    QPixmap *pm = 0;
	    if(d->current.brush.style() == QBrush::CustomPattern) {
		pm = d->current.brush.pixmap();
	    } else {
		qDebug("%d: a brush style pix..", __LINE__);
		pm = d->brush_style_pix;
		if(d->current.bg.mode == OpaqueMode) {
		    ::RGBColor f;
		    f.red = d->current.bg.color.red()*256;
		    f.green = d->current.bg.color.green()*256;
		    f.blue = d->current.bg.color.blue()*256;
		    RGBForeColor(&f);
		    PaintOval(&mac_r);
		}
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
		drawTiledPixmap(r, *pm, r.topLeft() - d->current.bg.origin, true);

		//restore the clip
		setClippedRegionInternal(clipon ? &clip : 0);
	    }
	}
    }

    if(d->current.pen.style() != NoPen) {
	setupQDPen();
	FrameOval(&mac_r);
    }
}

void
QQuickDrawGC::drawChord(const QRect &r, int a, int alen)
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
QQuickDrawGC::drawLineSegments(const QPointArray &pa, int index, int nlines)
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
QQuickDrawGC::drawPolyline(const QPointArray &pa, int index, int npoints)
{
    Q_ASSERT(isActive());
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
	plot_pixel = d->current.pen.style() == SolidLine; // plot last pixel
    }
    setupQDPort();
    if(d->clip.paintable.isEmpty())
	return;
    //make a region of it
    PolyHandle poly = OpenPoly();
    MoveTo(pa[index].x()+d->offx, pa[index].y()+d->offy);
    for(int x = index+1; x < npoints; x++)
	LineTo(pa[x].x()+d->offx, pa[x].y()+d->offy);
    ClosePoly();
    //now draw it
    setupQDPen();
    FramePoly(poly);
    KillPoly(poly);
}

void
QQuickDrawGC::drawPolygon(const QPointArray &a, bool /*winding*/, int index, int npoints)
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
QQuickDrawGC::drawConvexPolygon(const QPointArray &pa, int index, int npoints)
{
    // Implemented in terms of drawPolygon() [no optimization]
    drawPolygon(pa,false,index,npoints);
}

#ifndef QT_NO_BEZIER
void
QQuickDrawGC::drawCubicBezier(const QPointArray &pa, int index)
{
    Q_ASSERT(isActive());
    drawPolyline(pa.cubicBezier(), index);
}
#endif

void
QQuickDrawGC::drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &p, bool)
{
    int yPos=r.y(), xPos, drawH, drawW, yOff=p.y(), xOff;
    while(yPos < r.bottom()) {
	drawH = pixmap.height() - yOff;    // Cropping first row
	if(yPos + drawH > r.right())        // Cropping last row
	    drawH = r.bottom() - yPos;
	xPos = r.x();
	xOff = p.x();
	while(xPos < r.right()) {
	    drawW = pixmap.width() - xOff; // Cropping first column
	    if(xPos + drawW > r.right())    // Cropping last column
		drawW = r.right() - xPos;
	    drawPixmap(QRect(xPos, yPos, drawW, drawH), pixmap, QRect(xOff, yOff, drawW, drawH));
	    xPos += drawW;
	    xOff = 0;
	}
	yPos += drawH;
	yOff = 0;
    }
}

void
QQuickDrawGC::drawPie(const QRect &r, int a, int alen)
{
    Q_ASSERT(isActive());

    QPointArray pa;
    pa.makeArc(r.x(), r.y(), r.width(), r.height(), a, alen); // arc polyline
    int n = pa.size();
    pa.resize(n+2);
    pa.setPoint(n, r.right()/2, r.bottom()/2);	// add legs
    pa.setPoint(n+1, pa.at(0));
    drawPolyInternal(pa, true, false);
}

void
QQuickDrawGC::drawArc(const QRect &r, int a, int alen)
{
    Q_ASSERT(isActive());

    QPointArray pa;
    pa.makeArc(r.x(), r.y(), r.width(), r.height(), a, alen); // arc polyline
    drawPolyline(pa);
}

void
QQuickDrawGC::drawPixmap(const QRect &r, const QPixmap &pixmap, const QRect &sr)
{
    Q_ASSERT(isActive());
    if(pixmap.isNull())
	return;

    setupQDPort();
    if(d->clip.paintable.isEmpty())
	return;
    setupQDPen();
#ifdef USE_CORE_GRAPHICS
    QMacSavedPortInfo::setClipRegion(d->clip.paintable);
#endif
    unclippedBitBlt(d->pdev, r.x(), r.y(), &pixmap, sr.x(), sr.y(), sr.width(), sr.height(), (RasterOp)d->current.rop, false, false);
}

void
QQuickDrawGC::drawTextItem(const QPoint &p, const QTextItem &ti, int textflags)
{
#if 0
    Q_ASSERT(isActive());

    QTextEngine *engine = ti.engine();
    QScriptItem *si = &engine->items[ti.item()];
    engine->shape(ti.item());
    QFontEngine *fe = si->fontEngine;

    Q_ASSERT(fe);
    fe->draw(this, p.x() + si->x,  p.y() + si->y, engine, si, textflags);
#else
    qDebug("Must implement drawTextItem!!");
#endif
}

Qt::HANDLE
QQuickDrawGC::handle() const
{
    return d->pdev->handle();
}

void
QQuickDrawGC::initialize()
{
}

void 
QQuickDrawGC::cleanup()
{
}

/*!
    \internal
*/
void
QQuickDrawGC::setupQDPen()
{
    //pen color
    ::RGBColor f;
    f.red = d->current.pen.color().red()*256;
    f.green = d->current.pen.color().green()*256;
    f.blue = d->current.pen.color().blue()*256;
    RGBForeColor(&f);

    //pen size
    int dot = d->current.pen.width();
    if(dot < 1)
	dot = 1;
    PenSize(dot, dot);

    int	ps = d->current.pen.style();
    Pattern pat;
    switch(ps) {
	case DotLine:
	case DashDotLine:
	case DashDotDotLine:
//	    qDebug("Penstyle not implemented %s - %d", __FILE__, __LINE__);
	case DashLine:
	    GetQDGlobalsGray(&pat);
	    break;
	default:
	    GetQDGlobalsBlack(&pat);
	    break;
    }
    PenPat(&pat);

    //penmodes
    //Throw away a desktop when you paint into it non copy mode (xor?) I do this because
    //xor doesn't really work on an overlay widget FIXME
    if(d->current.rop != CopyROP && d->pdev->devType() == QInternal::Widget && ((QWidget *)d->pdev)->isDesktop())
	qt_recreate_root_win();
    PenMode(ropCodes[d->current.rop]);
}

/*!
    \internal
*/
void
QQuickDrawGC::setupQDBrush()
{
    static uchar dense1_pat[] = { 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff };
    static uchar dense2_pat[] = { 0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff };
    static uchar dense3_pat[] = { 0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee };
    static uchar dense4_pat[] = { 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa };
    static uchar dense5_pat[] = { 0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11 };
    static uchar dense6_pat[] = { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 };
    static uchar dense7_pat[] = { 0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00 };
    static uchar hor_pat[] = {			// horizontal pattern
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    static uchar ver_pat[] = {			// vertical pattern
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20 };
    static uchar cross_pat[] = {			// cross pattern
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0xff, 0xff, 0xff,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0xff, 0xff, 0xff, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0xff, 0xff, 0xff,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0xff, 0xff, 0xff, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20 };
    static uchar bdiag_pat[] = {			// backward diagonal pattern
	0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04, 0x02, 0x02, 0x01, 0x01,
	0x80, 0x80, 0x40, 0x40, 0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04,
	0x02, 0x02, 0x01, 0x01, 0x80, 0x80, 0x40, 0x40 };
    static uchar fdiag_pat[] = {			// forward diagonal pattern
	0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40,
	0x80, 0x80, 0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10,
	0x20, 0x20, 0x40, 0x40, 0x80, 0x80, 0x01, 0x01 };
    static uchar dcross_pat[] = {			// diagonal cross pattern
	0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14, 0x22, 0x22, 0x41, 0x41,
	0x80, 0x80, 0x41, 0x41, 0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14,
	0x22, 0x22, 0x41, 0x41, 0x80, 0x80, 0x41, 0x41 };
    static uchar *pat_tbl[] = {
	dense1_pat, dense2_pat, dense3_pat, dense4_pat, dense5_pat, dense6_pat, dense7_pat,
	hor_pat, ver_pat, cross_pat, bdiag_pat, fdiag_pat, dcross_pat };

    //Throw away a desktop when you paint into it non copy mode (xor?) I do this because
    //xor doesn't really work on an overlay widget FIXME
    if(d->current.rop != CopyROP && d->pdev->devType() == QInternal::Widget && ((QWidget *)d->pdev)->isDesktop())
	qt_recreate_root_win();

    //color
    ::RGBColor f;
    f.red = d->current.brush.color().red()*256;
    f.green = d->current.brush.color().green()*256;
    f.blue = d->current.brush.color().blue()*256;
    RGBForeColor(&f);

    d->brush_style_pix = 0;
    int bs = d->current.brush.style();
    if(bs >= Dense1Pattern && bs <= DiagCrossPattern) {

#if 1
	::RGBColor f;
	f.green = f.blue = 192*256;
	f.red = 256*256;
	RGBForeColor(&f);
#endif

	QString key;
	key.sprintf("$qt-brush$%d", bs);
	d->brush_style_pix = QPixmapCache::find(key);
	if(!d->brush_style_pix) {                        // not already in pm dict
	    uchar *pat=pat_tbl[bs-Dense1Pattern];
	    int size = 16;
	    if(bs<=Dense7Pattern)
		size=8;
	    else if(bs<=CrossPattern)
		size=24;
	    d->brush_style_pix = new QPixmap(size, size);
	    d->brush_style_pix->setMask(QBitmap(size, size, pat, false));
	    QPixmapCache::insert(key, d->brush_style_pix);
	}
	d->brush_style_pix->fill(d->current.brush.color());
    }

    //penmodes
    PenMode(ropCodes[d->current.rop]);
}

/*!
    \internal
*/
void
QQuickDrawGC::setupQDFont()
{
    setupQDPen();
    d->current.font.macSetFont(d->pdev);
}

/*!
    \internal
*/
void QQuickDrawGC::setupQDPort(bool force, QPoint *off, QRegion *rgn)
{
    bool remade_clip = false;
    if(d->pdev->devType() == QInternal::Printer) {
	if(force && d->pdev->handle()) {
	    remade_clip = true;
	    d->clip.pdev = QRegion(0, 0, d->pdev->metric(QPaintDeviceMetrics::PdmWidth),
					  d->pdev->metric(QPaintDeviceMetrics::PdmHeight));
	}
    } else if(d->pdev->devType() == QInternal::Widget) {                    // device is a widget
	paintevent_item *pevent = qt_mac_get_paintevent();
	if(pevent && (*pevent) != d->pdev)
	    pevent = 0;
	QWidget *w = (QWidget*)d->pdev, *clip = w;
	if(pevent && pevent->clip())
	    clip = pevent->clip();
	if(!(remade_clip = force)) {
	    if(pevent != d->paintevent)
		remade_clip = true;
	    else if(!clip->isVisible())
		remade_clip = d->clip.serial;
	    else
		remade_clip = (d->clip.serial != clip->clippedSerial(!d->unclipped));
	}
	if(remade_clip) {
	    //offset painting in widget relative the tld
	    QPoint wp(posInWindow(w));
	    d->offx = wp.x();
	    d->offy = wp.y();

	    if(!clip->isVisible()) {
		d->clip.pdev = QRegion(0, 0, 0, 0); //make the clipped reg empty if not visible!!!
		d->clip.serial = 0;
	    } else {
		d->clip.pdev = clip->clippedRegion(!d->unclipped);
		d->clip.serial = clip->clippedSerial(!d->unclipped);
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
    if(remade_clip || d->clip.dirty) { 	//update clipped region
	remade_clip = true;
	if(!d->clip.pdev.isEmpty() && testf(ClipOn)) {
	    d->clip.paintable = d->current.clip;
	    d->clip.paintable.translate(d->offx, d->offy);
	    d->clip.paintable &= d->clip.pdev;
	} else {
	    d->clip.paintable = d->clip.pdev;
	}

	CGrafPtr ptr;
	if(d->pdev->devType() == QInternal::Widget)
	    ptr = GetWindowPort((WindowPtr)d->pdev->handle());
	else
	    ptr = (GWorldPtr)d->pdev->handle();
	if(RgnHandle rgn = d->clip.paintable.handle()) {
	    QDAddRegionToDirtyRegion(ptr, rgn);
	} else {
	    QRect qr = d->clip.paintable.boundingRect();
	    Rect mr; SetRect(&mr, qr.x(), qr.y(), qr.right(), qr.bottom());
	    QDAddRectToDirtyRegion(ptr, &mr);
	}
	d->clip.dirty = false;
    }
    if(remade_clip || qt_mac_current_gc != this) {
	QMacSavedPortInfo::setPaintDevice(d->pdev);
#ifndef USE_CORE_GRAPHICS
	QMacSavedPortInfo::setClipRegion(d->clip.paintable);
#endif
	qt_mac_current_gc = this;
    }
    if(off)
	*off = QPoint(d->offx, d->offy);
    if(rgn)
	*rgn = d->clip.paintable;
}

#ifdef USE_CORE_GRAPHICS

/*****************************************************************************
  QCoreGraphicsGC utility functions
 *****************************************************************************/

inline static float qt_mac_convert_color_to_cg(int c) { return ((float)c * 1000 / 255) / 1000; }

struct QMacPattern {
    bool as_mask;
    union {
	QPixmap *pixmap;
	struct {
	    uchar *bytes;
	    uint byte_per_line;
	} mask;
    };
    struct ImageConv {
	CGImageRef image;
	CGColorSpaceRef colorspace;
	CGDataProviderRef provider;
    } *im;
};
static void qt_mac_draw_pattern(void *info, CGContextRef c)
{
    QMacPattern *pat = (QMacPattern*)info;
    int w = 0, h = 0;
    if(pat->as_mask) {
	qDebug("Completely untested!!"); //need to test patterns
	w = h = pat->mask.byte_per_line;
	if(!pat->im) {
	    pat->im = new QMacPattern::ImageConv;
	    pat->im->colorspace = CGColorSpaceCreateDeviceRGB();
	    pat->im->provider = CGDataProviderCreateWithData(0, pat->mask.bytes, w, 0);
	    pat->im->image = CGImageCreate(w, h, 8, 1, w, pat->im->colorspace, kCGImageAlphaOnly,
					   pat->im->provider, 0, 0, kCGRenderingIntentDefault);
	}
    } else {
	w = pat->pixmap->width();
	h = pat->pixmap->height();
	if(!pat->im) {
	    pat->im = new QMacPattern::ImageConv;
	    pat->im->colorspace = CGColorSpaceCreateDeviceRGB();
	    uint bpl = GetPixRowBytes(GetGWorldPixMap((GWorldPtr)pat->pixmap->handle()));
	    pat->im->provider = CGDataProviderCreateWithData(0, GetPixBaseAddr(GetGWorldPixMap((GWorldPtr)pat->pixmap->handle())), bpl, 0);
	    pat->im->image = CGImageCreate(w, h, 8, pat->pixmap->depth(), bpl, pat->im->colorspace, kCGImageAlphaNoneSkipFirst,
					   pat->im->provider, 0, 0, kCGRenderingIntentDefault);
	}
    }
    CGRect rect = CGRectMake(0, 0, w, h);
#if 1
    /* For whatever reason HIViews are top, left - so we'll just use this convenience function to
       actually render the CGImageRef. If this proves not to be an efficent function call (I doubt
       it), we'll just flip the image in the conversion above. */
    HIViewDrawCGImage(c, &rect, pat->im->image);
#else
    CGContextDrawImage(c, rect, pat->im->image);
#endif
}

static void qt_mac_dispose_pattern(void *info)
{
    QMacPattern *pat = (QMacPattern*)info;
    if(pat->im) {
	if(pat->im->image)
	    CGImageRelease(pat->im->image);
	if(pat->im->colorspace)
	    CGColorSpaceRelease(pat->im->colorspace);
	if(pat->im->provider)
	    CGDataProviderRelease(pat->im->provider);
    }
    if(!pat->as_mask)
	delete pat->pixmap;
    delete pat;
    pat = NULL;
}

static inline bool qt_mac_update_cg(QCoreGraphicsGCPrivate *paint_d)
{
    CGContextRef ret = 0;
    if(paint_d->pdev->devType() == QInternal::Widget)
	ret = (CGContextRef)((QWidget*)paint_d->pdev)->macCGHandle(!paint_d->unclipped);
    else
	ret = (CGContextRef)paint_d->pdev->macCGHandle();
    //apply paint event region (in global coords)
    if(paintevent_item *pevent = qt_mac_get_paintevent()) {
	if((*pevent) == paint_d->pdev)
	    qt_mac_clip_cg_handle(ret, pevent->region(), QPoint(0, 0), true);
    }
    paint_d->hd = ret;
    return ret != 0;
}

/*****************************************************************************
  QCoreGraphicsGC member functions
 *****************************************************************************/

QCoreGraphicsGC::QCoreGraphicsGC(const QPaintDevice *pdev)
{
    d = new QCoreGraphicsGCPrivate;
    d->pdev = const_cast<QPaintDevice*>(pdev);
}

QCoreGraphicsGC::~QCoreGraphicsGC()
{
    delete d;
}

bool
QCoreGraphicsGC::begin(const QPaintDevice *pdev, QPainterState *state, bool unclipped)
{
    if(isActive()) {                         // already active painting
        qWarning( "QQuickDrawGC::begin: Painter is already active."
                  "\n\tYou must end() the painter before a second begin()" );
	return false;
    }
    if(pdev->devType() == QInternal::Widget &&
       !static_cast<const QWidget*>(pdev)->testWState(WState_InPaintEvent)) {
	qWarning("QQuickDrawGC::begin: Widget painting can only begin as a "
		 "result of a paintEvent");
//	return false;
    }

    d->pdev = const_cast<QPaintDevice*>(pdev);
    if(qt_mac_update_cg(d)) // get handle to drawable
	CGContextRetain((CGContextRef)d->hd);
    setActive(true);
    assignf(IsActive | DirtyFont);

    if(d->pdev->devType() == QInternal::Pixmap)         // device is a pixmap
	((QPixmap*)d->pdev)->detach();             // will modify it

    d->fill_pattern = 0;
    d->offx = d->offy = 0;

    if(d->pdev->devType() == QInternal::Widget) {                    // device is a widget
	QWidget *w = (QWidget*)d->pdev;
	{ //offset painting in widget relative the tld
	    QPoint wp(posInWindow(w));
	    d->offx = wp.x();
	    d->offy = wp.y();
	}
	if(!unclipped)
	    unclipped = (bool)w->testWFlags(WPaintUnclipped);

	if(w->isDesktop()) {
	    if(!unclipped)
		qWarning("QPainter::begin: Does not support clipped desktop on MacOSX");
	    ShowWindow((WindowPtr)w->handle());
	}
    } else if(d->pdev->devType() == QInternal::Pixmap) {             // device is a pixmap
	QPixmap *pm = (QPixmap*)d->pdev;
	if(pm->isNull()) {
	    qWarning("QPainter::begin: Cannot paint null pixmap");
	    end();
	    return false;
	}
    }
    d->unclipped = unclipped;

    updateXForm(state);
    updateBrush(state);
    updatePen(state);
    updateClipRegion(state);
    return true;
}

bool
QCoreGraphicsGC::end()
{
    setActive(false);
    if(d->fill_pattern) {
        CGPatternRelease(d->fill_pattern);
	d->fill_pattern = 0;
    }
    if(d->fill_colorspace) {
	CGColorSpaceRelease(d->fill_colorspace);
	d->fill_colorspace = 0;
    }
    if(d->hd) {
	CGContextSynchronize((CGContextRef)d->hd);
	if(CFGetRetainCount(d->hd) == 1)
	    CGContextFlush((CGContextRef)d->hd);
	CGContextRelease((CGContextRef)d->hd);
	d->hd = 0;
    }
    if(d->pdev->devType() == QInternal::Widget && ((QWidget*)d->pdev)->isDesktop())
	HideWindow((WindowPtr)d->pdev->handle());
    d->pdev = 0;
    return true;
}

void
QCoreGraphicsGC::updatePen(QPainterState *ps)
{
    Q_ASSERT(isActive());
    d->current.pen = ps->pen;

    float *lengths = NULL;
    int count = 0;
    if(ps->pen.style() == DashLine) {
	static float inner_lengths[] = { 3, 1 };
	lengths = inner_lengths;
	count = sizeof(sizeof(inner_lengths) / sizeof(inner_lengths[0]));
    } else if(ps->pen.style() == DotLine) {
	static float inner_lengths[] = { 1, 1 };
	lengths = inner_lengths;
	count = sizeof(sizeof(inner_lengths) / sizeof(inner_lengths[0]));
    } else if(ps->pen.style() == DashDotLine) {
	static float inner_lengths[] = { 3, 1, 1, 1 };
	lengths = inner_lengths;
	count = sizeof(sizeof(inner_lengths) / sizeof(inner_lengths[0]));
    } else if(ps->pen.style() == DashDotDotLine) {
	static float inner_lengths[] = { 3, 1, 1, 1, 1, 1 };
	lengths = inner_lengths;
	count = sizeof(sizeof(inner_lengths) / sizeof(inner_lengths[0]));
    }
    CGContextSetLineDash((CGContextRef)d->hd, 0, lengths, count);

    CGLineCap cglinecap = kCGLineCapButt;
    if(ps->pen.capStyle() == SquareCap)
	cglinecap = kCGLineCapSquare;
    else if(ps->pen.capStyle() == RoundCap)
	cglinecap = kCGLineCapRound;
    CGContextSetLineCap((CGContextRef)d->hd, cglinecap);

    CGContextSetLineWidth((CGContextRef)d->hd, ps->pen.width() < 1 ? 1 : ps->pen.width());

    const QColor &col = ps->pen.color();
    CGContextSetRGBStrokeColor((CGContextRef)d->hd, qt_mac_convert_color_to_cg(col.red()),
			       qt_mac_convert_color_to_cg(col.green()), qt_mac_convert_color_to_cg(col.blue()), 1.0);

    CGLineJoin cglinejoin = kCGLineJoinMiter;
    if(ps->pen.joinStyle() == BevelJoin)
	cglinejoin = kCGLineJoinBevel;
    else if(ps->pen.joinStyle() == RoundJoin)
	cglinejoin = kCGLineJoinRound;
    CGContextSetLineJoin((CGContextRef)d->hd, cglinejoin);
}

void
QCoreGraphicsGC::updateBrush(QPainterState *ps)
{
    Q_ASSERT(isActive());
    d->current.brush = ps->brush;

    static uchar dense1_pat[] = { 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff };
    static uchar dense2_pat[] = { 0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff };
    static uchar dense3_pat[] = { 0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee };
    static uchar dense4_pat[] = { 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa };
    static uchar dense5_pat[] = { 0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11 };
    static uchar dense6_pat[] = { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 };
    static uchar dense7_pat[] = { 0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00 };
    static uchar hor_pat[] = {			// horizontal pattern
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    static uchar ver_pat[] = {			// vertical pattern
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20 };
    static uchar cross_pat[] = {			// cross pattern
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0xff, 0xff, 0xff,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0xff, 0xff, 0xff, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0xff, 0xff, 0xff,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0xff, 0xff, 0xff, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20 };
    static uchar bdiag_pat[] = {			// backward diagonal pattern
	0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04, 0x02, 0x02, 0x01, 0x01,
	0x80, 0x80, 0x40, 0x40, 0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04,
	0x02, 0x02, 0x01, 0x01, 0x80, 0x80, 0x40, 0x40 };
    static uchar fdiag_pat[] = {			// forward diagonal pattern
	0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40,
	0x80, 0x80, 0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10,
	0x20, 0x20, 0x40, 0x40, 0x80, 0x80, 0x01, 0x01 };
    static uchar dcross_pat[] = {			// diagonal cross pattern
	0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14, 0x22, 0x22, 0x41, 0x41,
	0x80, 0x80, 0x41, 0x41, 0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14,
	0x22, 0x22, 0x41, 0x41, 0x80, 0x80, 0x41, 0x41 };
    static uchar *pat_tbl[] = {
	dense1_pat, dense2_pat, dense3_pat, dense4_pat, dense5_pat, dense6_pat, dense7_pat,
	hor_pat, ver_pat, cross_pat, bdiag_pat, fdiag_pat, dcross_pat };

    const QColor &col = ps->brush.color();
    CGContextSetRGBFillColor((CGContextRef)d->hd, qt_mac_convert_color_to_cg(col.red()),
			       qt_mac_convert_color_to_cg(col.green()), qt_mac_convert_color_to_cg(col.blue()), 1.0);

    int bs = ps->brush.style();
    if(bs != SolidPattern && bs != NoBrush) {
	if(d->fill_pattern)
	    CGPatternRelease(d->fill_pattern);
	if(d->fill_colorspace)
	    CGColorSpaceRelease(d->fill_colorspace);

	int width = 0, height = 0;
	CGColorSpaceRef cs_base = 0;
	QMacPattern *qpattern = new QMacPattern;
	qpattern->im = 0;
	if(bs == CustomPattern) {
	    qpattern->as_mask = false;
	    qpattern->pixmap = new QPixmap(*ps->brush.pixmap());
	    width = qpattern->pixmap->width();
	    height = qpattern->pixmap->height();
	} else {
	    qpattern->as_mask = true;
	    qpattern->mask.bytes = pat_tbl[bs-Dense1Pattern];
	    if(bs<=Dense7Pattern)
		qpattern->mask.byte_per_line = 8;
	    else if(bs<=CrossPattern)
		qpattern->mask.byte_per_line = 24;
	    else
		qpattern->mask.byte_per_line = 16;
	    width = height = qpattern->mask.byte_per_line;
	    cs_base = CGColorSpaceCreateDeviceRGB();
	}
	d->fill_colorspace = CGColorSpaceCreatePattern(cs_base);

	CGPatternCallbacks callbks;
	callbks.version = 0;
	callbks.drawPattern = qt_mac_draw_pattern;
	callbks.releaseInfo = qt_mac_dispose_pattern;
	d->fill_pattern = CGPatternCreate(qpattern, CGRectMake(0, 0, width, height), CGContextGetCTM((CGContextRef)d->hd), width, height,
						  kCGPatternTilingNoDistortion, !qpattern->as_mask, &callbks);
	CGContextSetFillColorSpace((CGContextRef)d->hd, d->fill_colorspace);
	const float tmp_float = 1; //wtf?? --SAM (this seems to be necessary, but why!?!) ###
	CGContextSetFillPattern((CGContextRef)d->hd, d->fill_pattern, &tmp_float);
    }
}

void
QCoreGraphicsGC::updateFont(QPainterState *ps)
{
    Q_ASSERT(isActive());
    //TBD
}

void 
QCoreGraphicsGC::updateRasterOp(QPainterState *ps)
{
    Q_ASSERT(isActive());
    if(ps->rasterOp != CopyROP)
	qDebug("Cannot support any raster ops other than Copy!");
}

void 
QCoreGraphicsGC::updateBackground(QPainterState *ps)
{
    Q_ASSERT(isActive());
    d->current.bg.origin = ps->bgOrigin;
    d->current.bg.mode = ps->bgMode;
    d->current.bg.color = ps->bgColor;
}

void 
QCoreGraphicsGC::updateXForm(QPainterState *ps)
{
    Q_ASSERT(isActive());
}

void
QCoreGraphicsGC::updateClipRegion(QPainterState *ps)
{
    Q_ASSERT(isActive());

    bool old_clipon = testf(ClipOn);
    if(ps->clipEnabled)
	setf(ClipOn);
    else
	clearf(ClipOn);

    if(d->hd) {
	if(ps->clipEnabled || ps->clipEnabled != old_clipon) { //reset the clip
	    CGContextRelease((CGContextRef)d->hd);
	    if(qt_mac_update_cg(d))
		CGContextRetain((CGContextRef)d->hd);
	}
	if(d->hd && ps->clipEnabled)
	    qt_mac_clip_cg_handle((CGContextRef)d->hd, ps->clipRegion, QPoint(d->offx, d->offy), true);
    }
}

void 
QCoreGraphicsGC::setRasterOp(RasterOp r)
{
    Q_ASSERT(isActive());
    if(r != CopyROP)
	qDebug("Cannot support any raster ops other than Copy!");
}

void 
QCoreGraphicsGC::drawLine(const QPoint &p1, const QPoint &p2)
{
    Q_ASSERT(isActive());

    float cg_x, cg_y;
    d->mac_point(p1.x(), p1.y(), &cg_x, &cg_y);
    CGContextMoveToPoint((CGContextRef)d->hd, cg_x, cg_y);
    d->mac_point(p2.x(), p2.y(), &cg_x, &cg_y);
    CGContextAddLineToPoint((CGContextRef)d->hd, cg_x, cg_y);
    CGContextStrokePath((CGContextRef)d->hd);
}

void 
QCoreGraphicsGC::drawRect(const QRect &r)
{
    Q_ASSERT(isActive());

    CGRect mac_rect;
    d->mac_rect(r.x(), r.y(), r.width(), r.height(), &mac_rect);
    CGContextBeginPath((CGContextRef)d->hd);
    CGContextAddRect((CGContextRef)d->hd, mac_rect);
    if(d->current.brush.style() != NoBrush)
	CGContextFillPath((CGContextRef)d->hd);
    if(d->current.pen.style() != NoPen)
	CGContextStrokePath((CGContextRef)d->hd);
}

void 
QCoreGraphicsGC::drawPoint(const QPoint &p)
{
    Q_ASSERT(isActive());

    float cg_x, cg_y;
    d->mac_point(p.x(), p.y(), &cg_x, &cg_y);
    CGContextMoveToPoint((CGContextRef)d->hd, cg_x, cg_y);
    CGContextAddLineToPoint((CGContextRef)d->hd, cg_x+1, cg_y);
    CGContextStrokePath((CGContextRef)d->hd);
}

void 
QCoreGraphicsGC::drawPoints(const QPointArray &pa, int index, int npoints)
{
    Q_ASSERT(isActive());

    float cg_x, cg_y;
    for(int i=0; i<npoints; i++) {
	d->mac_point(pa[index+i].x(), pa[index+i].y(), &cg_x, &cg_y);
	CGContextMoveToPoint((CGContextRef)d->hd, cg_x, cg_y);
	CGContextAddLineToPoint((CGContextRef)d->hd, cg_x+1, cg_y);
	CGContextStrokePath((CGContextRef)d->hd);
    }
}

void 
QCoreGraphicsGC::drawWinFocusRect(const QRect &fr, bool xorPaint, const QColor &penColor)
{
    Q_ASSERT(isActive());

    //save
    QPainterState old_state = *state;

    //setup
    if(xorPaint) 
	qWarning("cannot support xor painting!");
    if(qGray(d->current.bg.color.rgb()) < 128)
	state->pen = QPen(white);
    else
	state->pen = QPen(black);
    state->pen.setStyle(DashLine);
    updatePen(state);

    //draw
    if(d->current.pen.style() != NoPen)
	drawRect(fr);

    //restore
    *state = old_state;
    updatePen(state);
}

void 
QCoreGraphicsGC::drawRoundRect(const QRect &r, int xRnd, int yRnd)
{
    Q_ASSERT(isActive());

    // I need to test how close this is to other platforms, I only rolled this without testing --Sam
    float cg_x, cg_y;
    d->mac_point(r.x(), r.y(), &cg_x, &cg_y);

    CGMutablePathRef path = CGPathCreateMutable();
    CGPathMoveToPoint(path, 0, cg_x, cg_y+yRnd);                                                                 //start
    CGPathAddQuadCurveToPoint(path, 0, cg_x, cg_y, cg_x+xRnd, cg_y);                                             //top left
    CGPathAddLineToPoint(path, 0, cg_x+(r.width()-xRnd), cg_y);                                                  //top
    CGPathAddQuadCurveToPoint(path, 0, cg_x+r.width(), cg_y, cg_x+r.width(), cg_y+yRnd);                         //top right
    CGPathAddLineToPoint(path, 0, cg_x+r.width(), cg_y+(r.height()-yRnd));                                       //right
    CGPathAddQuadCurveToPoint(path, 0, cg_x+r.width(), cg_y+r.height(), cg_x+(r.width()-xRnd), cg_y+r.height()); //bottom right
    CGPathAddLineToPoint(path, 0, cg_x+xRnd, cg_y+r.height());                                                   //bottom
    CGPathAddQuadCurveToPoint(path, 0, cg_x, cg_y+r.height(), cg_x, cg_y+(r.height()-yRnd));                     //bottom left
    CGPathAddLineToPoint(path, 0, cg_x, cg_y+yRnd);                                                              //left
    CGContextBeginPath((CGContextRef)d->hd);
    CGContextAddPath((CGContextRef)d->hd, path);
    if(d->current.brush.style() != NoBrush)
	CGContextFillPath((CGContextRef)d->hd);
    if(d->current.pen.style() != NoPen)
	CGContextStrokePath((CGContextRef)d->hd);
    CGPathRelease(path);
}

void 
QCoreGraphicsGC::drawEllipse(const QRect &r)
{
    Q_ASSERT(isActive());

    CGMutablePathRef path = CGPathCreateMutable();
    CGAffineTransform transform;
    if(r.width() != r.height())
	transform = CGAffineTransformMakeScale(((float)r.width())/r.height(), 1);
    float cg_x, cg_y;
    d->mac_point(r.x(), r.y(), &cg_x, &cg_y);
    CGPathAddArc(path, r.width() == r.height() ? 0 : &transform, 
		 (cg_x+(r.width()/2))/((float)r.width()/r.height()), 
		 cg_y + (r.height()/2), r.height()/2, 0, 360, false);
    CGContextBeginPath((CGContextRef)d->hd);
    CGContextAddPath((CGContextRef)d->hd, path);
    if(d->current.brush.style() != NoBrush)
	CGContextFillPath((CGContextRef)d->hd);
    if(d->current.pen.style() != NoPen)
	CGContextStrokePath((CGContextRef)d->hd);
    CGPathRelease(path);
}

void 
QCoreGraphicsGC::drawArc(const QRect &r, int a, int alen)
{
    Q_ASSERT(isActive());

    CGMutablePathRef path = CGPathCreateMutable();
    CGAffineTransform transform;
    if(r.width() != r.height())
	transform = CGAffineTransformMakeScale(((float)r.width())/r.height(), 1);
    float cg_x, cg_y;
    d->mac_point(r.x(), r.y(), &cg_x, &cg_y);
    float begin_radians = ((float)(a/16)+180) * (M_PI/180), end_radians = ((float)((a+alen)/16)+180) * (M_PI/180);
    CGPathAddArc(path, r.width() == r.height() ? 0 : &transform, 
		 (cg_x+(r.width()/2))/((float)r.width()/r.height()), cg_y + (r.height()/2), 
		 r.height()/2, begin_radians, end_radians, a < 0 || alen < 0);
    CGContextBeginPath((CGContextRef)d->hd);
    CGContextAddPath((CGContextRef)d->hd, path);
    if(d->current.pen.style() != NoPen)
	CGContextStrokePath((CGContextRef)d->hd);
    CGPathRelease(path);
}

void 
QCoreGraphicsGC::drawPie(const QRect &r, int a, int alen)
{
    Q_ASSERT(isActive());

    CGMutablePathRef path = CGPathCreateMutable();
    CGAffineTransform transform;
    if(r.width() != r.height())
	transform = CGAffineTransformMakeScale(((float)r.width())/r.height(), 1);
    float cg_x, cg_y;
    d->mac_point(r.x(), r.y(), &cg_x, &cg_y);
    float begin_radians = ((float)(a/16)+180) * (M_PI/180), end_radians = ((float)((a+alen)/16)+180) * (M_PI/180);
    CGPathMoveToPoint(path, 0, cg_x + (r.width()/2), cg_y + (r.height()/2));
    CGPathAddArc(path, (r.width() == r.height()) ? 0 : &transform, 
		 (cg_x+(r.width()/2))/((float)r.width()/r.height()), 
		 cg_y + (r.height()/2), r.height()/2, begin_radians, end_radians, a < 0 || alen < 0);
    CGPathAddLineToPoint(path, 0, cg_x + (r.width()/2), cg_y + (r.height()/2));
    CGContextBeginPath((CGContextRef)d->hd);
    CGContextAddPath((CGContextRef)d->hd, path);
    if(d->current.brush.style() != NoBrush)
	CGContextFillPath((CGContextRef)d->hd);
    if(d->current.pen.style() != NoPen)
	CGContextStrokePath((CGContextRef)d->hd);
    CGPathRelease(path);
}

void 
QCoreGraphicsGC::drawPolyInternal(const QPointArray &a, bool close)
{
    Q_ASSERT(isActive());

    float cg_x, cg_y;
    d->mac_point(a[0].x(), a[0].y(), &cg_x, &cg_y);
    CGContextMoveToPoint((CGContextRef)d->hd, cg_x, cg_y);
    for(int x = 1; x < a.size(); x++) {
	d->mac_point(a[x].x(), a[x].y(), &cg_x, &cg_y);
	CGContextAddLineToPoint((CGContextRef)d->hd, cg_x, cg_y);
    }
    if(close) {
	d->mac_point(a[0].x(), a[0].y(), &cg_x, &cg_y);
	CGContextAddLineToPoint((CGContextRef)d->hd, cg_x, cg_y);
    }
    if(d->current.brush.style() != NoBrush)
	CGContextFillPath((CGContextRef)d->hd);
    if(d->current.pen.style() != NoPen)
	CGContextStrokePath((CGContextRef)d->hd);
}

void 
QCoreGraphicsGC::drawChord(const QRect &r, int a, int alen)
{
    Q_ASSERT(isActive());

    CGMutablePathRef path = CGPathCreateMutable();
    CGAffineTransform transform;
    if(r.width() != r.height())
	transform = CGAffineTransformMakeScale(((float)r.width())/r.height(), 1);
    float cg_x, cg_y;
    d->mac_point(r.x(), r.y(), &cg_x, &cg_y);
    float begin_radians = ((float)(a/16)+180) * (M_PI/180), end_radians = ((float)((a+alen)/16)+180) * (M_PI/180);
    //We draw twice because the first draw will set the point to the end of arc, and the second pass will draw the line to the first point
    for(int i = 0; i < 2; i++)
	CGPathAddArc(path, r.width() == r.height() ? 0 : &transform, 
		     (cg_x+(r.width()/2))/((float)r.width()/r.height()), 
		     cg_y+(r.height()/2), r.height()/2, begin_radians, end_radians, a < 0 || alen < 0);
    CGContextBeginPath((CGContextRef)d->hd);
    CGContextAddPath((CGContextRef)d->hd, path);
    if(d->current.brush.style() != NoBrush)
	CGContextFillPath((CGContextRef)d->hd);
    if(d->current.pen.style() != NoPen)
	CGContextStrokePath((CGContextRef)d->hd);
    CGPathRelease(path);
}

void 
QCoreGraphicsGC::drawLineSegments(const QPointArray &pa, int index, int nlines)
{
    Q_ASSERT(isActive());

    float cg_x, cg_y;
    int  x1, y1, x2, y2;
    uint i = index;
    while(nlines--) {
	pa.point(i++, &x1, &y1);
	pa.point(i++, &x2, &y2);
	d->mac_point(x1, y1, &cg_x, &cg_y);
	CGContextMoveToPoint((CGContextRef)d->hd, cg_x, cg_y);
	d->mac_point(x2, y2, &cg_x, &cg_y);
	CGContextAddLineToPoint((CGContextRef)d->hd, cg_x, cg_y);
	CGContextStrokePath((CGContextRef)d->hd);
    }
}

void 
QCoreGraphicsGC::drawPolyline(const QPointArray &pa, int index, int npoints)
{
    Q_ASSERT(isActive());

    float cg_x, cg_y;
    d->mac_point(pa[0].x(), pa[0].y(), &cg_x, &cg_y);
    CGContextMoveToPoint((CGContextRef)d->hd, cg_x, cg_y);
    for(int x = 1; x < pa.size(); x++) {
	d->mac_point(pa[x].x(), pa[x].y(), &cg_x, &cg_y);
	CGContextAddLineToPoint((CGContextRef)d->hd, cg_x, cg_y);
    }
    if(d->current.pen.style() != NoPen)
	CGContextStrokePath((CGContextRef)d->hd);
}

void 
QCoreGraphicsGC::drawPolygon(const QPointArray &a, bool winding, int index, int npoints)
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
QCoreGraphicsGC::drawConvexPolygon(const QPointArray &pa, int index, int npoints)
{
    // Implemented in terms of drawPolygon() [no optimization]
    drawPolygon(pa,false,index,npoints);
}

#ifndef QT_NO_BEZIER
void 
QCoreGraphicsGC::drawCubicBezier(const QPointArray &pa, int index)
{
    Q_ASSERT(isActive());

    float cg_x, cg_y;
    d->mac_point(pa[0].x(), pa[0].y(), &cg_x, &cg_y);
    CGContextMoveToPoint((CGContextRef)d->hd, cg_x, cg_y);
    float c1_x, c1_y, c2_x, c2_y;
    d->mac_point(pa[1].x(), pa[1].y(), &c1_x, &c1_y);
    d->mac_point(pa[2].x(), pa[2].y(), &c2_x, &c2_y);
    d->mac_point(pa[3].x(), pa[3].y(), &cg_x, &cg_y);
    CGContextAddCurveToPoint((CGContextRef)d->hd, c1_x, c1_y, c2_x, c2_y, cg_x, cg_y);
    if(d->current.pen.style() != NoPen)
	CGContextStrokePath((CGContextRef)d->hd);
}
#endif

void 
QCoreGraphicsGC::drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr)
{
    qDebug("Must implement drawPixmap!!");
}

void 
QCoreGraphicsGC::drawTextItem(const QPoint &p, const QTextItem &ti, int textflags)
{
    qDebug("Must implement drawTextItem!!");
}

Qt::HANDLE
QCoreGraphicsGC::handle() const
{
    return d->hd;
}

void
QCoreGraphicsGC::initialize()
{
}

void 
QCoreGraphicsGC::cleanup()
{
}

void 
QCoreGraphicsGC::drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &p, bool optim)
{
    Q_ASSERT(isActive());

    //save the old state
    CGContextSaveGState((CGContextRef)d->hd);
    //setup the pattern
    QMacPattern *qpattern = new QMacPattern;
    qpattern->im = 0;
    qpattern->as_mask = false;
    qpattern->pixmap = new QPixmap(pixmap);
    CGPatternCallbacks callbks;
    callbks.version = 0;
    callbks.drawPattern = qt_mac_draw_pattern;
    callbks.releaseInfo = qt_mac_dispose_pattern;
    const int width = pixmap.width(), height = pixmap.height();
    CGPatternRef pat = CGPatternCreate(qpattern, CGRectMake(0, 0, width, height), CGContextGetCTM((CGContextRef)d->hd), width, height,
					      kCGPatternTilingNoDistortion, true, &callbks);
    CGColorSpaceRef cs = CGColorSpaceCreatePattern(NULL);
    CGContextSetFillColorSpace((CGContextRef)d->hd, cs);
    const float tmp_float = 1; //wtf?? --SAM (this seems to be necessary, but why!?!) ###
    CGContextSetFillPattern((CGContextRef)d->hd, pat, &tmp_float);
    CGContextSetPatternPhase((CGContextRef)d->hd, CGSizeMake(-p.x(), -p.y()));
    //fill the rectangle
    CGRect mac_rect;
    d->mac_rect(r.x(), r.y(), r.width(), r.height(), &mac_rect);
    CGContextFillRect((CGContextRef)d->hd, mac_rect);
    //restore the state
    CGContextRestoreGState((CGContextRef)d->hd);
    //cleanup
    CGColorSpaceRelease(cs);
    CGPatternRelease(pat);
}

#endif

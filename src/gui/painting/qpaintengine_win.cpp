/****************************************************************************
**
** Definition of QWin32PaintEngine class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qbitmap.h"
#include "qbrush.h"
#include "qpaintdevice.h"
#include "qpaintdevice.h"
#include "qpaintengine_win.h"
#include "qpaintengine_win_p.h"
#include "qpainter.h"
#include "qpainter_p.h"
#include "qpen.h"
#include "qpixmap.h"
#include "qt_windows.h"
#include "qtextlayout.h"
#include "qwidget.h"

#include <private/qfontengine_p.h>
#include <private/qtextengine_p.h>

#include <qdebug.h>

#include <math.h>

// #define NO_NATIVE_XFORM

#define d d_func()
#define q q_func()

#define MY_DEBUG() printf("%s:%d\n", __FILE__, __LINE__);

static QSysInfo::WinVersion qt_winver = QSysInfo::WV_NT;

/*****************************************************************************
  QPainter internal pen and brush cache

  The cache makes a significant contribution to speeding up drawing.
  Setting a new pen or brush specification will make the painter look for
  an existing pen or brush with the same attributes instead of creating
  a new pen or brush. The cache structure is optimized for fast lookup.
  Only solid line pens with line width 0 and solid brushes are cached.
 *****************************************************************************/
struct QHDCObj					// cached pen or brush
{
    HANDLE  obj;
    uint    pix;
    int	    count;
    int	    hits;
};

const int	cache_size = 29;		// multiply by 4
static QHDCObj *pen_cache_buf;
static QHDCObj *pen_cache[4*cache_size];
static QHDCObj *brush_cache_buf;
static QHDCObj *brush_cache[4*cache_size];
static bool	cache_init = FALSE;

static HPEN stock_nullPen;
static HPEN stock_blackPen;
static HPEN stock_whitePen;
static HBRUSH stock_nullBrush;
static HBRUSH stock_blackBrush;
static HBRUSH stock_whiteBrush;
static HFONT  stock_sysfont;

static QHDCObj stock_dummy;
static void  *stock_ptr = (void *)&stock_dummy;

static void init_cache()
{
    if (cache_init)
	return;
    int i;
    QHDCObj *h;
    cache_init = TRUE;
    h = pen_cache_buf = new QHDCObj[4*cache_size];
    memset(h, 0, 4*cache_size*sizeof(QHDCObj));
    for (i=0; i<4*cache_size; i++)
	pen_cache[i] = h++;
    h = brush_cache_buf = new QHDCObj[4*cache_size];
    memset(h, 0, 4*cache_size*sizeof(QHDCObj));
    for (i=0; i<4*cache_size; i++)
	brush_cache[i] = h++;
}

QRegion* paintEventClipRegion = 0;
QPaintDevice* paintEventDevice = 0;

void qt_set_paintevent_clipping( QPaintDevice* dev, const QRegion& region)
{
    if ( !paintEventClipRegion )
	paintEventClipRegion = new QRegion( region );
    else
	*paintEventClipRegion = region;
    paintEventDevice = dev;
}

void qt_clear_paintevent_clipping()
{
    delete paintEventClipRegion;
    paintEventClipRegion = 0;
    paintEventDevice = 0;
}

// #define CACHE_STAT
#if defined(CACHE_STAT)
#include "qtextstream.h"

static int c_numhits	= 0;
static int c_numcreates = 0;
static int c_numfaults	= 0;
#endif

static void cleanup_cache()
{
    if (!cache_init)
	return;
    int i;
#if defined(CACHE_STAT)
    qDebug("Number of cache hits = %d", c_numhits);
    qDebug("Number of cache creates = %d", c_numcreates);
    qDebug("Number of cache faults = %d", c_numfaults);
    qDebug("PEN CACHE");
    for (i=0; i<cache_size; i++) {
	QString	    str;
	QTextStream s(str,IO_WriteOnly);
	s << i << ": ";
	for (int j=0; j<4; j++) {
	    QHDCObj *h = pen_cache[i*4+j];
	    s << (h->obj ? 'X' : '-') << ',' << h->hits << ','
	      << h->count << '\t';
	}
	s << '\0';
	qDebug(str);
    }
    qDebug("BRUSH CACHE");
    for (i=0; i<cache_size; i++) {
	QString	    str;
	QTextStream s(str,IO_WriteOnly);
	s << i << ": ";
	for (int j=0; j<4; j++) {
	    QHDCObj *h = brush_cache[i*4+j];
	    s << (h->obj ? 'X' : '-') << ',' << h->hits << ','
	      << h->count << '\t';
	}
	s << '\0';
	qDebug(str);
    }
#endif
    for (i=0; i<4*cache_size; i++) {
	if (pen_cache[i]->obj)
	    DeleteObject(pen_cache[i]->obj);
	if (brush_cache[i]->obj)
	    DeleteObject(brush_cache[i]->obj);
    }
    delete [] pen_cache_buf;
    delete [] brush_cache_buf;
    cache_init = FALSE;
}


static bool obtain_obj(void **ref, HANDLE *obj, uint pix, QHDCObj **cache,
			bool is_pen)
{
    if (!cache_init)
	init_cache();

    int	     k = (pix % cache_size) * 4;
    QHDCObj *h = cache[k];
    QHDCObj *prev = 0;

#define NOMATCH (h->obj && h->pix != pix)

    if (NOMATCH) {
	prev = h;
	h = cache[++k];
	if (NOMATCH) {
	    prev = h;
	    h = cache[++k];
	    if (NOMATCH) {
		prev = h;
		h = cache[++k];
		if (NOMATCH) {
		    if (h->count == 0) {	// steal this pen/brush
#if defined(CACHE_STAT)
			c_numcreates++;
#endif
			h->pix	 = pix;
			h->count = 1;
			h->hits	 = 1;
			DeleteObject(h->obj);
			if (is_pen)
			    h->obj = CreatePen(PS_SOLID, 0, pix);
			else
			    h->obj = CreateSolidBrush(pix);
			cache[k]   = prev;
			cache[k-1] = h;
			*ref = (void *)h;
			*obj = h->obj;
			return TRUE;
		    } else {			// all objects in use
#if defined(CACHE_STAT)
			c_numfaults++;
#endif
			*ref = 0;
			return FALSE;
		    }
		}
	    }
	}
    }

#undef NOMATCH

    *ref = (void *)h;

    if (h->obj) {				// reuse existing pen/brush
#if defined(CACHE_STAT)
	c_numhits++;
#endif
	*obj = h->obj;
	h->count++;
	h->hits++;
	if (prev && h->hits > prev->hits) {	// maintain LRU order
	    cache[k]   = prev;
	    cache[k-1] = h;
	}
    } else {					// create new pen/brush
#if defined(CACHE_STAT)
	c_numcreates++;
#endif
	if (is_pen)
	    h->obj = CreatePen(PS_SOLID, 0, pix);
	else
	    h->obj = CreateSolidBrush(pix);
	h->pix	 = pix;
	h->count = 1;
	h->hits	 = 1;
	*obj = h->obj;
    }
    return TRUE;
}

static inline void release_obj(void *ref)
{
    ((QHDCObj*)ref)->count--;
}

static inline bool obtain_pen(void **ref, HPEN *pen, uint pix)
{
    return obtain_obj(ref, (HANDLE*)pen, pix, pen_cache, TRUE);
}

static inline bool obtain_brush(void **ref, HBRUSH *brush, uint pix)
{
    return obtain_obj(ref, (HANDLE*)brush, pix, brush_cache, FALSE);
}


#define release_pen	release_obj
#define release_brush	release_obj

QWin32PaintEngine::QWin32PaintEngine(QWin32PaintEnginePrivate &dptr, QPaintDevice *target,
				     GCCaps caps)
    :
#ifndef NO_NATIVE_XFORM
      QPaintEngine(dptr, caps)
#else
      QPaintEngine(dptr, caps)
#endif

{
    // ### below is temp hack to survive pixmap gc construction
    d->hwnd = (target && target->devType()==QInternal::Widget) ? ((QWidget*)target)->winId() : 0;
    d->flags |= IsStartingUp;
}

QWin32PaintEngine::QWin32PaintEngine(QPaintDevice *target)
    :
#ifndef NO_NATIVE_XFORM
      QPaintEngine(*(new QWin32PaintEnginePrivate), GCCaps(CoordTransform
							   | PenWidthTransform
							   | PixmapTransform
							   | UsesFontEngine))
#else
      QPaintEngine(*(new QWin32PaintEnginePrivate), GCCaps(UsesFontEngine))
#endif

{
    // ### below is temp hack to survive pixmap gc construction
    d->hwnd = (target && target->devType()==QInternal::Widget) ? ((QWidget*)target)->winId() : 0;
    d->flags |= IsStartingUp;
}

QWin32PaintEngine::~QWin32PaintEngine()
{
}


bool QWin32PaintEngine::begin(QPaintDevice *pdev, QPainterState *state, bool unclipped)
{
    if (isActive()) {				// already active painting
	qWarning("QWin32PaintEngine::begin: Painter is already active."
	       "\n\tYou must end() the painter before a second begin()\n");
// 	return true;
    }

    setActive(true);
    d->pdev = pdev;

    if (pdev->devType() == QInternal::Widget) {
	QWidget *w = (QWidget*)pdev;
	d->usesWidgetDC = (w->hdc != 0);
	if (d->usesWidgetDC) {
	    d->hdc = w->hdc;			// during paint event
	} else {
	    if ( unclipped || w->testWFlags( WPaintUnclipped ) ) {
		d->hdc = GetWindowDC( w->winId() );
		if ( w->isTopLevel() ) {
		    int dx = w->geometry().x() - w->frameGeometry().x();
		    int dy = w->geometry().y() - w->frameGeometry().y();
#ifndef Q_OS_TEMP
		    SetWindowOrgEx( d->hdc, -dx, -dy, 0 );
#else
//		    MoveWindow( w->winId(), w->frameGeometry().x(), w->frameGeometry().y(), w->frameGeometry().width(), w->frameGeometry().height(), FALSE );
//		    MoveWindow( w->winId(), w->frameGeometry().x() - 50, w->frameGeometry().y() - 50, w->frameGeometry().width(), w->frameGeometry().height(), FALSE );
#endif
		}
	    } else {
		d->hdc = GetDC( w->isDesktop() ? 0 : w->winId() );
	    }
	    w->hdc = d->hdc;
	}
    } else if (pdev->devType() == QInternal::Pixmap) {
	d->hdc = static_cast<QPixmap *>(pdev)->handle();
    }
    Q_ASSERT(d->hdc);

    QRegion *region = paintEventClipRegion;
    QRect r = region ? region->boundingRect() : QRect();
    if (region)
	SelectClipRgn(d->hdc, region->handle());

    if (QColor::hPal()) {
	d->holdpal = SelectPalette(d->hdc, QColor::hPal(), true);
	RealizePalette(d->hdc);
    }

    SetTextAlign(d->hdc, TA_BASELINE);
    SetBkColor(d->hdc, COLOR_VALUE(state->bgBrush.color()));
    SetBkMode(d->hdc, state->bgMode == TransparentMode ? TRANSPARENT : OPAQUE);
    SetROP2(d->hdc, rasterOpCodes[state->rasterOp]);
    SetTextColor(d->hdc, COLOR_VALUE(state->pen.color()));
    return true;
}

bool QWin32PaintEngine::end()
{
    if ( !isActive() ) {
	qWarning( "QWin32PaintEngine::end: Missing begin() or begin() failed" );
	return false;
    }

//     killPStack();

    if ( d->hpen ) {
	SelectObject( d->hdc, stock_nullPen );
	if ( d->penRef ) {
	    release_pen( d->penRef );
	    d->penRef = 0;
	} else {
	    DeleteObject( d->hpen );
	}
	d->hpen = 0;
    }
    if ( d->hbrush ) {
	SelectObject( d->hdc, stock_nullBrush );
	if ( d->brushRef ) {
	    release_brush( d->brushRef );
	    d->brushRef = 0;
	} else {
	    DeleteObject( d->hbrush );
	    if ( d->hbrushbm && !d->pixmapBrush )
		DeleteObject( d->hbrushbm );
	}
	d->hbrush = 0;
	d->hbrushbm = 0;
	d->pixmapBrush = d->nocolBrush = false;
    }
    if ( d->hfont ) {
	SelectObject( d->hdc, stock_sysfont );
	d->hfont = 0;
    }
    if ( d->holdpal ) {
	SelectPalette( d->hdc, d->holdpal, true );
	RealizePalette( d->hdc );
    }

    if (d->pdev->devType() == QInternal::Widget) {
	if (!d->usesWidgetDC) {
	    QWidget *w = (QWidget*)d->pdev;
  	    ReleaseDC( w->isDesktop() ? 0 : w->winId(), d->hdc );
	    w->hdc = 0;
	}
    } else if (d->pdev->devType() == QInternal::Pixmap) {
	QPixmap *pm = (QPixmap *)d->pdev;
	if (pm->optimization() == QPixmap::MemoryOptim &&
	     (qt_winver & QSysInfo::WV_DOS_based))
	    pm->allocCell();
    }

    if (GetGraphicsMode(d->hdc)==GM_ADVANCED) {
	if (!ModifyWorldTransform(d->hdc, 0, MWT_IDENTITY))
	    qWarning("QWin32PaintEngine::end(). ModifyWorldTransform failed: code: %d\n", GetLastError());
	SetGraphicsMode(d->hdc, GM_COMPATIBLE);
    }

    d->hdc = 0;

    setActive(false);
    return true;
}

void QWin32PaintEngine::drawLine(const QPoint &p1, const QPoint &p2)
{
    Q_ASSERT(isActive());

    int x1 = p1.x(), x2 = p2.x(), y1 = p1.y(), y2 = p2.y();
    bool plot_pixel = FALSE;
    plot_pixel = (d->pWidth == 0) && (d->pStyle == SolidLine);
    if (plot_pixel) {
	if (x1 == x2) {				// vertical
	    if (y1 < y2)
		y2++;
	    else
		y2--;
	    plot_pixel = FALSE;
	} else if (y1 == y2) {			// horizontal
	    if (x1 < x2)
		x2++;
	    else
		x2--;
	    plot_pixel = FALSE;
	}
    }
    bool path = FALSE;
#ifndef Q_OS_TEMP
    QT_WA({
	if (d->pWidth > 1) {
	    // on DOS based systems caps and joins are only supported on paths, so let's use them.
	    BeginPath(d->hdc);
	    path = TRUE;
	}
    }, { });
    MoveToEx(d->hdc, x1, y1, 0);
    LineTo(d->hdc, x2, y2);
    if (path) {
	EndPath(d->hdc);
	StrokePath(d->hdc);
	MoveToEx(d->hdc, x2, y2, 0);
    }
    if (plot_pixel)
	SetPixelV(d->hdc, x2, y2, d->pColor);
#else
    POINT pts[2];
    pts[0].x = x1;  pts[0].y = y1;
    pts[1].x = x2;  pts[1].y = y2;
    if (x1 != x2)
	pts[1].x += (x2 > x1 ? 1 : -1);
    if (y1 != y2)
	pts[1].y += (y2 > y1 ? 1 : -1);

    Polyline(d->hdc, pts, 2);
    if (plot_pixel)
	SetPixel(d->hdc, x2, y2, d->pColor);
    internalCurrentPos = QPoint(x2, y2);
#endif
}

void QWin32PaintEngine::drawRect(const QRect &r)
{
    Q_ASSERT(isActive());

    int w = r.width(), h = r.height();

    if (d->pStyle == NoPen) {
	++w;
	++h;
    }

    // Due to inclusive rectangles when using GM_ADVANCED
#ifndef NO_NATIVE_XFORM
    --w;
    --h;
#endif

    if (d->nocolBrush) {
	SetTextColor(d->hdc, d->bColor);
	Rectangle(d->hdc, r.x(), r.y(), r.x()+w, r.y()+h);
	SetTextColor(d->hdc, d->pColor);
    } else {
	Rectangle(d->hdc, r.x(), r.y(), r.x()+w, r.y()+h);
    }
}

void QWin32PaintEngine::drawPoint(const QPoint &p)
{
    Q_ASSERT(isActive());

    if (d->pStyle != NoPen)
#ifndef Q_OS_TEMP
	SetPixelV(d->hdc, p.x(), p.y(), d->pColor);
#else
	SetPixel(d->hdc, p.x(), p.y(), d->pColor);
#endif
}

void QWin32PaintEngine::drawPoints(const QPointArray &pts, int index, int npoints)
{
    Q_ASSERT(isActive());
    QPointArray pa = pts;
    if (d->pStyle != NoPen) {
	for (int i=0; i<npoints; i++) {
#ifndef Q_OS_TEMP
	    SetPixelV(d->hdc, pa[index+i].x(), pa[index+i].y(),
		       d->pColor);
#else
	    SetPixel(d->hdc, pa[index+i].x(), pa[index+i].y(),
		       d->pColor);
#endif
	}
    }
}

void QWin32PaintEngine::drawWinFocusRect(const QRect &fr, bool, const QColor &bgColor)
{
    if (!isActive())
	return;
    Q_ASSERT(d->hdc);

    RECT r;
    r.left   = fr.x();
    r.right  = fr.x() + fr.width();
    r.top    = fr.y();
    r.bottom = fr.y() + fr.height();

    if (qGray(bgColor.rgb()) < 10) { // Use white pen for very dark colors
	int col = GetBkColor(d->hdc);
 	SetBkColor(d->hdc, col /* ^0x00ffffff */ );
	DrawFocusRect(d->hdc, &r);
	SetBkColor(d->hdc, col);
    } else {
	DrawFocusRect(d->hdc, &r);
    }
}

void QWin32PaintEngine::drawRoundRect(const QRect &r, int xRnd, int yRnd)
{
    Q_ASSERT(isActive());

    if (xRnd <= 0 || yRnd <= 0) {
	drawRect(r);			// draw normal rectangle
	return;
    }
    if (xRnd >= 100)				// fix ranges
	xRnd = 99;
    if (yRnd >= 100)
	yRnd = 99;

    if (d->nocolBrush)
	SetTextColor(d->hdc, d->bColor);
    RoundRect(d->hdc, r.x(), r.y(), r.x()+r.width(), r.y()+r.height(),
	      r.width()*xRnd/100, r.height()*yRnd/100);
    if (d->nocolBrush)
	SetTextColor(d->hdc, d->pColor);
}

void QWin32PaintEngine::drawEllipse(const QRect &r)
{
    Q_ASSERT(isActive());
    // Workaround for Windows GDI
//     QPen oldPen = d->cpen;
//     if (d->cpen == PS_NULL) {
// // 	setPen(d->bColor);
// // 	updatePen();
//     }

    if (d->nocolBrush)
	SetTextColor(d->hdc, d->bColor);
    if (r.width() == 1 && r.height() == 1)
	drawPoint(r.topLeft());
    else
	Ellipse(d->hdc, r.x(), r.y(), r.x()+r.width(), r.y()+r.height());
    if (d->nocolBrush)
	SetTextColor(d->hdc, d->pColor);

//     if (oldPen == PS_NULL)
// 	setPen(oldPen);
}

void QWin32PaintEngine::drawArc(const QRect &r, int a, int alen)
{
    Q_ASSERT(isActive());

    double ra1 = 1.09083078249645598e-3 * a;
    double ra2 = 1.09083078249645598e-3 * alen + ra1;
    if (alen < 0.0) {				// swap angles
	double t = ra1;
	ra1 = ra2;
	ra2 = t;
    }
    int w = r.width();
    int h = r.height();
    if (d->pWidth > 3) {
	// work around a bug in the windows Arc method that
	// sometimes draw wrong cap styles.
	if (w % 2)
	    w += 1;
	if (h % 2)
	    h += 1;
    }

    double w2 = 0.5*w;
    double h2 = 0.5*h;
    int xS = qRound(w2 + (cos(ra1)*w2) + r.x());
    int yS = qRound(h2 - (sin(ra1)*h2) + r.y());
    int xE = qRound(w2 + (cos(ra2)*w2) + r.x());
    int yE = qRound(h2 - (sin(ra2)*h2) + r.y());
    if (QABS(alen) < 90*16) {
	if ((xS == xE) && (yS == yE)) {
	    // don't draw a whole circle
	    return; //### should we draw a point?
	}
    }
#ifndef Q_OS_TEMP
    if (d->rasterOp == CopyROP) {
        Arc(d->hdc, r.x(), r.y(), r.x()+w, r.y()+h, xS, yS, xE, yE);
    } else
#endif
    {
	QPointArray pa;
	pa.makeArc(r.x(), r.y(), w, h, a, alen);	// arc polyline
	drawPolyInternal(pa, FALSE);
    }
}

void QWin32PaintEngine::drawPie(const QRect &r, int a, int alen)
{
    Q_ASSERT(isActive());

    double ra1 = 1.09083078249645598e-3 * a;
    double ra2 = 1.09083078249645598e-3 * alen + ra1;
    if (alen < 0.0) {				// swap angles
	double t = ra1;
	ra1 = ra2;
	ra2 = t;
    }
    double w2 = 0.5*r.width();
    double h2 = 0.5*r.height();
    int xS = qRound(w2 + (cos(ra1)*r.width()) + r.x());
    int yS = qRound(h2 - (sin(ra1)*r.height()) + r.y());
    int xE = qRound(w2 + (cos(ra2)*r.width()) + r.x());
    int yE = qRound(h2 - (sin(ra2)*r.height()) + r.y());
    if (QABS(alen) < 90*16) {
	if ((xS == xE) && (yS == yE)) {
	    // don't draw a whole circle
	    return; //### should we draw something?
	}
    }
    if (d->nocolBrush)
	SetTextColor(d->hdc, d->bColor);
#ifndef Q_OS_TEMP
    Pie(d->hdc, r.x(), r.y(), r.right()+1, r.bottom()+1, xS, yS, xE, yE);
#endif
    if (d->nocolBrush)
	SetTextColor(d->hdc, d->pColor);
}

void QWin32PaintEngine::drawChord(const QRect &r, int a, int alen)
{
    Q_ASSERT(isActive());

    double ra1 = 1.09083078249645598e-3 * a;
    double ra2 = 1.09083078249645598e-3 * alen + ra1;
    if (ra2 < 0.0) {				// swap angles
	double t = ra1;
	ra1 = ra2;
	ra2 = t;
    }
    double w2 = 0.5*r.width();
    double h2 = 0.5*r.height();
    int xS = qRound(w2 + (cos(ra1)*r.width()) + r.x());
    int yS = qRound(h2 - (sin(ra1)*r.height()) + r.y());
    int xE = qRound(w2 + (cos(ra2)*r.width()) + r.x());
    int yE = qRound(h2 - (sin(ra2)*r.height()) + r.y());
    if (QABS(alen) < 90*16) {
	if ((xS == xE) && (yS == yE)) {
	    // don't draw a whole circle
	    return; //### should we draw something?
	}
    }
    if (d->nocolBrush)
	SetTextColor(d->hdc, d->bColor);
#ifndef Q_OS_TEMP
    Chord(d->hdc, r.x(), r.y(), r.right()+1, r.bottom()+1, xS, yS, xE, yE);
#endif
    if (d->nocolBrush)
	SetTextColor(d->hdc, d->pColor);
}


void QWin32PaintEngine::drawLineSegments(const QPointArray &a, int index, int nlines)
{
    Q_ASSERT(isActive());
    QPointArray pa = a;

    int	 x1, y1, x2, y2;
    uint i = index;
    uint pixel = d->pColor;
    bool maybe_plot_pixel = FALSE;
    QT_WA({
	maybe_plot_pixel = (d->pWidth == 0) && (d->pStyle == SolidLine);
    } , {
	maybe_plot_pixel = (d->pWidth <= 1) && (d->pStyle == SolidLine);
    });

    while (nlines--) {
	pa.point(i++, &x1, &y1);
	pa.point(i++, &x2, &y2);
	if (x1 == x2) {			// vertical
	    if (y1 < y2)
		y2++;
	    else
		y2--;
	} else if (y1 == y2) {		// horizontal
	    if (x1 < x2)
		x2++;
	    else
		x2--;
	} else if (maybe_plot_pixel) {	// draw last pixel
#ifndef Q_OS_TEMP
	    SetPixelV(d->hdc, x2, y2, pixel);
#else
	    SetPixel(d->hdc, x2, y2, pixel);
#endif
	}

#ifndef Q_OS_TEMP
        MoveToEx(d->hdc, x1, y1, 0);
	LineTo(d->hdc, x2, y2);
#else
	// PolyLine from x1, y1 to x2, y2.
	POINT linePts[2] = { { x1, y1 }, { x2, y2 } };
	Polyline(d->hdc, linePts, 2);
	internalCurrentPos = QPoint(x2, y2);
#endif
    }
}

void QWin32PaintEngine::drawPolyline(const QPointArray &a, int index, int npoints)
{
    QPointArray pa = a;
    int x1, y1, x2, y2, xsave, ysave;
    pa.point(index+npoints-2, &x1, &y1);	// last line segment
    pa.point(index+npoints-1, &x2, &y2);
    xsave = x2; ysave = y2;
    bool plot_pixel = FALSE;
    QT_WA({
	plot_pixel = (d->pWidth == 0) && (d->pStyle == SolidLine);
    } , {
	plot_pixel = (d->pWidth <= 1) && (d->pStyle == SolidLine);
    });

    if (plot_pixel) {
	if (x1 == x2) {				// vertical
	    if (y1 < y2)
		y2++;
	    else
		y2--;
	    plot_pixel = FALSE;
	} else if (y1 == y2) {			// horizontal
	    if (x1 < x2)
		x2++;
	    else
		x2--;
	    plot_pixel = FALSE;
	}
    }
    if (plot_pixel) {
	Polyline(d->hdc, (POINT*)(pa.data()+index), npoints);
#ifndef Q_OS_TEMP
	SetPixelV(d->hdc, x2, y2, d->pColor);
#else
	SetPixel(d->hdc, x2, y2, d->pColor);
#endif
    } else {
	pa.setPoint(index+npoints-1, x2, y2);
	Polyline(d->hdc, (POINT*)(pa.data()+index), npoints);
	pa.setPoint(index+npoints-1, xsave, ysave);
    }
}

void QWin32PaintEngine::drawPolygon(const QPointArray &a, bool winding, int index, int npoints)
{
    QPointArray pa = a;
#ifndef Q_OS_TEMP
    if (winding)				// set to winding fill mode
	SetPolyFillMode(d->hdc, WINDING);
#endif
    if (d->nocolBrush)
	SetTextColor(d->hdc, d->bColor);
    Polygon(d->hdc, (POINT*)(pa.data()+index), npoints);
    if (d->nocolBrush)
	SetTextColor(d->hdc, d->pColor);
#ifndef Q_OS_TEMP
    if (winding)				// set to normal fill mode
	SetPolyFillMode(d->hdc, ALTERNATE);
#endif

}

void QWin32PaintEngine::drawConvexPolygon(const QPointArray &pa, int index, int npoints)
{
    // Any efficient way?
    drawPolygon(pa,FALSE,index,npoints);

}

#ifndef QT_NO_BEZIER
void QWin32PaintEngine::drawCubicBezier(const QPointArray &a, int index)
{
    if (!isActive())
	return;
    if ((int)a.size() - index < 4) {
	qWarning("QPainter::drawCubicBezier: Cubic Bezier needs 4 control "
		 "points");
	return;
    }
    QPointArray pa(a);
#ifndef Q_OS_TEMP
    PolyBezier(d->hdc, (POINT*)(pa.data()+index), 4);
#endif
}
#endif


void QWin32PaintEngine::initialize()
{
    stock_nullPen    = (HPEN)GetStockObject(NULL_PEN);
    stock_blackPen   = (HPEN)GetStockObject(BLACK_PEN);
    stock_whitePen   = (HPEN)GetStockObject(WHITE_PEN);
    stock_nullBrush  = (HBRUSH)GetStockObject(NULL_BRUSH);
    stock_blackBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
    stock_whiteBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
    stock_sysfont    = (HFONT)GetStockObject(SYSTEM_FONT);
    init_cache();
}


void QWin32PaintEngine::cleanup()
{
    cleanup_cache();
}


void QWin32PaintEngine::drawPolyInternal(const QPointArray &a, bool close)
{
    if (!isActive())
	return;

    if (d->nocolBrush)
	SetTextColor(d->hdc, d->bColor);
    if (close) {
	Polygon(d->hdc, (POINT*)a.data(), a.size());
    } else {
	Polyline(d->hdc, (POINT*)a.data(), a.size());
    }
    if (d->nocolBrush)
	SetTextColor(d->hdc, d->pColor);
}

void QWin32PaintEngine::drawPixmap(const QRect &r, const QPixmap &pixmap, const QRect &sr, bool imask)
{
    if (!isActive())
	return;

    QPixmap *pm	  = (QPixmap*)&pixmap;
    QBitmap *mask = (QBitmap*)pm->mask();

    HDC pm_dc;
    int pm_offset;
    if (pm->isMultiCellPixmap()) {
	pm_dc = pm->multiCellHandle();
	pm_offset = pm->multiCellOffset();
    } else {
	pm_dc = pm->handle();
	pm_offset = 0;
    }

    bool stretch = r.width() != sr.width() || r.height() != sr.height();

    if (!imask && mask) {
	if (stretch) {
	    QImage imageData(pixmap);
	    QImage imageMask = imageData.createAlphaMask();
	    QBitmap tmpbm = imageMask;
	    QBitmap bm(sr.width(), sr.height());
	    {
		QPainter p(&bm);
		p.drawPixmap(QRect(0, 0, sr.width(), sr.height()), tmpbm, sr, imask);
	    }
	    QWMatrix xform = QWMatrix(r.width()/(double)sr.width(), 0,
				      0, r.height()/(double)sr.height(),
				      0, 0 );
	    bm = bm.xForm(xform);
	    QRegion region(bm);
 	    region.translate(r.x(), r.y());

	    if ( state->painter->hasClipping() )
		region &= state->painter->clipRegion();
	    state->painter->save();
	    state->painter->setClipRegion( region );
	    updateState(state);
	    StretchBlt(d->hdc, r.x(), r.y(), r.width(), r.height(),
		       pixmap.handle(), sr.x(), sr.y(), sr.width(), sr.height(),
		       SRCCOPY);
	    state->painter->restore();
	} else {
	    MaskBlt(d->hdc, r.x(), r.y(), sr.width(), sr.height(),
		    pm_dc, sr.x(), sr.y()+pm_offset,
		    mask->hbm(), sr.x(), sr.y()+pm_offset,
		    MAKEROP4(0x00aa0000, SRCCOPY));
	}
    } else {
	if (stretch)
	    StretchBlt(d->hdc, r.x(), r.y(), r.width(), r.height(),
		       pixmap.handle(), sr.x(), sr.y(), sr.width(), sr.height(),
		       SRCCOPY);
	else
	    BitBlt(d->hdc, r.x(), r.y(), sr.width(), sr.height(),
		   pixmap.handle(), sr.x(), sr.y(),
		   SRCCOPY);
    }
}

void QWin32PaintEngine::drawTextItem(const QPoint &p, const QTextItem &ti, int textFlags)
{
    ti.fontEngine->draw(this, p.x(),  p.y(), ti, textFlags);
}


HDC QWin32PaintEngine::handle() const
{
    Q_ASSERT(isActive());
    Q_ASSERT(d->hdc);
    return d->hdc;
}


void QWin32PaintEngine::updatePen(QPainterState *state)
{
    int old_pix = d->pColor;
    d->pColor = COLOR_VALUE(state->pen.color());
    d->pWidth = state->pen.width();
    d->pStyle = state->pen.style();
    bool cacheIt = (d->pStyle == PS_NULL || (d->pStyle == PS_SOLID && state->pen.width() == 0));
    HANDLE hpen_old;

    if (d->penRef) {
	release_pen(d->penRef);
	d->penRef = 0;
	hpen_old = 0;
    } else {
	hpen_old = d->hpen;
    }
    if (cacheIt) {
	if (d->pStyle == NoPen) {
	    d->hpen = stock_nullPen;
	    d->penRef = stock_ptr;
	    goto set;
	}
	if (obtain_pen(&d->penRef, &d->hpen, d->pColor))
	    goto set;
    }

    int s;

    switch (d->pStyle) {
    case NoPen:             s = PS_NULL;        break;
    case SolidLine:         s = PS_SOLID;	break;
    case DashLine:          s = PS_DASH;	break;
#ifndef Q_OS_TEMP
    case DotLine:           s = PS_DOT; 	break;
    case DashDotLine:	    s = PS_DASHDOT; 	break;
    case DashDotDotLine:    s = PS_DASHDOTDOT; 	break;
#endif
	default:
	    s = PS_SOLID;
	    qWarning("QPainter::updatePen: Invalid pen style");
    }
#ifndef Q_OS_TEMP
    if (((state->pen.width() != 0) || state->pen.width() > 1) &&
	 (qt_winver & QSysInfo::WV_NT_based || d->pStyle == SolidLine)) {
	LOGBRUSH lb;
	lb.lbStyle = 0;
	lb.lbColor = d->pColor;
	lb.lbHatch = 0;
	int pst = PS_GEOMETRIC | s;
	switch (state->pen.capStyle()) {
	    case SquareCap:
		pst |= PS_ENDCAP_SQUARE;
		break;
	    case RoundCap:
		pst |= PS_ENDCAP_ROUND;
		break;
	    case FlatCap:
		pst |= PS_ENDCAP_FLAT;
		break;
	}
	switch (state->pen.joinStyle()) {
	    case BevelJoin:
		pst |= PS_JOIN_BEVEL;
		break;
	    case RoundJoin:
		pst |= PS_JOIN_ROUND;
		break;
	    case MiterJoin:
		pst |= PS_JOIN_MITER;
		break;
	}
	d->hpen = ExtCreatePen(pst, d->pWidth, &lb, 0, 0);
    }
    else
#endif
    {
	d->hpen = CreatePen(s, state->pen.width(), d->pColor);
    }

set:
    if ( old_pix != d->pColor )
	SetTextColor( d->hdc, d->pColor );
    SelectObject(d->hdc, d->hpen);
    if (hpen_old)
	DeleteObject(hpen_old);
    return;
}


void QWin32PaintEngine::updateBrush(QPainterState *state)
{
#ifndef Q_OS_TEMP
    static short d1_pat[] = { 0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00 };
    static short d2_pat[] = { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 };
    static short d3_pat[] = { 0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11 };
    static short d4_pat[] = { 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa };
    static short d5_pat[] = { 0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee };
    static short d6_pat[] = { 0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff };
    static short d7_pat[] = { 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff };
    static short *dense_patterns[]
	= { d1_pat, d2_pat, d3_pat, d4_pat, d5_pat, d6_pat, d7_pat };
#else
    static DWORD d1_pat[]     = { 0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00 };
    static DWORD d2_pat[]     = { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 };
    static DWORD d3_pat[]     = { 0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11 };
    static DWORD d4_pat[]     = { 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa };
    static DWORD d5_pat[]     = { 0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee };
    static DWORD d6_pat[]     = { 0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff };
    static DWORD d7_pat[]     = { 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff };
    static DWORD *dense_patterns[]
	= { d1_pat, d2_pat, d3_pat, d4_pat, d5_pat, d6_pat, d7_pat };

    static DWORD hor_pat[]    = { 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff };
    static DWORD ver_pat[]    = { 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef };
    static DWORD cross_pat[]  = { 0xef, 0xef, 0xef, 0x00, 0xef, 0xef, 0xef, 0xef };
    static DWORD bdiag_pat[]  = { 0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe };
    static DWORD fdiag_pat[]  = { 0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f };
    static DWORD dcross_pat[] = { 0x7e, 0xbd, 0xdb, 0xe7, 0xe7, 0xdb, 0xbd, 0x7e };
    static DWORD *hatch_patterns[]
	= { hor_pat, ver_pat, cross_pat, bdiag_pat, fdiag_pat, dcross_pat };
#endif

    int	   bs	   = state->brush.style();
    d->bColor	   = COLOR_VALUE(state->brush.color());
    bool   cacheIt = bs == NoBrush || bs == SolidPattern;
    HBRUSH hbrush_old;

    if (d->brushRef) {
	release_brush(d->brushRef);
	d->brushRef = 0;
	hbrush_old = 0;
    } else {
	hbrush_old = d->hbrush;
    }
    if (cacheIt) {
	if (bs == NoBrush) {
	    d->hbrush = stock_nullBrush;
	    d->brushRef = stock_ptr;
	    SelectObject(d->hdc, d->hbrush);
	    if (hbrush_old) {
		DeleteObject(hbrush_old);
		if (d->hbrushbm && !d->pixmapBrush)
		    DeleteObject(d->hbrushbm);
		d->hbrushbm = 0;
		d->pixmapBrush = d->nocolBrush = FALSE;
	    }
	    return;
	}
	if (obtain_brush(&d->brushRef, &d->hbrush, d->bColor)) {
	    SelectObject(d->hdc, d->hbrush);
	    if (hbrush_old) {
		DeleteObject(hbrush_old);
		if (d->hbrushbm && !d->pixmapBrush)
		    DeleteObject(d->hbrushbm);
		d->hbrushbm = 0;
		d->pixmapBrush = d->nocolBrush = FALSE;
	    }
	    return;
	}
    }

    HBITMAP hbrushbm_old    = d->hbrushbm;
    bool    pixmapBrush_old = d->pixmapBrush;

    d->pixmapBrush = d->nocolBrush = FALSE;
    d->hbrushbm = 0;

    if (bs == SolidPattern) {			// create solid brush
	d->hbrush = CreateSolidBrush(d->bColor);
#ifndef Q_OS_TEMP
    } else if ((bs >= Dense1Pattern && bs <= Dense7Pattern) ||
		(bs == CustomPattern)) {
	if (bs == CustomPattern) {
	    // The brush pixmap can never be a multi cell pixmap
	    d->hbrushbm = state->brush.pixmap()->hbm();
	    d->pixmapBrush = TRUE;
	    d->nocolBrush = state->brush.pixmap()->depth() == 1;
	} else {
	    short *bm = dense_patterns[ bs - Dense1Pattern ];
	    d->hbrushbm = CreateBitmap(8, 8, 1, 1, bm);
	    d->nocolBrush = TRUE;
	}
	d->hbrush = CreatePatternBrush(d->hbrushbm);
	DeleteObject(d->hbrushbm);
    } else {					// one of the hatch brushes
	int s;
	switch (bs) {
	    case HorPattern:
		s = HS_HORIZONTAL;
		break;
	    case VerPattern:
		s = HS_VERTICAL;
		break;
	    case CrossPattern:
		s = HS_CROSS;
		break;
	    case BDiagPattern:
		s = HS_BDIAGONAL;
		break;
	    case FDiagPattern:
		s = HS_FDIAGONAL;
		break;
	    case DiagCrossPattern:
		s = HS_DIAGCROSS;
		break;
	    default:
		s = HS_HORIZONTAL;
	}
	d->hbrush = CreateHatchBrush(s, d->bColor);
    }
#else
    } else {
	if (bs == CustomPattern) {
	    // The brush pixmap can never be a multi cell pixmap
	    d->hbrushbm = state->brush.pixmap()->hbm();
	    d->pixmapBrush = TRUE;
	    d->nocolBrush = state->brush.pixmap()->depth() == 1;
	    d->hbrush = CreatePatternBrush(d->hbrushbm);
	    DeleteObject(d->hbrushbm);
	} else {
	    struct {
		BITMAPINFOHEADER bmi;
		COLORREF palette[2];
		DWORD bitmapData[8];

	    } bitmapBrush;

	    memset(&bitmapBrush, 0, sizeof(bitmapBrush));

	    bitmapBrush.bmi.biSize     = sizeof(BITMAPINFOHEADER);
	    bitmapBrush.bmi.biWidth    =
	    bitmapBrush.bmi.biHeight   = 8;
	    bitmapBrush.bmi.biPlanes   =
	    bitmapBrush.bmi.biBitCount = 1;
	    bitmapBrush.bmi.biClrUsed  = 0;
	    QRgb *coltbl = (QRgb*)bitmapBrush.palette;
	    coltbl[0] = d->bColor.rgb();
	    coltbl[1] = Qt::color0.rgb();

	    static DWORD *pattern = hatch_patterns[ 0 ]; // HorPattern

	    if (bs >= Dense1Pattern && bs <= Dense7Pattern)
		pattern = dense_patterns[ bs - Dense1Pattern ];
	    else if (bs >= HorPattern && bs <= DiagCrossPattern)
		pattern = hatch_patterns[ bs - HorPattern ];

	    memcpy(bitmapBrush.bitmapData, pattern, 64);
	    d->hbrush = CreateDIBPatternBrushPt(&bitmapBrush, DIB_RGB_COLORS);
	}
    }
#endif
    SetBrushOrgEx(d->hdc, state->bgOrigin.x(), state->bgOrigin.y(), 0);
    SelectObject(d->hdc, d->hbrush);

    if (hbrush_old) {
	DeleteObject(hbrush_old);		// delete last brush
	if (hbrushbm_old && !pixmapBrush_old)
	    DeleteObject(hbrushbm_old);	// delete last brush pixmap
    }

    SetBkColor(d->hdc, COLOR_VALUE(state->bgBrush.color()));
}


void QWin32PaintEngine::updateRasterOp(QPainterState *state)
{
    Q_ASSERT(isActive());
    SetROP2(d->hdc, rasterOpCodes[state->rasterOp]);
    d->rasterOp = state->rasterOp;
}


void QWin32PaintEngine::updateBackground(QPainterState *state)
{
    Q_ASSERT(isActive());

    SetBkColor(d->hdc, COLOR_VALUE(state->bgBrush.color()));
    SetBkMode(d->hdc, state->bgMode == TransparentMode ? TRANSPARENT : OPAQUE);
}


void QWin32PaintEngine::updateXForm(QPainterState *state)
{
#ifdef NO_NATIVE_XFORM
    return;
#endif

    XFORM m;

//     if (state->WxF) {
	QWMatrix mtx;
// 	if (state->VxF) {
// 	    mtx.translate( state->vx, state->vy );
// 	    mtx.scale( 1.0*state->vw/state->ww, 1.0*state->vh/state->wh );
// 	    mtx.translate( -state->wx, -state->wy );
// 	    mtx = state->worldMatrix * mtx;
// 	} else {
	mtx = state->matrix;
// 	}
	m.eM11 = mtx.m11();
	m.eM12 = mtx.m12();
	m.eM21 = mtx.m21();
	m.eM22 = mtx.m22();
	m.eDx  = mtx.dx();
	m.eDy  = mtx.dy();
	SetGraphicsMode(d->hdc, GM_ADVANCED);
	if (!SetWorldTransform(d->hdc, &m)) {
	    printf("QWin32PaintEngine::updateXForm(), SetWorldTransformation() failed.. %d\n", GetLastError());
	}
//     } else {
// 	m.eM11 = m.eM22 = (float)1.0;
// 	m.eM12 = m.eM21 = m.eDx = m.eDy = (float)0.0;
// 	SetGraphicsMode(d->hdc, GM_ADVANCED);
// 	ModifyWorldTransform(d->hdc, &m, MWT_IDENTITY);
// 	SetGraphicsMode(d->hdc, GM_COMPATIBLE);
//     }
}


void QWin32PaintEngine::updateClipRegion(QPainterState *state)
{
    if (!isActive())
	return;

//     if ( !isActive() || enable == testf(ClipOn) )
// 	return;

    if (state->clipEnabled) {
	QRegion rgn = state->clipRegion;
// 	rgn.translate(-redirection_offset);
// 	if ( pdev == dirty_hack_paintDevice() )
// 	rgn = rgn.intersect( *(QPainter::dirty_hack_paintRegion()) );

	if (state->VxF || state->WxF) {
	    if (state->txop == QPainter::TxTranslate)
		rgn.translate(state->worldMatrix.dx(), state->worldMatrix.dy());
	    else
		rgn = state->worldMatrix * rgn;
	}

	// Setting an empty region as clip region on Win just dmainisables clipping completely.
	// To workaround this and get the same semantics on Win and Unix, we set a 1x1 pixel
	// clip region far out in coordinate space in this case.
	if ( rgn.isEmpty() )
	    rgn = QRect(-0x1000000, -0x1000000, 1, 1);
	SelectClipRgn(d->hdc, rgn.handle());
    }
    else {
// 	if (pdev == paintEventDevice)
// 	SelectClipRgn(d->hdc, paintEventClipRegion->handle());
// 	else
	    SelectClipRgn(d->hdc, 0);
    }

}

void QWin32PaintEngine::updateFont(QPainterState *state)
{
    QFont &f = state->font;
    d->hfont = f.handle();
    SelectObject( d->hdc, d->hfont );

    d->fontFlags = 0;
    if (f.underline()) d->fontFlags |= Qt::Underline;
    if (f.overline()) d->fontFlags |= Qt::Overline;
    if (f.strikeOut()) d->fontFlags |= Qt::StrikeOut;
}

/*
  This function adjusts the raster operations for painting into a QBitmap on
  Windows.

  For bitmaps und Windows, color0 is 0xffffff and color1 is 0x000000 -- so we
  have to use adjusted ROPs in this case to get the same effect as on Unix.
*/
Qt::RasterOp qt_map_rop_for_bitmaps( Qt::RasterOp r )
{
    static const Qt::RasterOp ropCodes[] = {
	Qt::CopyROP,	// CopyROP
	Qt::AndROP,		// OrROP
	Qt::NotXorROP,	// XorROP
	Qt::NotOrROP,	// NotAndROP
	Qt::NotCopyROP,	// NotCopyROP
	Qt::NotAndROP,	// NotOrROP
	Qt::XorROP,	// NotXorROP
	Qt::OrROP,	// AndROP
	Qt::NotROP,	// NotROP
	Qt::SetROP,	// ClearROP
	Qt::ClearROP,	// SetROP
	Qt::NopROP,	// NopROP
	Qt::OrNotROP,	// AndNotROP
	Qt::AndNotROP,	// OrNotROP
	Qt::NorROP,	// NandROP
	Qt::NandROP	// NorROP
    };
    return ropCodes[r];
}

extern void qt_fill_tile( QPixmap *tile, const QPixmap &pixmap );
extern void qt_draw_tile(QPaintEngine *, int, int, int, int, const QPixmap &, int, int);

void QWin32PaintEngine::drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &p, bool)
{

    QBitmap *mask = (QBitmap *)pixmap.mask();

    int sw = pixmap.width();
    int sh = pixmap.height();

    if ( sw*sh < 8192 && sw*sh < 16*r.width()*r.height() ) {
	int tw = sw, th = sh;
	while ( tw*th < 32678 && tw < r.width()/2 )
	    tw *= 2;
	while ( tw*th < 32678 && th < r.height()/2 )
	    th *= 2;
	QPixmap tile( tw, th, pixmap.depth(), QPixmap::BestOptim );
	qt_fill_tile( &tile, pixmap );
	if ( mask ) {
	    QBitmap tilemask( tw, th, FALSE, QPixmap::NormalOptim );
	    qt_fill_tile( &tilemask, *mask );
	    tile.setMask( tilemask );
	}
	qt_draw_tile( this, r.x(), r.y(), r.width(), r.height(), tile, p.x(), p.y() );
    } else {
	qt_draw_tile( this, r.x(), r.y(), r.width(), r.height(), pixmap, p.x(), p.y() );
    }
}

#if defined (QT_GDIPLUS_SUPPORT)
/*******************************************************************************
 *
 * And thus begindeth the GDI+ paint engine
 *
 ******************************************************************************/
using namespace Gdiplus;

static const DashStyle qt_penstyle_map[] = {
    { (DashStyle)-1 },		// Qt::NoPen
    { DashStyleSolid },		// Qt::SolidLine
    { DashStyleDash },		// Qt::DashLine
    { DashStyleDot },		// Qt::DotLine
    { DashStyleDashDot },	// Qt::DashDotLine
    { DashStyleDashDotDot }	// Qt::DashDotDotLine
};

static const HatchStyle qt_hatchstyle_map[] = {
    { (HatchStyle) -1 }, 		// Qt::NoBrush
    { (HatchStyle) -1 },     		// Qt::SolidPattern
    { HatchStyle90Percent },    	// Qt::Dense1Pattern
    { HatchStyle75Percent },    	// Qt::Dense2Pattern
    { HatchStyle60Percent },    	// Qt::Dense3Pattern
    { HatchStyle50Percent },    	// Qt::Dense4Pattern
    { HatchStyle30Percent },    	// Qt::Dense5Pattern
    { HatchStyle20Percent },    	// Qt::Dense6Pattern
    { HatchStyle05Percent },   		// Qt::Dense7Pattern
    { HatchStyleHorizontal },   	// Qt::HorPattern
    { HatchStyleVertical },		// Qt::VerPattern
    { HatchStyleCross },   		// Qt::CrossPattern
    { HatchStyleBackwardDiagonal },   	// Qt::BDiagPattern
    { HatchStyleForwardDiagonal },   	// Qt::FDiagPattern
    { HatchStyleDiagonalCross }   	// Qt::DiagCrossPattern
};

static inline Color conv(const QColor &c) { return Color(c.red(), c.green(), c.blue()); }
static inline Point conv(const QPoint &p) { return Point(p.x(), p.y()); }
static inline Rect conv(const QRect &r) { return Rect(r.x(), r.y(), r.width(), r.height()); }

QGdiplusPaintEngine::QGdiplusPaintEngine(QPaintDevice *dev)
    : QPaintEngine(*(new QGdiplusPaintEnginePrivate),
		   GCCaps(CoordTransform
			  | PenWidthTransform
			  | PatternTransform
			  | PixmapTransform
			  | LinearGradientSupport))

{
    d->pdev = dev;
}

QGdiplusPaintEngine::~QGdiplusPaintEngine()
{
}

/* Start painting for this device.
 */
bool QGdiplusPaintEngine::begin(QPaintDevice *pdev, QPainterState *, bool)
{
    Q_ASSERT(d->pdev == pdev);
    // Verify the presence of an HDC
    if (pdev->devType() == QInternal::Widget) {
	d->hdc = pdev->handle();
	QWidget *widget = static_cast<QWidget*>(pdev);
	if (!d->hdc) {
	    d->hdc = GetDC(widget->winId());
	    d->usesTempDC = true;
 	}
    }

    d->graphics = new Graphics(d->hdc);
    Q_ASSERT(d->graphics);

    d->pen = new Pen(Color(0, 0, 0), 0);

    setActive(true);

    return true;
}

bool QGdiplusPaintEngine::end()
{
    delete d->graphics;
    delete d->pen;
    delete d->brush;
    if (d->cachedSolidBrush != d->brush) {
	delete d->cachedSolidBrush;
    }

    d->graphics = 0;
    d->pen = 0;
    d->cachedSolidBrush = 0;
    d->brush = 0;

    if (d->pdev->devType() == QInternal::Widget) {
	QWidget *widget = static_cast<QWidget*>(d->pdev);
	if (!d->usesTempDC) {
	    ReleaseDC(widget->winId(), d->hdc);
	    d->usesTempDC = false;
 	}
    }

    return true;
}

void QGdiplusPaintEngine::updatePen(QPainterState *ps)
{
    d->pen->SetWidth(ps->pen.width());
    d->pen->SetColor(conv(ps->pen.color()));

    Qt::PenStyle style = ps->pen.style();
    if (style == Qt::NoPen) {
	d->usePen = false;
    } else {
	Q_ASSERT(style >= 0 && style < 5);
	d->usePen = true;
	d->pen->SetDashStyle(qt_penstyle_map[style]);
    }
}

void QGdiplusPaintEngine::updateBrush(QPainterState *ps)
{
    QColor c = ps->brush.color();
    if (d->temporaryBrush) {
	d->temporaryBrush = false;
	delete d->brush;
    }

    switch (ps->brush.style()) {
    case Qt::NoBrush:
	d->brush = 0;
	break;
    case Qt::SolidPattern:
	if (!d->cachedSolidBrush) {
	    d->cachedSolidBrush = new SolidBrush(conv(ps->brush.color()));
	    d->brush = d->cachedSolidBrush;
	} else {
	    d->cachedSolidBrush->SetColor(conv(ps->brush.color()));
	    d->brush = d->cachedSolidBrush;
	}
	break;
    case Qt::LinearGradientPattern: {
	QBrush &b = ps->brush;
	d->brush = new LinearGradientBrush(conv(b.gradientStart()), conv(b.gradientStop()),
					   conv(b.color()), conv(b.gradientColor()));
	d->temporaryBrush = true;
	break;
    }
    case Qt::CustomPattern: {
	QPixmap *pm = ps->brush.pixmap();
	if (pm) {
	    Bitmap *bm = new Bitmap(pm->hbm(), (HPALETTE)0);
	    d->brush = new TextureBrush(bm, WrapModeTile);
	    d->temporaryBrush = true;
	}
	break;
    }
    default: // HatchBrush
	Q_ASSERT(ps->brush.style() > Qt::SolidPattern && ps->brush.style() < Qt::CustomPattern);
	d->brush = new HatchBrush(qt_hatchstyle_map[ps->brush.style()],
				  conv(ps->brush.color()),
				  conv(ps->bgBrush.color()));
	d->graphics->SetRenderingOrigin(state->bgOrigin.x(), state->bgOrigin.y());
	d->temporaryBrush = true;
    }
}

void QGdiplusPaintEngine::updateFont(QPainterState *)
{

}

void QGdiplusPaintEngine::updateRasterOp(QPainterState *)
{
}

void QGdiplusPaintEngine::updateBackground(QPainterState *)
{
}

void QGdiplusPaintEngine::updateXForm(QPainterState *ps)
{
    QWMatrix &qm = ps->matrix;
    Matrix m(qm.m11(), qm.m12(), qm.m21(), qm.m22(), qm.dx(), qm.dy());
    d->graphics->SetTransform(&m);
}

void QGdiplusPaintEngine::updateClipRegion(QPainterState *ps)
{
    if (ps->clipEnabled) {
	Region r(ps->clipRegion.handle());
	d->graphics->SetClip(&r, CombineModeReplace);
    } else {
	d->graphics->ResetClip();
    }
}

HDC QGdiplusPaintEngine::handle() const
{
    return 0;
}

void QGdiplusPaintEngine::drawLine(const QPoint &p1, const QPoint &p2)
{
    if (d->usePen) {
	d->graphics->DrawLine(d->pen, p1.x(), p1.y(), p2.x(), p2.y());
    }
}

void QGdiplusPaintEngine::drawRect(const QRect &r)
{
    int subtract = d->usePen ? 1 : 0;
    if (d->brush) {
	d->graphics->FillRectangle(d->brush, r.x(), r.y(),
				   r.width()-subtract, r.height()-subtract);
    }
    if (d->usePen) {
 	d->graphics->DrawRectangle(d->pen, r.x(), r.y(),
				   r.width()-subtract, r.height()-subtract);
    }
}

void QGdiplusPaintEngine::drawPoint(const QPoint &p)
{
    if (d->usePen)
	d->graphics->DrawRectangle(d->pen, p.x(), p.y(), 0, 0);
}

void QGdiplusPaintEngine::drawPoints(const QPointArray &pa, int index, int npoints)
{
    if (d->usePen)
	for (int i=0; i<npoints; ++i)
	    d->graphics->DrawRectangle(d->pen, pa[index+i].x(), pa[index+i].y(), 0, 0);
}

void QGdiplusPaintEngine::drawWinFocusRect(const QRect &r, bool, const QColor &)
{
    Pen pen(Color(0, 0, 0), 0);
    pen.SetDashStyle(DashStyleDot);
    d->graphics->DrawRectangle(&pen, r.x(), r.y(), r.width()-1, r.height()-1);
}

void QGdiplusPaintEngine::drawRoundRect(const QRect &r, int xRnd, int yRnd)
{
    GraphicsPath path(FillModeAlternate);

    int subtract = d->usePen ? 1 : 0;

    int top = r.y();
    int bottom = r.y() + r.height() - subtract;
    int left = r.x();
    int right = r.x() + r.width() - subtract;

    int horLength = (99 - xRnd) / 99.0 * r.width() / 1;
    int horStart  = r.x() + r.width() / 2 - horLength / 2;
    int horEnd = horStart + horLength;
    int arcWidth = r.width() - horLength;

    int verLength = (99 - yRnd) / 99.0 * r.height() / 1;
    int verStart  = r.y() + r.height() / 2 - verLength / 2;
    int verEnd = verStart + verLength;
    int arcHeight = r.width() - horLength;

    path.AddLine(horStart, r.y(), horEnd, r.y());
    path.AddArc(right - arcWidth, top, arcWidth, arcHeight, 270, 90);
    path.AddLine(right, verStart, right, verEnd);
    path.AddArc(right - arcWidth, bottom - arcHeight, arcWidth, arcHeight, 0, 90);
    path.AddLine(horEnd, bottom, horStart, bottom);
    path.AddArc(left, bottom - arcHeight, arcWidth, arcHeight, 90, 90);
    path.AddLine(left, verEnd, left, verStart);
    path.AddArc(left, top, arcWidth, arcHeight, 180, 90);
    path.CloseFigure();
    if (d->brush)
	d->graphics->FillPath(d->brush, &path);
    if (d->usePen)
	d->graphics->DrawPath(d->pen, &path);
}

void QGdiplusPaintEngine::drawEllipse(const QRect &r)
{
    int subtract = d->usePen ? 1 : 0;
    if (d->brush)
	d->graphics->FillEllipse(d->brush, r.x(), r.y(), r.width()-subtract, r.height()-subtract);
    if (d->usePen)
	d->graphics->DrawEllipse(d->pen, r.x(), r.y(), r.width()-subtract, r.height()-subtract);
}

void QGdiplusPaintEngine::drawArc(const QRect &r, int a, int alen)
{
    int subtract = d->usePen ? 1 : 0;
    if (d->usePen) {
	d->graphics->DrawArc(d->pen, r.x(), r.y(),
			     r.width()-subtract, r.height()-subtract,
			     -a/16.0, -alen/16.0);
    }
}

void QGdiplusPaintEngine::drawPie(const QRect &r, int a, int alen)
{
    int subtract = d->usePen ? 1 : 0;
    if (d->brush) {
	d->graphics->FillPie(d->brush, r.x(), r.y(),
			     r.width()-subtract, r.height()-subtract,
			     -a/16.0, -alen/16.0);
    }
    if (d->usePen) {
	d->graphics->DrawPie(d->pen, r.x(), r.y(),
			     r.width()-subtract, r.height()-subtract,
			     -a/16.0, -alen/16.0);
    }
}

void QGdiplusPaintEngine::drawChord(const QRect &r, int a, int alen)
{
    GraphicsPath path(FillModeAlternate);
    int subtract = d->usePen ? 1 : 0;
    path.AddArc(r.x(), r.y(), r.width()-subtract, r.height()-subtract, -a/16.0, -alen/16.0);
    path.CloseFigure();
    if (d->brush)
	d->graphics->FillPath(d->brush, &path);
    if (d->usePen)
	d->graphics->DrawPath(d->pen, &path);
}

void QGdiplusPaintEngine::drawLineSegments(const QPointArray &pa, int index, int nlines)
{
    if (d->usePen) {
	GraphicsPath path;
	for (int i=0; i<nlines*2; i+=2) {
	    path.AddLine(pa[index+i].x(), pa[index+i].y(), pa[index+i+1].x(), pa[index+i+1].y());
	    path.CloseFigure();
	}
	d->graphics->DrawPath(d->pen, &path);
    }
}

void QGdiplusPaintEngine::drawPolyline(const QPointArray &pa, int index, int npoints)
{
    if (d->usePen) {
	GraphicsPath path;
	for (int i=1; i<npoints; ++i)
	    path.AddLine(pa.at(index+i-1).x(), pa.at(index+i-1).y(),
			 pa.at(index+i).x(), pa.at(index+i).y());
	d->graphics->DrawPath(d->pen, &path);
    }
}

void QGdiplusPaintEngine::drawPolygon(const QPointArray &pa, bool winding, int index, int npoints)
{
    if (d->usePen || d->brush) {
	Point *p = new Point[npoints];
	for (int i=0; i<npoints; ++i)
	    p[i] = Point(pa[index+i].x(), pa[index+i].y());
	if (d->usePen)
	    d->graphics->DrawPolygon(d->pen, p, npoints);
	if (d->brush)
	    d->graphics->FillPolygon(d->brush, p, npoints,
				     winding ? FillModeWinding : FillModeAlternate);
	delete [] p;
    }
}

void QGdiplusPaintEngine::drawConvexPolygon(const QPointArray &pa, int index, int npoints)
{
    drawPolygon(pa, index, npoints);
}



void QGdiplusPaintEngine::drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr, bool imask)
{
    if (!pm.hasAlpha()) {
	Bitmap bitmap(pm.hbm(), HPALETTE(0));
	d->graphics->DrawImage(&bitmap,
			       Rect(r.x(), r.y(), r.width(), r.height()),
			       sr.x(), sr.y(), sr.width(), sr.height(), UnitPixel);
    } else { // 1 bit masks or 8 bit alpha...
	QImage image = pm.convertToImage();
	PixelFormat pf;
	int depth = image.depth();

	if (depth == 32) {
	    pf = PixelFormat32bppARGB;
	} else if (depth = 16) {
	    pf = PixelFormat16bppARGB1555;
	} else if (depth = 8) {
	    image = image.convertDepth(16);
	    pf = PixelFormat16bppARGB1555;
	    return;
	} else {
	    qDebug() << "QGdiplusPaintEngine::drawPixmap(), unsupported depth:" << image.depth();
	    return;
	}

	Bitmap bitmap(pm.width(), pm.height(), image.bytesPerLine(), pf,
	       image.bits());
	d->graphics->DrawImage(&bitmap,
			       Rect(r.x(), r.y(), r.width(), r.height()),
			       sr.x(), sr.y(), sr.width(), sr.height(), UnitPixel);
    }
}

void QGdiplusPaintEngine::drawTextItem(const QPoint &, const QTextItem &, int)
{
}

void QGdiplusPaintEngine::drawTiledPixmap(const QRect &r, const QPixmap &pm,
					  const QPoint &, bool)
{
    QImage image = pm.convertToImage();
    Q_ASSERT(image.depth() == 32);
    Bitmap bitmap(pm.width(), pm.height(), image.bytesPerLine(), PixelFormat32bppARGB,
		  image.bits());
    TextureBrush texture(&bitmap, WrapModeTile);
    texture.TranslateTransform(r.x(), r.y());
    int subtract = d->usePen ? 1 : 0;
    d->graphics->FillRectangle(&texture, r.x(), r.y(), r.width()-subtract, r.height()-subtract);
}

#ifndef QT_NO_BEZIER
void QGdiplusPaintEngine::drawCubicBezier(const QPointArray &pa, int index)
{
    if (d->usePen) {
	Point *p = new Point[pa.size()];
	for (int i=0; i<pa.size() - index; ++i) {
	    p[i] = Point(pa[i+index].x(), pa[i+index].y());
	}
	if (d->usePen)
	    d->graphics->DrawCurve(d->pen, p, pa.size());
	delete [] p;
    }
}
#endif

QPainter::RenderHints QGdiplusPaintEngine::supportedRenderHints() const
{
    return QPainter::LineAntialiasing;
}

QPainter::RenderHints QGdiplusPaintEngine::renderHints() const
{
    QPainter::RenderHints hints;
    if (d->antiAliasEnabled)
	hints |= QPainter::LineAntialiasing;
    return hints;
}

void QGdiplusPaintEngine::setRenderHint(QPainter::RenderHint hint, bool enable)
{
    if (hint & QPainter::LineAntialiasing) {
	d->graphics->SetSmoothingMode(enable ? SmoothingModeHighQuality : SmoothingModeHighSpeed);
// 	d->graphics->SetPixelOffsetMode(enable ? PixelOffsetModeHalf : PixelOffsetModeNone );
	d->antiAliasEnabled = enable;
    }
}


/* Some required initialization of GDI+ needed prior to
   doing anything GDI+ related. Called by qt_init() in
   qapplication_win.cpp
*/
static GdiplusStartupInput *gdiplusStartupInput = 0;
static ULONG_PTR gdiplusToken = 0;
void QGdiplusPaintEngine::initialize()
{
    Q_ASSERT(!gdiplusStartupInput);

    gdiplusStartupInput = new GdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, gdiplusStartupInput, NULL);
}

void QGdiplusPaintEngine::cleanup()
{
    GdiplusShutdown(gdiplusToken);
}

#endif // QT_GDIPLUS_SUPPORT

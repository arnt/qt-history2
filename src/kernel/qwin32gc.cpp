#include "q4painter_p.h"
#include "qbitmap.h"
#include "qbrush.h"
#include "qfontengine_p.h"
#include "qpaintdevice.h"
#include "qpaintdevice.h"
#include "qpainter.h"
#include "qpen.h"
#include "qpixmap.h"
#include "qt_windows.h"
#include "qtextengine_p.h"
#include "qtextlayout_p.h"
#include "qwidget.h"
#include "qwin32gc.h"
#include "qwin32gc_p.h"

#include <math.h>

// #define NO_NATIVE_XFORM

#define MY_DEBUG() printf("%s:%d\n", __FILE__, __LINE__);

static int qt_winver = Qt::WV_NT;

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

//  #define CACHE_STAT
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

QWin32GC::QWin32GC(const QPaintDevice *target)
    :
#ifndef NO_NATIVE_XFORM
      QAbstractGC( GCCaps(CoordTransform | PenWidthTransform | PixmapTransform) ),
#endif
      d(new QWin32GCPrivate)
{
    // ### below is temp hack to survive pixmap gc construction
    d->hwnd = target ? ((QWidget*)target)->winId() : 0;
    d->flags |= IsStartingUp;
}


QWin32GC::~QWin32GC()
{
    delete d;
}


bool QWin32GC::begin(const QPaintDevice *pdev, QPainterState *state, bool clipEnabled)
{
    Q_ASSERT(pdev->devType()==QInternal::Widget);
    if (isActive()) {				// already active painting
	qWarning("QWin32GC::begin: Painter is already active."
	       "\n\tYou must end() the painter before a second begin()\n");
// 	return true;
    }

    setActive(true);
    d->hdc = GetDC(((QWidget*)pdev)->winId());
    d->device = const_cast<QPaintDevice *>(pdev);
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
    return true;
}

bool QWin32GC::end()
{
    if ( !isActive() ) {
	qWarning( "QWin32GC::end: Missing begin() or begin() failed" );
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
//     if ( d->hfont ) {
// 	SelectObject( d->hdc, d->stock_sysfont );
// 	d->hfont = 0;
//     }
    if ( d->holdpal ) {
	SelectPalette( d->hdc, d->holdpal, true );
	RealizePalette( d->hdc );
    }

    // ### use stuff below...
    if (d->device->devType() == QInternal::Widget) {
	ReleaseDC(static_cast<QWidget*>(d->device)->winId(), d->hdc);
    }

//     if ( pdev->devType() == QInternal::Widget ) {
// 	if (!usesWidgetDC) {
// 	    QWidget *w = (QWidget*)pdev;
// 	    ReleaseDC( w->isDesktop() ? 0 : w->winId(), hdc );
// 	    w->hdc = 0;
// 	}
//     }
//     else if ( pdev->devType() == QInternal::Pixmap ) {
// 	QPixmap *pm = (QPixmap*)pdev;
// 	if ( pm->optimization() == QPixmap::MemoryOptim &&
// 	     ( qt_winver & WV_DOS_based ) )
// 	    pm->allocCell();
//     }

//     if ( d->pfont ) {
// 	delete d->pfont;
// 	d->pfont = 0;
//     }

    if (GetGraphicsMode(d->hdc)==GM_ADVANCED) {
	if (!ModifyWorldTransform(d->hdc, 0, MWT_IDENTITY))
	    qWarning("QWin32GC::end(). ModifyWorldTransform failed: code: %d\n", GetLastError());
	SetGraphicsMode(d->hdc, GM_COMPATIBLE);
    }

    d->hdc = 0;

    setActive(false);
    return true;
}

void QWin32GC::drawLine(int x1, int y1, int x2, int y2)
{
    Q_ASSERT(isActive());

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

void QWin32GC::drawRect(int x, int y, int w, int h)
{
    Q_ASSERT(isActive());

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
	Rectangle(d->hdc, x, y, x+w, y+h);
	SetTextColor(d->hdc, d->pColor);
    } else {
	Rectangle(d->hdc, x, y, x+w, y+h);
    }
}

void QWin32GC::drawPoint(int x, int y)
{
    Q_ASSERT(isActive());

    if (d->pStyle != NoPen)
#ifndef Q_OS_TEMP
	SetPixelV(d->hdc, x, y, d->pColor);
#else
	SetPixel(d->hdc, x, y, d->pColor);
#endif
}

void QWin32GC::drawPoints(const QPointArray &pts, int index, int npoints)
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

void QWin32GC::drawWinFocusRect(int x, int y, int w, int h, bool, const QColor &bgColor)
{
    if (!isActive())
	return;
    Q_ASSERT(d->hdc);

    RECT r;
    r.left   = x;
    r.right  = x + w;
    r.top    = y;
    r.bottom = y + h;

    if (qGray(bgColor.rgb()) < 10) { // Use white pen for very dark colors
	int col = GetBkColor(d->hdc);
 	SetBkColor(d->hdc, col /* ^0x00ffffff */ );
	DrawFocusRect(d->hdc, &r);
	SetBkColor(d->hdc, col);
    } else {
	DrawFocusRect(d->hdc, &r);
    }
}

void QWin32GC::drawRoundRect(int x, int y, int w, int h, int xRnd, int yRnd)
{
    Q_ASSERT(isActive());

    if (xRnd <= 0 || yRnd <= 0) {
	drawRect(x, y, w, h);			// draw normal rectangle
	return;
    }
    if (xRnd >= 100)				// fix ranges
	xRnd = 99;
    if (yRnd >= 100)
	yRnd = 99;

    if (d->nocolBrush)
	SetTextColor(d->hdc, d->bColor);
    RoundRect(d->hdc, x, y, x+w, y+h, w*xRnd/100, h*yRnd/100);
    if (d->nocolBrush)
	SetTextColor(d->hdc, d->pColor);
}

void QWin32GC::drawEllipse(int x, int y, int w, int h)
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
    if (w == 1 && h == 1)
	drawPoint(x, y);
    else
	Ellipse(d->hdc, x, y, x+w, y+h);
    if (d->nocolBrush)
	SetTextColor(d->hdc, d->pColor);

//     if (oldPen == PS_NULL)
// 	setPen(oldPen);
}

void QWin32GC::drawArc(int x, int y, int w, int h, int a, int alen)
{
    Q_ASSERT(isActive());

    double ra1 = 1.09083078249645598e-3 * a;
    double ra2 = 1.09083078249645598e-3 * alen + ra1;
    if (alen < 0.0) {				// swap angles
	double t = ra1;
	ra1 = ra2;
	ra2 = t;
    }
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
    int xS = qRound(w2 + (cos(ra1)*w2) + x);
    int yS = qRound(h2 - (sin(ra1)*h2) + y);
    int xE = qRound(w2 + (cos(ra2)*w2) + x);
    int yE = qRound(h2 - (sin(ra2)*h2) + y);
    if (QABS(alen) < 90*16) {
	if ((xS == xE) && (yS == yE)) {
	    // don't draw a whole circle
	    return; //### should we draw a point?
	}
    }
#ifndef Q_OS_TEMP
    if (d->rasterOp == CopyROP) {
        Arc(d->hdc, x, y, x+w, y+h, xS, yS, xE, yE);
    } else
#endif
    {
	QPointArray pa;
	pa.makeArc(x, y, w, h, a, alen);	// arc polyline
	drawPolyInternal(pa, FALSE);
    }
}

void QWin32GC::drawPie(int x, int y, int w, int h, int a, int alen)
{
    Q_ASSERT(isActive());

    double ra1 = 1.09083078249645598e-3 * a;
    double ra2 = 1.09083078249645598e-3 * alen + ra1;
    if (alen < 0.0) {				// swap angles
	double t = ra1;
	ra1 = ra2;
	ra2 = t;
    }
    double w2 = 0.5*w;
    double h2 = 0.5*h;
    int xS = qRound(w2 + (cos(ra1)*w) + x);
    int yS = qRound(h2 - (sin(ra1)*h) + y);
    int xE = qRound(w2 + (cos(ra2)*w) + x);
    int yE = qRound(h2 - (sin(ra2)*h) + y);
    if (QABS(alen) < 90*16) {
	if ((xS == xE) && (yS == yE)) {
	    // don't draw a whole circle
	    return; //### should we draw something?
	}
    }
    if (d->nocolBrush)
	SetTextColor(d->hdc, d->bColor);
#ifndef Q_OS_TEMP
    Pie(d->hdc, x, y, x+w, y+h, xS, yS, xE, yE);
#endif
    if (d->nocolBrush)
	SetTextColor(d->hdc, d->pColor);
}

void QWin32GC::drawChord(int x, int y, int w, int h, int a, int alen)
{
    Q_ASSERT(isActive());

    double ra1 = 1.09083078249645598e-3 * a;
    double ra2 = 1.09083078249645598e-3 * alen + ra1;
    if (ra2 < 0.0) {				// swap angles
	double t = ra1;
	ra1 = ra2;
	ra2 = t;
    }
    double w2 = 0.5*w;
    double h2 = 0.5*h;
    int xS = qRound(w2 + (cos(ra1)*w) + x);
    int yS = qRound(h2 - (sin(ra1)*h) + y);
    int xE = qRound(w2 + (cos(ra2)*w) + x);
    int yE = qRound(h2 - (sin(ra2)*h) + y);
    if (QABS(alen) < 90*16) {
	if ((xS == xE) && (yS == yE)) {
	    // don't draw a whole circle
	    return; //### should we draw something?
	}
    }
    if (d->nocolBrush)
	SetTextColor(d->hdc, d->bColor);
#ifndef Q_OS_TEMP
    Chord(d->hdc, x, y, x+w, y+h, xS, yS, xE, yE);
#endif
    if (d->nocolBrush)
	SetTextColor(d->hdc, d->pColor);
}


void QWin32GC::drawLineSegments(const QPointArray &a, int index, int nlines)
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

void QWin32GC::drawPolyline(const QPointArray &a, int index, int npoints)
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

void QWin32GC::drawPolygon(const QPointArray &a, bool winding, int index, int npoints)
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

void QWin32GC::drawConvexPolygon(const QPointArray &pa, int index, int npoints)
{
    // Any efficient way?
    drawPolygon(pa,FALSE,index,npoints);

}

#ifndef QT_NO_BEZIER
void QWin32GC::drawCubicBezier(const QPointArray &a, int index)
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


void QWin32GC::initialize()
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


void QWin32GC::drawPolyInternal(const QPointArray &a, bool close)
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

void QWin32GC::drawPixmap( int x, int y, const QPixmap &pixmap,
			   int sx, int sy, int sw, int sh )
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

    if (mask)
	MaskBlt(d->hdc, x, y, sw, sh,
		pm_dc, sx, sy+pm_offset,
		mask->hbm(), sx, sy+pm_offset,
		MAKEROP4(0x00aa0000, SRCCOPY));
    else
	BitBlt(d->hdc, x, y, sw, sh, pixmap.handle(), sx, sy, SRCCOPY);
}

void QWin32GC::drawTextItem(int x, int y, const QTextItem &ti, int textFlags)
{
    return;

    QTextEngine *engine = ti.engine();
    QScriptItem *si = &engine->items[ti.item()];

    engine->shape( ti.item() );
    QFontEngine *fe = si->fontEngine;
    Q_ASSERT( fe );

    x += si->x;
    y += si->y;

    HDC oldDC = fe->hdc;
    fe->hdc = d->hdc;
    SelectObject( d->hdc, fe->hfont );
//     fe->draw( this, x,  y, engine, si, textFlags );
}


HDC QWin32GC::handle() const
{
    Q_ASSERT(isActive());
    Q_ASSERT(d->hdc);
    return d->hdc;
}


void QWin32GC::updatePen(QPainterState *state)
{
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
	    SelectObject(d->hdc, d->hpen);
	    SetTextColor(d->hdc, d->pColor);
	    if (hpen_old)
		DeleteObject(hpen_old);
	    return;
	}
	if (obtain_pen(&d->penRef, &d->hpen, d->pColor)) {
	    SelectObject(d->hdc, d->hpen);
	    SetTextColor(d->hdc, d->pColor);
	    if (hpen_old)
		DeleteObject(hpen_old);
	    return;
	}
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
	 (qt_winver & WV_NT_based || d->pStyle == SolidLine)) {
	LOGBRUSH lb;
	lb.lbStyle = 0;
	lb.lbColor = d->pColor;
	lb.lbHatch = 0;
	int pst = PS_GEOMETRIC | s;
// 	qFatal( "not supported... yet.." );
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
    SetTextColor(d->hdc, d->pColor);			// pen color is also text color
    SelectObject(d->hdc, d->hpen);
    if (hpen_old)				// delete last pen
	DeleteObject(hpen_old);

}


void QWin32GC::updateBrush(QPainterState *state)
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

    SelectObject(d->hdc, d->hbrush);

    if (hbrush_old) {
	DeleteObject(hbrush_old);		// delete last brush
	if (hbrushbm_old && !pixmapBrush_old)
	    DeleteObject(hbrushbm_old);	// delete last brush pixmap
    }

    SetBkColor(d->hdc, COLOR_VALUE(state->bgColor));
}


void QWin32GC::updateRasterOp(QPainterState *state)
{
    Q_ASSERT(isActive());
    SetROP2(d->hdc, rasterOpCodes[state->rasterOp]);
    d->rasterOp = state->rasterOp;
}


void QWin32GC::updateBackground(QPainterState *state)
{
    Q_ASSERT(isActive());

    SetBkColor(d->hdc, COLOR_VALUE(state->bgBrush.color()));
    SetBkMode(d->hdc, state->bgMode == TransparentMode ? TRANSPARENT : OPAQUE);
}


void QWin32GC::updateXForm(QPainterState *state)
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
	    printf("QWin32GC::updateXForm(), SetWorldTransformation() failed.. %d\n", GetLastError());
	}
//     } else {
// 	m.eM11 = m.eM22 = (float)1.0;
// 	m.eM12 = m.eM21 = m.eDx = m.eDy = (float)0.0;
// 	SetGraphicsMode(d->hdc, GM_ADVANCED);
// 	ModifyWorldTransform(d->hdc, &m, MWT_IDENTITY);
// 	SetGraphicsMode(d->hdc, GM_COMPATIBLE);
//     }
}


void QWin32GC::updateClipRegion(QPainterState *state)
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

// 	if (state->VxF || state->WxF) {
// 	    qDebug(" --> Translate region...");
// 	    rgn = state->worldMatrix * rgn;
// 	}

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

void QWin32GC::updateFont(QPainterState *state)
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


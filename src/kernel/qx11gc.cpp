#include "qplatformdefs.h"

#include "qx11gc.h"

#include "qfont.h"
#include "qwidget.h"
#include "qbitmap.h"
#include "qpixmapcache.h"
#include "qtextcodec.h"
#include "qpaintdevicemetrics.h"
#include "qpaintdevice.h" // tmp
#include "qpainter.h" // tmp

#include "qt_x11_p.h"

#include "q4painter_p.h"
#include "qtextlayout_p.h"
#include "qfontdata_p.h"
#include "qfontengine_p.h"
#include "qtextengine_p.h"

#include "qpen.h"
#include "qcolor.h"
#include "qfont.h"

#include <math.h>

//
// Some global variables - these are initialized by QColor::initialize()
//

Display *QX11GC::x_appdisplay = 0;
int QX11GC::x_appscreen;

int *QX11GC::x_appdepth_arr;
int *QX11GC::x_appcells_arr;
Qt::HANDLE *QX11GC::x_approotwindow_arr;
Qt::HANDLE *QX11GC::x_appcolormap_arr;
bool *QX11GC::x_appdefcolormap_arr;
void **QX11GC::x_appvisual_arr;
bool *QX11GC::x_appdefvisual_arr;



// paintevent magic to provide Windows semantics on X11
static QRegion* paintEventClipRegion = 0;
static QPaintDevice* paintEventDevice = 0;

void qt_set_paintevent_clipping( QPaintDevice* dev, const QRegion& region)
{
    if ( !paintEventClipRegion )
        paintEventClipRegion = new QRegion( region );
    else
        *paintEventClipRegion = region;
    paintEventDevice = dev;
}

void qt_intersect_paintevent_clipping(QPaintDevice *dev, QRegion &region)
{
    if ( dev == paintEventDevice && paintEventClipRegion )
	region &= *paintEventClipRegion;
}

void qt_clear_paintevent_clipping()
{
    delete paintEventClipRegion;
    paintEventClipRegion = 0;
    paintEventDevice = 0;
}

#ifdef QT_NO_XRENDER
typedef unsigned long Picture;
static const Picture rendhd = 0;
#endif

// hack, so we don't have to make QRegion::clipRectangles() public or include
// X11 headers in qregion.h
inline void *qt_getClipRects( const QRegion &r, int &num )
{
    return r.clipRectangles( num );
}

static inline void x11SetClipRegion( Display *dpy, GC gc, Picture rh, const QRegion &r )
{
    int num;
    XRectangle *rects = (XRectangle *)qt_getClipRects( r, num );
#ifndef QT_NO_XRENDER
    if (rh)
	XRenderSetPictureClipRectangles(dpy, rh, 0, 0, rects, num );
#else
    Q_UNUSED( 1rh );
#endif // QT_NO_XRENDER
    XSetClipRectangles( dpy, gc, 0, 0, rects, num, YXBanded );
}

static inline void x11SetClipRegion( Display *dpy, GC gc, Picture rh, const QRegion &r, GC gc2 )
{
    int num;
    XRectangle *rects = (XRectangle *)qt_getClipRects( r, num );
#ifndef QT_NO_XRENDER
    if (rh)
	XRenderSetPictureClipRectangles(dpy, rh, 0, 0, rects, num );
#else
    Q_UNUSED( rh );
#endif // QT_NO_XRENDER
    XSetClipRectangles( dpy, gc, 0, 0, rects, num, YXBanded );
    XSetClipRectangles( dpy, gc2, 0, 0, rects, num, YXBanded );
}


/*****************************************************************************
  Trigonometric function for QPainter

  We have implemented simple sine and cosine function that are called from
  QPainter::drawPie() and QPainter::drawChord() when drawing the outline of
  pies and chords.
  These functions are slower and less accurate than math.h sin() and cos(),
  but with still around 1/70000th sec. execution time (on a 486DX2-66) and
  8 digits accuracy, it should not be the bottleneck in drawing these shapes.
  The advantage is that you don't have to link in the math library.
 *****************************************************************************/

const double Q_PI   = 3.14159265358979323846;   // pi
const double Q_2PI  = 6.28318530717958647693;   // 2*pi
const double Q_PI2  = 1.57079632679489661923;   // pi/2


#if defined(Q_CC_GNU) && defined(Q_OS_AIX)
// AIX 4.2 gcc 2.7.2.3 gets internal error.
static int qRoundAIX( double d )
{
    return qRound(d);
}
#define qRound qRoundAIX
#endif


#if defined(Q_CC_GNU) && defined(__i386__)

inline double qcos( double a )
{
    double r;
    __asm__ (
        "fcos"
        : "=t" (r) : "0" (a) );
    return(r);
}

inline double qsin( double a )
{
    double r;
    __asm__ (
        "fsin"
        : "=t" (r) : "0" (a) );
    return(r);
}

double qsincos( double a, bool calcCos=false )
{
    return calcCos ? qcos(a) : qsin(a);
}

#else

double qsincos( double a, bool calcCos=false )
{
    if ( calcCos )                              // calculate cosine
        a -= Q_PI2;
    if ( a >= Q_2PI || a <= -Q_2PI ) {          // fix range: -2*pi < a < 2*pi
        int m = (int)(a/Q_2PI);
        a -= Q_2PI*m;
    }
    if ( a < 0.0 )                              // 0 <= a < 2*pi
        a += Q_2PI;
    int sign = a > Q_PI ? -1 : 1;
    if ( a >= Q_PI )
        a = Q_2PI - a;
    if ( a >= Q_PI2 )
        a = Q_PI - a;
    if ( calcCos )
        sign = -sign;
    double a2  = a*a;                           // here: 0 <= a < pi/4
    double a3  = a2*a;                          // make taylor sin sum
    double a5  = a3*a2;
    double a7  = a5*a2;
    double a9  = a7*a2;
    double a11 = a9*a2;
    return (a-a3/6+a5/120-a7/5040+a9/362880-a11/39916800)*sign;
}

inline double qsin( double a ) { return qsincos(a, false); }
inline double qcos( double a ) { return qsincos(a, true); }

#endif

/*
 * GC cache stuff
 *
 */

/*****************************************************************************
  QPainter internal GC (Graphics Context) allocator.

  The GC allocator offers two functions; alloc_gc() and free_gc() that
  reuse GC objects instead of calling XCreateGC() and XFreeGC(), which
  are a whole lot slower.
 *****************************************************************************/

struct QGC
{
    GC   gc;
    char in_use;
    bool mono;
    int scrn;
};

const  int  gc_array_size = 256;
static QGC  gc_array[gc_array_size];            // array of GCs
static bool gc_array_init = false;


static void init_gc_array()
{
    if ( !gc_array_init ) {
        memset( gc_array, 0, gc_array_size*sizeof(QGC) );
        gc_array_init = true;
    }
}

static void cleanup_gc_array( Display *dpy )
{
    register QGC *p = gc_array;
    int i = gc_array_size;
    if ( gc_array_init ) {
        while ( i-- ) {
            if ( p->gc )                        // destroy GC
                XFreeGC( dpy, p->gc );
            p++;
        }
        gc_array_init = false;
    }
}

// #define DONT_USE_GC_ARRAY

static GC alloc_gc( Display *dpy, int scrn, Drawable hd, bool monochrome=false,
                    bool privateGC = false )
{
#if defined(DONT_USE_GC_ARRAY)
    privateGC = true;                           // will be slower
#endif
    if ( privateGC ) {
        GC gc = XCreateGC( dpy, hd, 0, 0 );
        XSetGraphicsExposures( dpy, gc, False );
        return gc;
    }
    register QGC *p = gc_array;
    int i = gc_array_size;
    if ( !gc_array_init )                       // not initialized
        init_gc_array();
    while ( i-- ) {
        if ( !p->gc ) {                         // create GC (once)
            p->gc = XCreateGC( dpy, hd, 0, 0 );
            p->scrn = scrn;
            XSetGraphicsExposures( dpy, p->gc, False );
            p->in_use = false;
            p->mono   = monochrome;
        }
        if ( !p->in_use && p->mono == monochrome && p->scrn == scrn ) {
            p->in_use = true;                   // available/compatible GC
            return p->gc;
        }
        p++;
    }
    qWarning( "QPainter: Internal error; no available GC" );
    GC gc = XCreateGC( dpy, hd, 0, 0 );
    XSetGraphicsExposures( dpy, gc, False );
    return gc;
}

static void free_gc( Display *dpy, GC gc, bool privateGC = false )
{
#if defined(DONT_USE_GC_ARRAY)
    privateGC = true;                           // will be slower
#endif
    if ( privateGC ) {
        Q_ASSERT( dpy != 0 );
        XFreeGC( dpy, gc );
        return;
    }
    register QGC *p = gc_array;
    int i = gc_array_size;
    if ( gc_array_init ) {
        while ( i-- ) {
            if ( p->gc == gc ) {
                p->in_use = false;              // set available
                XSetClipMask( dpy, gc, None );  // make it reusable
                XSetFunction( dpy, gc, GXcopy );
                XSetFillStyle( dpy, gc, FillSolid );
                XSetTSOrigin( dpy, gc, 0, 0 );
                return;
            }
            p++;
        }
    }
}



/*****************************************************************************
  QPainter internal GC (Graphics Context) cache for solid pens and
  brushes.

  The GC cache makes a significant contribution to speeding up
  drawing.  Setting new pen and brush colors will make the painter
  look for another GC with the same color instead of changing the
  color value of the GC currently in use. The cache structure is
  optimized for fast lookup.  Only solid line pens with line width 0
  and solid brushes are cached.

  In addition, stored GCs may have an implicit clipping region
  set. This prevents any drawing outside paint events. Both
  updatePen() and updateBrush() keep track of the validity of this
  clipping region by storing the clip_serial number in the cache.

*****************************************************************************/

struct QGCC                                     // cached GC
{
    GC gc;
    uint pix;
    int count;
    int hits;
    uint clip_serial;
    int scrn;
};

const  int   gc_cache_size = 29;                // multiply by 4
static QGCC *gc_cache_buf;
static QGCC *gc_cache[4*gc_cache_size];
static bool  gc_cache_init;
static uint gc_cache_clip_serial;


static void init_gc_cache()
{
    if ( !gc_cache_init ) {
        gc_cache_init = true;
	gc_cache_clip_serial = 0;
        QGCC *g = gc_cache_buf = new QGCC[4*gc_cache_size];
        memset( g, 0, 4*gc_cache_size*sizeof(QGCC) );
        for ( int i=0; i<4*gc_cache_size; i++ )
            gc_cache[i] = g++;
    }
}


// #define GC_CACHE_STAT
#if defined(GC_CACHE_STAT)
#include "qtextstream.h"
#include "qbuffer.h"

static int g_numhits    = 0;
static int g_numcreates = 0;
static int g_numfaults  = 0;
#endif


static void cleanup_gc_cache()
{
    if ( !gc_cache_init )
        return;
#if defined(GC_CACHE_STAT)
    qDebug( "Number of cache hits = %d", g_numhits );
    qDebug( "Number of cache creates = %d", g_numcreates );
    qDebug( "Number of cache faults = %d", g_numfaults );
    for ( int i=0; i<gc_cache_size; i++ ) {
        QCString    str;
        QBuffer     buf( str );
        buf.open(IO_ReadWrite);
        QTextStream s(&buf);
        s << i << ": ";
        for ( int j=0; j<4; j++ ) {
            QGCC *g = gc_cache[i*4+j];
            s << (g->gc ? 'X' : '-') << ',' << g->hits << ','
              << g->count << '\t';
        }
        s << '\0';
        qDebug( str );
        buf.close();
    }
#endif
    delete [] gc_cache_buf;
    gc_cache_init = false;
}


static bool obtain_gc( void **ref, GC *gc, uint pix, Display *dpy, int scrn,
		       Qt::HANDLE hd, uint painter_clip_serial )
{
    if ( !gc_cache_init )
        init_gc_cache();

    int   k = (pix % gc_cache_size) * 4;
    QGCC *g = gc_cache[k];
    QGCC *prev = 0;

#define NOMATCH (g->gc && (g->pix != pix || g->scrn != scrn || \
                 (g->clip_serial > 0 && g->clip_serial != painter_clip_serial)))

    if ( NOMATCH ) {
        prev = g;
        g = gc_cache[++k];
        if ( NOMATCH ) {
            prev = g;
            g = gc_cache[++k];
            if ( NOMATCH ) {
                prev = g;
                g = gc_cache[++k];
                if ( NOMATCH ) {
                    if ( g->count == 0 && g->scrn == scrn) {    // steal this GC
                        g->pix   = pix;
                        g->count = 1;
                        g->hits  = 1;
			g->clip_serial = 0;
                        XSetForeground( dpy, g->gc, pix );
			XSetClipMask(dpy, g->gc, None);
                        gc_cache[k]   = prev;
                        gc_cache[k-1] = g;
                        *ref = (void *)g;
                        *gc = g->gc;
                        return true;
                    } else {                    // all GCs in use
#if defined(GC_CACHE_STAT)
                        g_numfaults++;
#endif
                        *ref = 0;
                        return false;
                    }
                }
            }
        }
    }

#undef NOMATCH

    *ref = (void *)g;

    if ( g->gc ) {                              // reuse existing GC
#if defined(GC_CACHE_STAT)
        g_numhits++;
#endif
        *gc = g->gc;
        g->count++;
        g->hits++;
        if ( prev && g->hits > prev->hits ) {   // maintain LRU order
            gc_cache[k]   = prev;
            gc_cache[k-1] = g;
        }
        return true;
    } else {                                    // create new GC
#if defined(GC_CACHE_STAT)
        g_numcreates++;
#endif
        g->gc = alloc_gc( dpy, scrn, hd, false );
        g->scrn = scrn;
        g->pix = pix;
        g->count = 1;
        g->hits = 1;
        g->clip_serial = 0;
        *gc = g->gc;
        return false;
    }
}

static inline void release_gc( void *ref )
{
    ((QGCC*)ref)->count--;
}



// ########
void qt_erase_background(Qt::HANDLE hd, int screen,
			 int x, int y, int w, int h,
			 const QBrush &brush, int xoff, int yoff)
{
    Display *dpy = QPaintDevice::x11AppDisplay();
    GC gc;
    void *penref = 0;
    ulong pixel = brush.color().pixel(screen);
    bool obtained = obtain_gc(&penref, &gc, pixel, dpy, screen, hd, gc_cache_clip_serial);

    if (!obtained && !penref) {
	gc = alloc_gc(dpy, screen, hd, false);
    } else {
	if (penref && ((QGCC*)penref)->clip_serial) {
	    XSetClipMask(dpy, gc, None);
	    ((QGCC*)penref)->clip_serial = 0;
	}
    }

    if (!obtained) {
	XSetForeground(dpy, gc, pixel);
    }

    if (brush.pixmap()) {
	XSetTile(dpy, gc, brush.pixmap()->handle());
	XSetFillStyle(dpy, gc, FillTiled);
	XSetTSOrigin(dpy, gc, x-xoff, y-yoff);
	XFillRectangle(dpy, hd, gc, x, y, w, h);
	XSetTSOrigin(dpy, gc, 0, 0);
	XSetFillStyle(dpy, gc, FillSolid);
    } else {
	XFillRectangle(dpy, hd, gc, x, y, w, h);
    }

    if (penref) {
	release_gc(penref);
    } else {
	free_gc(dpy, gc);
    }
}

void qt_draw_transformed_rect( QPainter *p,  int x, int y, int w,  int h, bool fill )
{
//     XPoint points[5];
//     int xp = x,  yp = y;
//     p->map( xp, yp, &xp, &yp );
//     points[0].x = xp;
//     points[0].y = yp;
//     xp = x + w; yp = y;
//     p->map( xp, yp, &xp, &yp );
//     points[1].x = xp;
//     points[1].y = yp;
//     xp = x + w; yp = y + h;
//     p->map( xp, yp, &xp, &yp );
//     points[2].x = xp;
//     points[2].y = yp;
//     xp = x; yp = y + h;
//     p->map( xp, yp, &xp, &yp );
//     points[3].x = xp;
//     points[3].y = yp;
//     points[4] = points[0];

//     if ( fill )
// 	XFillPolygon( p->dpy, p->hd, p->gc, points, 4, Convex, CoordModeOrigin );
//     else
// 	XDrawLines( p->dpy, p->hd, p->gc, points, 5, CoordModeOrigin );
}


// void qt_draw_background( QPainter *p, int x, int y, int w,  int h )
// {
//     if (p->testf(QPainter::ExtDev)) {
// 	if (p->pdev->devType() == QInternal::Printer)
// 	    p->fillRect(x, y, w, h, p->bg_brush);
// 	return;
//     }
//     XSetForeground( p->dpy, p->gc, p->bg_brush.color().pixel(p->scrn) );
//     qt_draw_transformed_rect( p, x, y, w, h, TRUE);
//     XSetForeground( p->dpy, p->gc, p->cpen.color().pixel(p->scrn) );
// }
// ########

/*
 * QX11GCPrivate
 */

struct QX11GCPrivate {
    QX11GCPrivate()
	{
	    dpy = 0;
	    scrn = -1;
	    hd = 0;
	    rendhd = 0;
//  	    flags = Qt::IsStartingUp;
	    bg_col = Qt::white;                             // default background color
 	    bg_mode = Qt::TransparentMode;                  // default background mode
	    rop = Qt::CopyROP;                                // default ROP
	    tabstops = 0;                               // default tabbing
	    tabarray = 0;
	    tabarraylen = 0;
	    ps_stack = 0;
	    wm_stack = 0;
	    gc = gc_brush = 0;
	    pdev = 0;
	    dpy  = 0;
// 	    txop = txinv = 0;
	    penRef = brushRef = 0;
	    clip_serial = 0;
// 	    pfont = 0;
// 	    block_ext = false;
	}
    Display *dpy;
    int scrn;
    Qt::HANDLE hd;
    Qt::HANDLE rendhd;
    GC gc;
    GC gc_brush;

    QColor bg_col;
    uchar bg_mode;
    Qt::RasterOp rop;
//     uchar pu;
//     QPoint bro;
//     QFont cfont;
//     QFont *pfont; 	// font used for metrics (might be different for printers)
    QPen cpen;
    QBrush cbrush;
    QBrush bg_brush;
    QRegion crgn;
    int tabstops;
    int *tabarray;
    int tabarraylen;

    void *penRef;
    void *brushRef;
    void *ps_stack;
    void *wm_stack;
    uint clip_serial;
    QPaintDevice *pdev; // tmp - QPaintDevice
};


/*
 * QX11GC members
 */

QX11GC::QX11GC(const QPaintDevice *target)
{
    d = new QX11GCPrivate;

    d->dpy = QPaintDevice::x11AppDisplay();
    d->scrn = QPaintDevice::x11AppScreen();
    d->hd = target->handle();
    d->pdev = (QPaintDevice*)target;
    x11Data = 0; // prob. move to the d obj.
}

QX11GC::~QX11GC()
{
    delete d;
}

void QX11GC::initialize()
{
    // tmp stuff - these will be inited from QApplication etc. when
    // putting it into main
    static bool doInit = true;
    if (doInit) {
	QX11GC::x_appdisplay = QPaintDevice::x11AppDisplay();
	QX11GC::x_appscreen = QPaintDevice::x11AppScreen();
	init_gc_array();
	init_gc_cache();
	doInit = false;
    }
}

void QX11GC::cleanup()
{
    cleanup_gc_cache();
    cleanup_gc_array(QX11GC::x_appdisplay);
}

bool QX11GC::begin(const QPaintDevice *pdev, QPainterState *ps, bool unclipped)
{
//     d->hd = d->pdev->handle(); // because of double buffering the hd might change
//     updatePen(ps);
//     updateBrush(ps);
//     return true;

    if ( isActive() ) {                         // already active painting
        qWarning( "QX11GC::begin: Painter is already active."
                  "\n\tYou must end() the painter before a second begin()" );
        return false;
    }
    
    setActive(true);

    QPixmap::x11SetDefaultScreen( d->pdev->x11Screen() );

//     const QWidget *copyMe = 0;
//     if ((d->pdev = const_cast<QPaintDevice*>(redirected(d->pdev, &redirection_offset)))) {
// 	if ( d->pdev->devType() == QInternal::Widget )
// 	    copyMe = static_cast<const QWidget *>(pd); // copy widget settings
//     } else {
// 	pdev = const_cast<QPaintDevice*>(pd);
//     }


//     bool reinit = flags != IsStartingUp;        // 2nd or 3rd etc. time called
//     flags = IsActive | DirtyFont;               // init flags
//     int dt = d->pdev->devType();                   // get the device type

//     if ( (pdev->devFlags & QInternal::ExternalDevice) != 0 )
//         setf(ExtDev);
//     else if ( dt == QInternal::Pixmap )         // device is a pixmap
//         ((QPixmap*)pdev)->detach();             // will modify it

    d->dpy = d->pdev->x11Display();                   // get display variable
    d->scrn = d->pdev->x11Screen();			// get screen variable
    d->hd = d->pdev->handle();                       // get handle to drawable - NB! double buffering might change the handle
    d->rendhd = d->pdev->x11RenderHandle();

    if (d->pdev->x11Depth() != d->pdev->x11AppDepth( d->scrn)) { // non-standard depth
        setf(NoCache);
        setf(UsePrivateCx);
    }

//     pdev->painters++;                           // also tell paint device
//     bro = curPt = QPoint( 0, 0 );
//     if ( reinit ) {
//         d->bg_mode = TransparentMode;              // default background mode
//         d->rop = CopyROP;                          // default ROP
//     }

    QWidget *w = (QWidget *) d->pdev;
//     if ( reinit ) {
// 	QBrush defaultBrush;
// 	ps->brush = defaultBrush;
//     }
    if (unclipped || w->testWFlags(WPaintUnclipped)) {  // paint direct on device
	setf(NoCache);
	setf(UsePrivateCx);
	updatePen(ps);
	updateBrush(ps);
	XSetSubwindowMode(d->dpy, d->gc, IncludeInferiors);
	XSetSubwindowMode(d->dpy, d->gc_brush, IncludeInferiors);
#ifndef QT_NO_XRENDER
	if (d->rendhd) {
	    XRenderPictureAttributes pattr;
	    pattr.subwindow_mode = IncludeInferiors;
	    XRenderChangePicture(d->dpy, d->rendhd, CPSubwindowMode, &pattr);
	}
#endif
    }
//     else if ( dt == QInternal::Pixmap ) {             // device is a pixmap
//         QPixmap *pm = (QPixmap*)pdev;
//         if ( pm->isNull() ) {
//             qWarning( "QPainter::begin: Cannot paint null pixmap" );
//             end();
//             return FALSE;
//         }
//         bool mono = pm->depth() == 1;           // monochrome bitmap
//         if ( mono ) {
//             setf( MonoDev );
//             bg_brush = color0;
//             cpen.setColor( color1 );
//         }
//         ww = vw = pm->width();                  // default view size
//         wh = vh = pm->height();
//     }

    d->clip_serial = gc_cache_clip_serial++;
    updateBrush(ps);
    updatePen(ps);
    updateClipRegion(ps);

//     QRect rg = ps->clipRegion.boundingRect();
//     if (!redirection_offset.isNull()) {
// 	txop = TxTranslate;
// 	setf(WxF, true);
//     }

    return true;
}

bool QX11GC::end()
{
    setActive(false);
    return true;
}

void QX11GC::drawLine(int x1, int y1, int x2, int y2)
{
    if (!isActive())
        return;
    if (d->cpen.style() != NoPen)
        XDrawLine(d->dpy, d->hd, d->gc, x1, y1, x2, y2);
}

void QX11GC::drawRect(int x, int y, int w, int h)
{
    if (!isActive())
        return;
    if (w <= 0 || h <= 0) {
        if (w == 0 || h == 0)
            return;
    }
    if (d->cbrush.style() != NoBrush) {
        if (d->cpen.style() == NoPen) {
            XFillRectangle(d->dpy, d->hd, d->gc_brush, x, y, w, h);
            return;
        }
	int lw = d->cpen.width();
	int lw2 = (lw+1)/2;
        if (w > lw && h > lw)
            XFillRectangle(d->dpy, d->hd, d->gc_brush, x+lw2, y+lw2, w-lw-1, h-lw-1);
    }
    if (d->cpen.style() != NoPen)
        XDrawRectangle(d->dpy, d->hd, d->gc, x, y, w-1, h-1);
}

void QX11GC::drawPoint(int x, int y)
{
    if (!isActive())
        return;
    if (d->cpen.style() != NoPen)
        XDrawPoint(d->dpy, d->hd, d->gc, x, y);
}

void QX11GC::drawPoints(const QPointArray &a, int index , int npoints)
{
    if (npoints < 0)
        npoints = a.size() - index;
    if (index + npoints > (int)a.size())
        npoints = a.size() - index;
    if (!isActive() || npoints < 1 || index < 0)
        return;
    QPointArray pa = a;
//    if ( testf(ExtDev|VxF|WxF) ) {
//         if ( txop != TxNone ) {
//             pa = xForm( a, index, npoints );
//             if ( pa.size() != a.size() ) {
//                 index = 0;
//                 npoints = pa.size();
//             }
//         }
//     }
    if (d->cpen.style() != NoPen)
        XDrawPoints(d->dpy, d->hd, d->gc, (XPoint*)(pa.shortPoints(index, npoints)),
		    npoints, CoordModeOrigin);
}

void QX11GC::drawWinFocusRect(int x, int y, int w, int h, bool xorPaint, const QColor &bgColor)
{
    if (!isActive()) //|| txop == TxRotShear)
        return;
    static char winfocus_line[] = {1, 1};

//     QPen old_pen = d->cpen;
//     RasterOp old_rop = (RasterOp) d->rop;

//     if (xorPaint) {
//         if (QColor::numBitPlanes() <= 8)
//             setPen(color1);
//         else
//             setPen(white);
//         setRasterOp(XorROP);
//     } else {
//         if (qGray(bgColor.rgb()) < 128)
//             setPen(white);
//         else
//             setPen(black);
//     }

//     if ( testf(ExtDev|VxF|WxF) ) {
//         map( x, y, w, h, &x, &y, &w, &h );
//     }
    if (w <= 0 || h <= 0) {
        if (w == 0 || h == 0)
            return;
        fix_neg_rect(&x, &y, &w, &h);
    }
    XSetDashes(d->dpy, d->gc, 0, winfocus_line, 2);
    XSetLineAttributes(d->dpy, d->gc, 1, LineOnOffDash, CapButt, JoinMiter);

    XDrawRectangle(d->dpy, d->hd, d->gc, x, y, w-1, h-1);
    XSetLineAttributes(d->dpy, d->gc, 0, LineSolid, CapButt, JoinMiter);
//     setRasterOp(old_rop);
//     setPen(old_pen);
}

void QX11GC::updatePen(QPainterState *state)
{
    d->cpen = state->pen;
    d->cbrush = state->brush;
//     d->bg_brush = state->bgBrush;

    int ps = d->cpen.style();
    bool cacheIt = !testf(ClipOn|MonoDev|NoCache) &&
                   (ps == NoPen || ps == SolidLine) &&
                   d->cpen.width() == 0 && d->rop == CopyROP;

    bool obtained = false;
    bool internclipok = hasClipping();
    if (cacheIt) {
        if (d->gc) {
            if (d->penRef)
                release_gc(d->penRef);
            else
                free_gc(d->dpy, d->gc);
        }
        obtained = obtain_gc(&d->penRef, &d->gc, d->cpen.color().pixel(d->scrn), d->dpy, d->scrn,
			     d->hd, d->clip_serial);
        if (!obtained && !d->penRef)
            d->gc = alloc_gc(d->dpy, d->scrn, d->hd, false);
    } else {
        if (d->gc) {
            if (d->penRef) {
                release_gc(d->penRef);
                d->penRef = 0;
                d->gc = alloc_gc(d->dpy, d->scrn, d->hd, testf(MonoDev));
            } else {
                internclipok = true;
            }
        } else {
            d->gc = alloc_gc(d->dpy, d->scrn, d->hd, testf(MonoDev), testf(UsePrivateCx));
        }
    }

    if (!internclipok) {
        if (d->pdev == paintEventDevice && paintEventClipRegion) {
            if (d->penRef &&((QGCC*)d->penRef)->clip_serial < gc_cache_clip_serial) {
		x11SetClipRegion(d->dpy, d->gc, d->rendhd, *paintEventClipRegion);
                ((QGCC*)d->penRef)->clip_serial = gc_cache_clip_serial;
            } else if (!d->penRef) {
		x11SetClipRegion(d->dpy, d->gc, d->rendhd, *paintEventClipRegion);
            }
        } else if (d->penRef && ((QGCC*)d->penRef)->clip_serial) {
#ifndef QT_NO_XRENDER
            if (d->rendhd) {
                XRenderPictureAttributes pattr;
                pattr.clip_mask = None;
                XRenderChangePicture(d->dpy, d->rendhd, CPClipMask, &pattr);
            }
#endif // QT_NO_XRENDER
            XSetClipMask(d->dpy, d->gc, None);
            ((QGCC*)d->penRef)->clip_serial = 0;
        }
    }

    if (obtained)
        return;

    char dashes[10];                            // custom pen dashes
    int dash_len = 0;                           // length of dash list
    int s = LineSolid;
    int cp = CapButt;
    int jn = JoinMiter;

    /*
      We are emulating Windows here.  Windows treats cpen.width() == 1
      (or 0) as a very special case.  The fudge variable unifies this
      case with the general case.
    */
    int dot = d->cpen.width();                     // width of a dot
    int fudge = 1;
    bool allow_zero_lw = true;
    if (dot <= 1) {
        dot = 3;
        fudge = 2;
    }

    switch(ps) {
	case NoPen:
	case SolidLine:
	    s = LineSolid;
	    break;
	case DashLine:
	    dashes[0] = fudge * 3 * dot;
	    dashes[1] = fudge * dot;
	    dash_len = 2;
	    allow_zero_lw = false;
	    break;
	case DotLine:
	    dashes[0] = dot;
	    dashes[1] = dot;
	    dash_len = 2;
	    allow_zero_lw = false;
	    break;
	case DashDotLine:
	    dashes[0] = 3 * dot;
	    dashes[1] = fudge * dot;
	    dashes[2] = dot;
	    dashes[3] = fudge * dot;
	    dash_len = 4;
	    allow_zero_lw = false;
	    break;
	case DashDotDotLine:
	    dashes[0] = 3 * dot;
	    dashes[1] = dot;
	    dashes[2] = dot;
	    dashes[3] = dot;
	    dashes[4] = dot;
	    dashes[5] = dot;
	    dash_len = 6;
	    allow_zero_lw = false;
    }
    Q_ASSERT(dash_len <= (int) sizeof(dashes));

    switch (d->cpen.capStyle()) {
	case SquareCap:
	    cp = CapProjecting;
	    break;
	case RoundCap:
	    cp = CapRound;
	    break;
	case FlatCap:
	default:
	    cp = CapButt;
	    break;
    }
    switch (d->cpen.joinStyle()) {
	case BevelJoin:
	    jn = JoinBevel;
	    break;
	case RoundJoin:
	    jn = JoinRound;
	    break;
	case MiterJoin:
	default:
	    jn = JoinMiter;
	    break;
    }

    XSetForeground(d->dpy, d->gc, d->cpen.color().pixel(d->scrn));
    XSetBackground(d->dpy, d->gc, d->bg_col.pixel(d->scrn));

    if (dash_len) {                           // make dash list
        XSetDashes(d->dpy, d->gc, 0, dashes, dash_len);
        s = d->bg_mode == TransparentMode ? LineOnOffDash : LineDoubleDash;
    }
    XSetLineAttributes(d->dpy, d->gc,
		       (! allow_zero_lw && d->cpen.width() == 0) ? 1 : d->cpen.width(),
		       s, cp, jn);
}

void QX11GC::updateBrush(QPainterState *state)
{
    d->cbrush = state->brush;

    static const uchar dense1_pat[] = { 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff };
    static const uchar dense2_pat[] = { 0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff };
    static const uchar dense3_pat[] = { 0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee };
    static const uchar dense4_pat[] = { 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa };
    static const uchar dense5_pat[] = { 0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11 };
    static const uchar dense6_pat[] = { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 };
    static const uchar dense7_pat[] = { 0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00 };
    static const uchar hor_pat[] = {                      // horizontal pattern
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    static const uchar ver_pat[] = {                      // vertical pattern
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20 };
    static const uchar cross_pat[] = {                    // cross pattern
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0xff, 0xff, 0xff,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0xff, 0xff, 0xff, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0xff, 0xff, 0xff,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0xff, 0xff, 0xff, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20 };
    static const uchar bdiag_pat[] = {                    // backward diagonal pattern
	0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04, 0x02, 0x02, 0x01, 0x01,
	0x80, 0x80, 0x40, 0x40, 0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04,
	0x02, 0x02, 0x01, 0x01, 0x80, 0x80, 0x40, 0x40 };
    static const uchar fdiag_pat[] = {                    // forward diagonal pattern
	0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40,
	0x80, 0x80, 0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10,
	0x20, 0x20, 0x40, 0x40, 0x80, 0x80, 0x01, 0x01 };
    static const uchar dcross_pat[] = {                   // diagonal cross pattern
	0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14, 0x22, 0x22, 0x41, 0x41,
	0x80, 0x80, 0x41, 0x41, 0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14,
	0x22, 0x22, 0x41, 0x41, 0x80, 0x80, 0x41, 0x41 };
    static const uchar * const pat_tbl[] = {
	dense1_pat, dense2_pat, dense3_pat, dense4_pat, dense5_pat,
	dense6_pat, dense7_pat,
	hor_pat, ver_pat, cross_pat, bdiag_pat, fdiag_pat, dcross_pat };

    int  bs = d->cbrush.style();
    int x, y;
//     map( d->bro.x(), d->bro.y(), &x, &y );
    bool cacheIt = !testf(ClipOn|MonoDev|NoCache) &&
                   (bs == NoBrush || bs == SolidPattern) &&
	           x == 0 && y == 0 && d->rop == CopyROP;

    bool obtained = false;
    bool internclipok = hasClipping();
    if (cacheIt) {
        if (d->gc_brush) {
            if (d->brushRef)
                release_gc(d->brushRef);
            else
                free_gc(d->dpy, d->gc_brush);
        }
        obtained = obtain_gc(&d->brushRef, &d->gc_brush, d->cbrush.color().pixel(d->scrn), d->dpy,
			     d->scrn, d->hd, d->clip_serial);
        if (!obtained && !d->brushRef)
            d->gc_brush = alloc_gc(d->dpy, d->scrn, d->hd, false);
    } else {
        if (d->gc_brush) {
            if (d->brushRef) {
                release_gc(d->brushRef);
                d->brushRef = 0;
                d->gc_brush = alloc_gc(d->dpy, d->scrn, d->hd, testf(MonoDev));
            } else {
                internclipok = true;
            }
        } else {
            d->gc_brush = alloc_gc(d->dpy, d->scrn, d->hd, testf(MonoDev), testf(UsePrivateCx));
        }
    }

    if (!internclipok) {
        if (d->pdev == paintEventDevice && paintEventClipRegion) {
            if (d->brushRef &&((QGCC*)d->brushRef)->clip_serial < gc_cache_clip_serial) {
		x11SetClipRegion(d->dpy, d->gc_brush, d->rendhd, *paintEventClipRegion);
                ((QGCC*)d->brushRef)->clip_serial = gc_cache_clip_serial;
            } else if (!d->brushRef){
		x11SetClipRegion(d->dpy, d->gc_brush, d->rendhd, *paintEventClipRegion);
            }
        } else if (d->brushRef && ((QGCC*)d->brushRef)->clip_serial) {
#ifndef QT_NO_XRENDER
            if (d->rendhd) {
                XRenderPictureAttributes pattr;
                pattr.clip_mask = None;
                XRenderChangePicture(d->dpy, d->rendhd, CPClipMask, &pattr);
            }
#endif // QT_NO_XRENDER
            XSetClipMask(d->dpy, d->gc_brush, None);
            ((QGCC*)d->brushRef)->clip_serial = 0;
        }
    }

    if (obtained)
        return;

    const uchar *pat = 0;                             // pattern
    int dd = 0;                                  // defalt pattern size: d*d
    int s  = FillSolid;
    if (bs >= Dense1Pattern && bs <= DiagCrossPattern) {
        pat = pat_tbl[bs - Dense1Pattern];
        if (bs <= Dense7Pattern)
            dd = 8;
        else if (bs <= CrossPattern)
            dd = 24;
        else
            dd = 16;
    }

    XSetLineAttributes(d->dpy, d->gc_brush, 0, LineSolid, CapButt, JoinMiter);
    XSetForeground(d->dpy, d->gc_brush, d->cbrush.color().pixel(d->scrn));
    XSetBackground(d->dpy, d->gc_brush, d->bg_col.pixel(d->scrn));

    if (bs == CustomPattern || pat) {
        QPixmap pm;
        if (pat) {
            QString key;
            key.sprintf("$qt-brush$%d", bs);
            if (!QPixmapCache::find(key, pm)) {                        // not already in pm dict
                pm = QBitmap(dd, dd, pat, true);
                QPixmapCache::insert(key, pm);
            }
// private stuff in QBrush - need access to this
//             if ( d->cbrush.d->pixmap )
//                 delete d->cbrush.d->pixmap;
//             cbrush.d->pixmap = new QPixmap( pm );
        }
//         pm = *d->cbrush.d->pixmap;
        pm.x11SetScreen(d->scrn);
        if ( pm.depth() == 1 ) {
            XSetStipple(d->dpy, d->gc_brush, pm.handle());
            s = d->bg_mode == TransparentMode ? FillStippled : FillOpaqueStippled;
        } else {
            XSetTile(d->dpy, d->gc_brush, pm.handle());
            s = FillTiled;
        }
    }
    XSetFillStyle(d->dpy, d->gc_brush, s);
}



/*
  \internal
  Makes a shallow copy of the X11-specific data of \a fromDevice, if it is not
  null. Otherwise this function sets it to null.
*/

void QX11GC::copyX11Data(const QX11GC *fromDevice)
{
    setX11Data(fromDevice ? fromDevice->x11Data : 0);
}

/*
  \internal
  Makes a deep copy of the X11-specific data of \a fromDevice, if it is not
  null. Otherwise this function sets it to null.
*/

void QX11GC::cloneX11Data(const QX11GC *fromDevice)
{
    if (fromDevice && fromDevice->x11Data) {
	QX11GCData *d = new QX11GCData;
	*d = *fromDevice->x11Data;
	d->count = 0;
	setX11Data(d);
    } else {
	setX11Data(0);
    }
}

/*
  \internal
  Makes a shallow copy of the X11-specific data \a d and assigns it to this
  class. This function increments the reference code of \a d.
*/

void QX11GC::setX11Data(const QX11GCData* d)
{
    if (x11Data && x11Data->deref())
	delete x11Data;
    x11Data = (QX11GCData *)d;
    if (x11Data)
	x11Data->ref();
}


/*
  \internal
  If \a def is false, returns a deep copy of the x11Data, or 0 if x11Data is 0.
  If \a def is true, makes a QX11GCData struct filled with the default
  values.

  In either case the caller is responsible for deleting the returned
  struct. But notice that the struct is a shared class, so other
  classes might also have a reference to it. The reference count of
  the returned QX11GCData* is 0.
*/

QX11GCData* QX11GC::getX11Data(bool def) const
{
    QX11GCData* res = 0;
    if (def) {
	res = new QX11GCData;
	res->x_display = x11AppDisplay();
	res->x_screen = x11AppScreen();
	res->x_depth = x11AppDepth();
	res->x_cells = x11AppCells();
	res->x_colormap = x11Colormap();
	res->x_defcolormap = x11AppDefaultColormap();
	res->x_visual = x11AppVisual();
	res->x_defvisual = x11AppDefaultVisual();
	res->deref();
    } else if (x11Data) {
	res = new QX11GCData;
	*res = *x11Data;
	res->count = 0;
    }
    return res;
}

static const short ropCodes[] = {                     // ROP translation table
    GXcopy, // CopyROP
    GXor, // OrROP
    GXxor, // XorROP
    GXandInverted, // NotAndROP EraseROP
    GXcopyInverted, // NotCopyROP
    GXorInverted, // NotOrROP
    GXequiv, // NotXorROP
    GXand, // AndROP
    GXinvert, // NotROP
    GXclear, // ClearROP
    GXset, // SetROP
    GXnoop, // NopROP
    GXandReverse, // AndNotROP
    GXorReverse, // OrNotROP
    GXnand, // NandROP
    GXnor // NorROP
};

void QX11GC::setRasterOp(RasterOp r)
{
    if (!isActive()) {
        qWarning("QX11GC::setRasterOp: Call begin() first");
        return;
    }
    if ((uint)r > LastROP) {
        qWarning("QX11GC::setRasterOp: Invalid ROP code");
        return;
    }
    d->rop = r;
    if (d->penRef)
        updatePen(0);                            // get non-cached pen GC
    if (d->brushRef)
        updateBrush(0);                          // get non-cached brush GC
    XSetFunction(d->dpy, d->gc, ropCodes[d->rop]);
    XSetFunction(d->dpy, d->gc_brush, ropCodes[d->rop]);
}

void QX11GC::drawRoundRect(int x, int y, int w, int h, int xRnd, int yRnd)
{
    if (!isActive())
        return;
    if (xRnd <= 0 || yRnd <= 0) {
        drawRect(x, y, w, h);                 // draw normal rectangle
        return;
    }
    if (xRnd >= 100)                          // fix ranges
        xRnd = 99;
    if (yRnd >= 100)
        yRnd = 99;
    w--;
    h--;
    if (w <= 0 || h <= 0) {
        if (w == 0 || h == 0)
            return;
        fix_neg_rect(&x, &y, &w, &h);
    }
    int rx = (w*xRnd)/200;
    int ry = (h*yRnd)/200;
    int rx2 = 2*rx;
    int ry2 = 2*ry;
    if (d->cbrush.style() != NoBrush) {          // draw filled round rect
        int dp, ds;
        if (d->cpen.style() == NoPen) {
            dp = 0;
            ds = 1;
        } else {
            dp = 1;
            ds = 0;
        }
#define SET_ARC(px, py, w, h, a1, a2) \
    a->x=px; a->y=py; a->width=w; a->height=h; a->angle1=a1; a->angle2=a2; a++
        XArc arcs[4];
        XArc *a = arcs;
        SET_ARC( x+w-rx2, y, rx2, ry2, 0, 90*64 );
        SET_ARC( x, y, rx2, ry2, 90*64, 90*64 );
        SET_ARC( x, y+h-ry2, rx2, ry2, 180*64, 90*64 );
        SET_ARC( x+w-rx2, y+h-ry2, rx2, ry2, 270*64, 90*64 );
        XFillArcs(d->dpy, d->hd, d->gc_brush, arcs, 4);
#undef SET_ARC
#define SET_RCT(px, py, w, h) \
    r->x=px; r->y=py; r->width=w; r->height=h; r++
        XRectangle rects[3];
        XRectangle *r = rects;
        SET_RCT( x+rx, y+dp, w-rx2, ry );
        SET_RCT( x+dp, y+ry, w+ds, h-ry2 );
        SET_RCT( x+rx, y+h-ry, w-rx2, ry+ds );
        XFillRectangles(d->dpy, d->hd, d->gc_brush, rects, 3);
#undef SET_RCT
    }
    if (d->cpen.style() != NoPen) {              // draw outline
#define SET_ARC(px, py, w, h, a1, a2) \
    a->x=px; a->y=py; a->width=w; a->height=h; a->angle1=a1; a->angle2=a2; a++
        XArc arcs[4];
        XArc *a = arcs;
        SET_ARC( x+w-rx2, y, rx2, ry2, 0, 90*64 );
        SET_ARC( x, y, rx2, ry2, 90*64, 90*64 );
        SET_ARC( x, y+h-ry2, rx2, ry2, 180*64, 90*64 );
        SET_ARC( x+w-rx2, y+h-ry2, rx2, ry2, 270*64, 90*64 );
        XDrawArcs(d->dpy, d->hd, d->gc, arcs, 4);
#undef SET_ARC
#define SET_SEG(xp1, yp1, xp2, yp2) \
    s->x1=xp1; s->y1=yp1; s->x2=xp2; s->y2=yp2; s++
        XSegment segs[4];
        XSegment *s = segs;
        SET_SEG( x+rx, y, x+w-rx, y );
        SET_SEG( x+rx, y+h, x+w-rx, y+h );
        SET_SEG( x, y+ry, x, y+h-ry );
        SET_SEG( x+w, y+ry, x+w, y+h-ry );
        XDrawSegments(d->dpy, d->hd, d->gc, segs, 4);
#undef SET_SET
    }
}

void QX11GC::drawEllipse(int x, int y, int w, int h)
{
    if (!isActive())
        return;
    if (w == 1 && h == 1) {
	XDrawPoint(d->dpy, d->hd, (d->cpen.style() == NoPen) ? d->gc_brush : d->gc, x, y);
	return;
    }
    w--;
    h--;
    if (d->cbrush.style() != NoBrush) {          // draw filled ellipse
        XFillArc(d->dpy, d->hd, d->gc_brush, x, y, w, h, 0, 360*64);
        if (d->cpen.style() == NoPen) {
            XDrawArc(d->dpy, d->hd, d->gc_brush, x, y, w, h, 0, 360*64);
            return;
        }
    }
    if (d->cpen.style() != NoPen)                // draw outline
        XDrawArc(d->dpy, d->hd, d->gc, x, y, w, h, 0, 360*64);
}

void QX11GC::drawArc(int x, int y, int w, int h, int a, int alen)
{
    if (!isActive())
        return;
    w--;
    h--;
    if (w <= 0 || h <= 0) {
        if (w == 0 || h == 0)
            return;
        fix_neg_rect(&x, &y, &w, &h);
    }
    if (d->cpen.style() != NoPen)
        XDrawArc(d->dpy, d->hd, d->gc, x, y, w, h, a*4, alen*4);
}

void QX11GC::drawPie(int x, int y, int w, int h, int a, int alen)
{
    // Make sure "a" is 0..360*16, as otherwise a*4 may overflow 16 bits.
    if (a > (360*16)) {
        a = a % (360*16);
    } else if (a < 0) {
        a = a % (360*16);
        if (a < 0) a += (360*16);
    }

    if (!isActive())
        return;

    XSetArcMode(d->dpy, d->gc_brush, ArcPieSlice);

    w--;
    h--;
    if (w <= 0 || h <= 0) {
        if (w == 0 || h == 0)
            return;
        fix_neg_rect(&x, &y, &w, &h);
    }

    GC g = d->gc;
    bool nopen = d->cpen.style() == NoPen;

    if (d->cbrush.style() != NoBrush) {          // draw filled pie
        XFillArc(d->dpy, d->hd, d->gc_brush, x, y, w, h, a*4, alen*4);
        if (nopen) {
            g = d->gc_brush;
            nopen = false;
        }
    }
    if ( !nopen ) {                             // draw pie outline
        double w2 = 0.5*w;                      // with, height in ellipsis
        double h2 = 0.5*h;
        double xc = (double)x+w2;
        double yc = (double)y+h2;
        double ra1 = Q_PI/2880.0*a;             // convert a, alen to radians
        double ra2 = ra1 + Q_PI/2880.0*alen;
        int xic = qRound(xc);
        int yic = qRound(yc);
        XDrawLine(d->dpy, d->hd, g, xic, yic,
		  qRound(xc + qcos(ra1)*w2), qRound(yc - qsin(ra1)*h2));
        XDrawLine(d->dpy, d->hd, g, xic, yic,
		  qRound(xc + qcos(ra2)*w2), qRound(yc - qsin(ra2)*h2));
        XDrawArc(d->dpy, d->hd, g, x, y, w, h, a*4, alen*4);
    }
}

void QX11GC::drawChord(int x, int y, int w, int h, int a, int alen)
{
    if (!isActive())
        return;

    XSetArcMode(d->dpy, d->gc_brush, ArcChord);

    w--;
    h--;
    if (w <= 0 || h <= 0) {
        if (w == 0 || h == 0)
            return;
        fix_neg_rect(&x, &y, &w, &h);
    }

    GC g = d->gc;
    bool nopen = d->cpen.style() == NoPen;

    if (d->cbrush.style() != NoBrush) {          // draw filled chord
        XFillArc(d->dpy, d->hd, d->gc_brush, x, y, w, h, a*4, alen*4);
        if (nopen) {
            g = d->gc_brush;
            nopen = false;
        }
    }
    if (!nopen) {                             // draw chord outline
        double w2 = 0.5*w;                      // with, height in ellipsis
        double h2 = 0.5*h;
        double xc = (double)x+w2;
        double yc = (double)y+h2;
        double ra1 = Q_PI/2880.0*a;             // convert a, alen to radians
        double ra2 = ra1 + Q_PI/2880.0*alen;
        XDrawLine(d->dpy, d->hd, g,
		  qRound(xc + qcos(ra1)*w2), qRound(yc - qsin(ra1)*h2),
		  qRound(xc + qcos(ra2)*w2), qRound(yc - qsin(ra2)*h2));
        XDrawArc(d->dpy, d->hd, g, x, y, w, h, a*4, alen*4);
    }

    XSetArcMode(d->dpy, d->gc_brush, ArcPieSlice);
}

void QX11GC::drawLineSegments(const QPointArray &a, int index, int nlines)
{
    if (nlines < 0)
        nlines = a.size()/2 - index/2;
    if (index + nlines*2 > (int)a.size())
        nlines = (a.size() - index)/2;
    if (!isActive() || nlines < 1 || index < 0)
        return;
    QPointArray pa = a;
    if (d->cpen.style() != NoPen)
        XDrawSegments(d->dpy, d->hd, d->gc,
		      (XSegment*)(pa.shortPoints( index, nlines*2 )), nlines);
}

void QX11GC::drawPolyline(const QPointArray &a, int index, int npoints)
{
    if (npoints < 0)
        npoints = a.size() - index;
    if (index + npoints > (int)a.size())
        npoints = a.size() - index;
    if (!isActive() || npoints < 2 || index < 0)
        return;
    QPointArray pa = a;
    if (d->cpen.style() != NoPen) {
        while(npoints > 65535) {
            XDrawLines(d->dpy, d->hd, d->gc, (XPoint*)(pa.shortPoints(index, 65535)),
		       65535, CoordModeOrigin);
            npoints -= 65535;
            index += 65535;
        }
        XDrawLines(d->dpy, d->hd, d->gc, (XPoint*)(pa.shortPoints(index, npoints)),
		   npoints, CoordModeOrigin);
    }
}

int global_polygon_shape = Complex;

void QX11GC::drawPolygon(const QPointArray &a, bool winding, int index, int npoints)
{
    if (npoints < 0)
        npoints = a.size() - index;
    if (index + npoints > (int)a.size())
        npoints = a.size() - index;
    if (!isActive() || npoints < 2 || index < 0)
        return;
    QPointArray pa = a;
    if (winding)                              // set to winding fill rule
        XSetFillRule(d->dpy, d->gc_brush, WindingRule);

    if (pa[index] != pa[index + npoints - 1]) {   // close open pointarray
        pa.detach();
        pa.resize(index + npoints + 1);
        pa.setPoint(index + npoints, pa[index]);
        npoints++;
    }

    if (d->cbrush.style() != NoBrush) {          // draw filled polygon
        XFillPolygon(d->dpy, d->hd, d->gc_brush,
		     (XPoint*)(pa.shortPoints(index, npoints)),
		     npoints, global_polygon_shape, CoordModeOrigin);
    }
    if (d->cpen.style() != NoPen) {              // draw outline
        XDrawLines(d->dpy, d->hd, d->gc, (XPoint*)(pa.shortPoints(index, npoints)),
		   npoints, CoordModeOrigin);
    }
    if (winding)                              // set to normal fill rule
        XSetFillRule(d->dpy, d->gc_brush, EvenOddRule);
}

void QX11GC::drawConvexPolygon(const QPointArray &pa, int index, int npoints)
{
    global_polygon_shape = Convex;
    drawPolygon(pa, false, index, npoints);
    global_polygon_shape = Complex;
}

void QX11GC::drawCubicBezier(const QPointArray &a, int index)
{
    if (!isActive())
        return;
    if (a.size() - index < 4) {
        qWarning( "QX11GC::drawCubicBezier: Cubic Bezier needs 4 control "
                  "points" );
        return;
    }
    QPointArray pa(a);
    if (index != 0 || a.size() > 4) {
        pa = QPointArray(4);
        for (int i = 0; i < 4; i++)
            pa.setPoint(i, a.point(index + i));
    }
    if (d->cpen.style() != NoPen) {
        pa = pa.cubicBezier();
        XDrawLines(d->dpy, d->hd, d->gc, (XPoint*)pa.shortPoints(), pa.size(),
		   CoordModeOrigin);
    }
}

void QX11GC::drawPixmap(int x, int y, const QPixmap &pixmap, int sx, int sy, int sw, int sh)
{
    if ( !isActive() || pixmap.isNull() )
        return;

    // right/bottom
    if ( sw < 0 )
        sw = pixmap.width()  - sx;
    if ( sh < 0 )
        sh = pixmap.height() - sy;

    // Sanity-check clipping
    if ( sx < 0 ) {
        x -= sx;
        sw += sx;
        sx = 0;
    }
    if ( sw + sx > pixmap.width() )
        sw = pixmap.width() - sx;
    if ( sy < 0 ) {
        y -= sy;
        sh += sy;
        sy = 0;
    }
    if ( sh + sy > pixmap.height() )
        sh = pixmap.height() - sy;

    if ( sw <= 0 || sh <= 0 )
        return;

    if ( d->pdev->x11Screen() != pixmap.x11Screen() ) {
        QPixmap* p = (QPixmap*) &pixmap;
        p->x11SetScreen( d->pdev->x11Screen() );
    }

    QPixmap::x11SetDefaultScreen( pixmap.x11Screen() );

    QBitmap *mask = (QBitmap *)pixmap.mask();
    bool mono = pixmap.depth() == 1;

    if ( mask && !hasClipping() && d->pdev != paintEventDevice ) {
        if ( mono ) {                           // needs GCs pen // color
//             bool selfmask = pixmap.data->selfmask;
	    bool selfmask = false;
            if ( selfmask ) {
                XSetFillStyle( d->dpy, d->gc, FillStippled );
                XSetStipple( d->dpy, d->gc, pixmap.handle() );
            } else {
                XSetFillStyle( d->dpy, d->gc, FillOpaqueStippled );
                XSetStipple( d->dpy, d->gc, pixmap.handle() );
                XSetClipMask( d->dpy, d->gc, mask->handle() );
                XSetClipOrigin( d->dpy, d->gc, x-sx, y-sy );
            }
            XSetTSOrigin( d->dpy, d->gc, x-sx, y-sy );
            XFillRectangle( d->dpy, d->hd, d->gc, x, y, sw, sh );
            XSetTSOrigin( d->dpy, d->gc, 0, 0 );
            XSetFillStyle( d->dpy, d->gc, FillSolid );
            if ( !selfmask ) {
                if ( d->pdev == paintEventDevice && paintEventClipRegion ) {
                    x11SetClipRegion( d->dpy, d->gc, d->rendhd, *paintEventClipRegion );
		} else {
                    XSetClipMask( d->dpy, d->gc, None );
		}
            }
        } else {
            bitBlt( d->pdev, x, y, &pixmap, sx, sy, sw, sh, d->rop );
        }
        return;
    }

    QRegion rgn = d->crgn; // ### remove this
//     rgn.translate(-redirection_offset);

    if ( mask ) {                               // pixmap has clip mask
        // Implies that clipping is on, either explicit or implicit
        // Create a new mask that combines the mask with the clip region

        if ( d->pdev == paintEventDevice && paintEventClipRegion ) {
            if ( hasClipping() )
                rgn = rgn.intersect( *paintEventClipRegion );
            else
                rgn = *paintEventClipRegion;
        }
	QRect rg = rgn.boundingRect();
	
        QBitmap *comb = new QBitmap( sw, sh );
        comb->detach();
        GC cgc = qt_xget_temp_gc( pixmap.x11Screen(), TRUE );   // get temporary mono GC
        XSetForeground( d->dpy, cgc, 0 );
        XFillRectangle( d->dpy, comb->handle(), cgc, 0, 0, sw, sh );
        XSetBackground( d->dpy, cgc, 0 );
        XSetForeground( d->dpy, cgc, 1 );
	int num;
	XRectangle *rects = (XRectangle *)qt_getClipRects( rgn, num );
        XSetClipRectangles( d->dpy, cgc, -x, -y, rects, num, Unsorted );
        XSetFillStyle( d->dpy, cgc, FillOpaqueStippled );
        XSetStipple( d->dpy, cgc, mask->handle() );
        XSetTSOrigin( d->dpy, cgc, -sx, -sy );
        XFillRectangle( d->dpy, comb->handle(), cgc, 0, 0, sw, sh );
        XSetTSOrigin( d->dpy, cgc, 0, 0 );         // restore cgc
        XSetFillStyle( d->dpy, cgc, FillSolid );
        XSetClipMask( d->dpy, cgc, None );
        mask = comb;                            // it's deleted below

        XSetClipMask( d->dpy, d->gc, mask->handle() );
        XSetClipOrigin( d->dpy, d->gc, x, y );
    }

    if ( mono ) {
        XSetBackground( d->dpy, d->gc, d->bg_brush.color().pixel(d->scrn) );
        XSetFillStyle( d->dpy, d->gc, FillOpaqueStippled );
        XSetStipple( d->dpy, d->gc, pixmap.handle() );
        XSetTSOrigin( d->dpy, d->gc, x-sx, y-sy );
        XFillRectangle( d->dpy, d->hd, d->gc, x, y, sw, sh );
        XSetTSOrigin( d->dpy, d->gc, 0, 0 );
        XSetFillStyle( d->dpy, d->gc, FillSolid );
    } else {
#ifndef QT_NO_XRENDER
	QPixmap *alpha = 0;//pixmap.data->alphapm;

	if ( d->rendhd && pixmap.x11RenderHandle() &&
	     alpha && alpha->x11RenderHandle()) {
	    XRenderComposite(d->dpy, PictOpOver, pixmap.x11RenderHandle(),
			     alpha->x11RenderHandle(), d->rendhd,
			     sx, sy, sx, sy, x, y, sw, sh);
	} else
#endif // QT_NO_XRENDER
	{
	    XCopyArea( d->dpy, pixmap.handle(), d->hd, d->gc, sx, sy, sw, sh, x, y );
	}
    }

    if ( mask ) {                               // restore clipping
        XSetClipOrigin( d->dpy, d->gc, 0, 0 );
        XSetRegion( d->dpy, d->gc, rgn.handle() );
        delete mask;                            // delete comb, created above
    }
}

void QX11GC::updateRasterOp(QPainterState *ps)
{
    Q_ASSERT(isActive());
    d->rop = ps->rasterOp;
    if ( d->penRef )
        updatePen(ps);                            // get non-cached pen GC
    if ( d->brushRef )
        updateBrush(ps);                          // get non-cached brush GC
    XSetFunction(d->dpy, d->gc, ropCodes[d->rop]);
    XSetFunction(d->dpy, d->gc_brush, ropCodes[d->rop]);
}

void QX11GC::updateBackground(QPainterState *ps)
{
    Q_ASSERT(isActive());
    d->bg_mode = ps->bgMode;
    if ( !d->penRef )
        updatePen(ps);                            // update pen setting
    if ( !d->brushRef )
        updateBrush(ps);                          // update brush setting
}

void QX11GC::updateXForm(QPainterState *ps)
{

}

void QX11GC::updateClipRegion(QPainterState *ps)
{
    Q_ASSERT(isActive());
    
    clearf(ClipOn);
    // ### Extremely expensive matr. mult. for every set call - what to do with that?
    // if (m == CoordDevice)
    //     crgn = rgn;
    // else
    //     crgn = xmat * rgn;
     if (ps->VxF || ps->WxF)
	 ps->clipRegion = ps->worldMatrix * ps->clipRegion;
    if (ps->clipEnabled) {
 	if (d->pdev == paintEventDevice && paintEventClipRegion)
 	    ps->clipRegion = ps->clipRegion.intersect(*paintEventClipRegion);
	x11SetClipRegion(d->dpy, d->gc, d->rendhd, ps->clipRegion, d->gc_brush);
	setf(ClipOn);
    } else {
        if (d->pdev == paintEventDevice && paintEventClipRegion) {
	    x11SetClipRegion(d->dpy, d->gc, d->rendhd, *paintEventClipRegion, d->gc_brush);
        } else {
            XSetClipMask(d->dpy, d->gc, None);
            XSetClipMask(d->dpy, d->gc_brush, None);

#ifndef QT_NO_XRENDER
            if (d->rendhd) {
                XRenderPictureAttributes pattr;
                pattr.clip_mask = None;
                XRenderChangePicture(d->dpy, d->rendhd, CPClipMask, &pattr);
            }
#endif // QT_NO_XRENDER
	}
    }
    d->crgn = ps->clipRegion;
}

void QX11GC::updateFont(QPainterState *ps)
{
    
}

void QX11GC::drawTextItem(int x, int y, const QTextItem &ti, int textflags)
{
    
}

Qt::HANDLE QX11GC::handle() const
{
    Q_ASSERT(isActive());
    Q_ASSERT(d->hd);
    return d->hd;
}

#include "qwsgc_qws.h"
#include "q4painter_p.h"

#include "qgfx_qws.h"
#include "qregion.h"
#include "qbitmap.h"
#include "qpixmapcache.h"


QWSGC::QWSGC(const QPaintDevice *)
    :mygfx(0)//, d(0), qwsData(0)
{
	qDebug("QWSGC::QWSGC");
}
QWSGC::~QWSGC()
{
	qDebug("QWSGC::~QWSGC");
}

bool QWSGC::begin(const QPaintDevice *pdev, QPainterState *ps, bool begin)
{
        if ( isActive() ) {                         // already active painting
         qWarning( "QWSC::begin: Painter is already active."
                   "\n\tYou must end() the painter before a second begin()" );
	return true;
    }

    Q_ASSERT(mygfx == 0);
    mygfx = pdev->graphicsContext();
    qDebug("QWSGC::begin %p gfx %p", this, mygfx);
    setActive(true);

    updatePen(ps);
    updateBrush(ps);
    updateClipRegion(ps);

    return true;
}
bool QWSGC::end(){
    setActive(false);
    qDebug("QWSGC::end %p (gfx %p)", this, mygfx);
    delete mygfx;
    mygfx = 0;
    return true;
}

void QWSGC::updatePen(QPainterState *ps)
{
    mygfx->setPen(ps->pen);
    
//    qDebug("QWSGC::updatePen");
}
void QWSGC::updateBrush(QPainterState *ps)
{
    //qDebug("QWSGC::updateBrush");

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
	dense1_pat, dense2_pat, dense3_pat, dense4_pat, dense5_pat,
	dense6_pat, dense7_pat,
	hor_pat, ver_pat, cross_pat, bdiag_pat, fdiag_pat, dcross_pat };

    if ( !mygfx )
	return;

    uchar * pat=0;
    int bs=ps->brush.style();
    int d=0;
    if( bs>=Dense1Pattern && bs <= DiagCrossPattern ) {
	pat=pat_tbl[ bs-Dense1Pattern ];
	if(bs<=Dense7Pattern)
	    d=8;
	else if (bs<=CrossPattern)
	    d=24;
	else
	    d=16;
    }
    if ( bs == CustomPattern || pat ) {
        QPixmap *pm;
        if ( pat ) {
            QString key="$qt-brush$" + QString::number( bs );
            pm = QPixmapCache::find( key );
            bool del = FALSE;
            if ( !pm ) {                        // not already in pm dict
		pm = new QBitmap( d, d, pat, TRUE );
		del = !QPixmapCache::insert( key, pm );
            }
	    if ( ps->brush.d->pixmap )
		delete ps->brush.d->pixmap;
	    ps->brush.d->pixmap = new QPixmap( *pm );
	    if (del) delete pm;
	}
	pm = ps->brush.d->pixmap;
    }

    mygfx->setBrush(ps->brush);
    mygfx->setBrushPixmap( ps->brush.d->pixmap );
}

void QWSGC::updateFont(QPainterState *ps){
    qDebug("QWSGC::updateFont");
}
void QWSGC::updateRasterOp(QPainterState *ps){
    qDebug("QWSGC::updateRasterOp");
}
void QWSGC::updateBackground(QPainterState *ps){
    qDebug("QWSGC::updateBackground");
}
void QWSGC::updateXForm(QPainterState *ps)
{
//    qDebug("QWSGC::updateXForm");
}
void QWSGC::updateClipRegion(QPainterState *ps){
    qDebug("QWSGC::updateClipRegion");
}

void QWSGC::setRasterOp(RasterOp r){
    qDebug("QWSGC::setRasterOp");
}

void QWSGC::drawLine(int x1, int y1, int x2, int y2)
{
    if ( !isActive() )
	return;
//####    if ( cpen.style() != NoPen )
	mygfx->drawLine( x1, y1, x2, y2 );
}
void QWSGC::drawRect(int x1, int y1, int w, int h)
{
    //qDebug("QWSGC::drawRect");
    if ( !isActive() || w == 0 || h == 0 )
	return;


    if ( w <= 0 || h <= 0 )
	fix_neg_rect( &x1, &y1, &w, &h );

    //############ gfx->setBrushOffset( x-bro.x(), y-bro.y() );

    if ( 1 /* cpen.style() != NoPen */ ) {
    /*###########
	if ( cpen.width() > 1 ) {
	    QPointArray a( QRect(x1,y1,w,h), TRUE );
	    drawPolyInternal( a );
	    return;
	} else
    */
	{
	    int x2 = x1 + (w-1);
	    int y2 = y1 + (h-1);
	    mygfx->drawLine(x1, y1, x2, y1);
	    mygfx->drawLine(x2, y1, x2, y2);
	    mygfx->drawLine(x1, y2, x2, y2);
	    mygfx->drawLine(x1, y1, x1, y2);
	    x1 += 1;
	    y1 += 1;
	    w -= 2;
	    h -= 2;
	}
    }

    mygfx->fillRect( x1, y1, w, h );
}
void QWSGC::drawPoint(int x, int y)
{
    if ( !isActive() )
	return;
    mygfx->drawPoint(x,y);
}

void QWSGC::drawPoints(const QPointArray &pa, int index, int npoints){
    qDebug("QWSGC::drawPoints");
}
void QWSGC::drawWinFocusRect(int x, int y, int w, int h, bool xorPaint, const QColor &penColor){
    qDebug("QWSGC::drawWinFocusRect");
}
void QWSGC::drawRoundRect(int x, int y, int w, int h, int xRnd, int yRnd){
    qDebug("QWSGC::drawRoundRect");
}
void QWSGC::drawEllipse(int x, int y, int w, int h){
    qDebug("QWSGC::drawEllipse");
}
void QWSGC::drawArc(int x, int y, int w, int h, int a, int alen){
    qDebug("QWSGC::drawArc");
}
void QWSGC::drawPie(int x, int y, int w, int h, int a, int alen){
    qDebug("QWSGC::drawPie");
}
void QWSGC::drawChord(int x, int y, int w, int h, int a, int alen){
    qDebug("QWSGC::drawChord");
}
void QWSGC::drawLineSegments(const QPointArray &a, int index, int nlines)
{
    if (nlines < 0)
        nlines = a.size()/2 - index/2;
    if (index + nlines*2 > (int)a.size())
        nlines = (a.size() - index)/2;
    if (!isActive() || nlines < 1 || index < 0)
        return;

    for ( int i=0; i<nlines; i++ ) {
	int x1,y1,x2,y2;
	a.point( index++, &x1, &y1 );
	a.point( index++, &x2, &y2 );
	//#### if ( cpen.style() != NoPen )
	    mygfx->drawLine( x1, y1, x2, y2 );
    }
}
void QWSGC::drawPolyline(const QPointArray &pa, int index, int npoints)
{
    //qDebug("QWSGC::drawPolyline");

    if ( npoints < 0 )
	npoints = pa.size() - index;
    if ( index + npoints > pa.size() )
	npoints = pa.size() - index;
    if ( !isActive() || npoints < 2 || index < 0 )
	return;
    //####### if ( cpen.style() != NoPen )
	mygfx->drawPolyline( pa, index, npoints );
}
void QWSGC::drawPolygon(const QPointArray &pa, bool winding, int index, int npoints){
    qDebug("QWSGC::drawPolygon");
}
void QWSGC::drawConvexPolygon(const QPointArray &pa, int index, int npoints){
    qDebug("QWSGC::drawConvexPolygon");
}
#ifndef QT_NO_BEZIER
void QWSGC::drawCubicBezier(const QPointArray &pa, int index){
    qDebug("QWSGC::drawCubicBezier");
}
#endif

void QWSGC::drawPixmap(int x, int y, const QPixmap &pixmap, int sx, int sy, int sw, int sh)
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

    //bitBlt( pdev, x, y, &pixmap, sx, sy, sw, sh, CopyROP );
    mygfx->setSource(&pixmap);
    if(sw>pixmap.width()) { // ###hanord: isn't this done above?
	sw=pixmap.width();
    }
    if(sh>pixmap.height()) {
	sh=pixmap.height();
    }
    if(pixmap.mask()) {
	QBitmap * mymask=( (QBitmap *)pixmap.mask() );
	unsigned char * thebits=mymask->scanLine(0);
	int ls=mymask->bytesPerLine();
	mygfx->setAlphaType(QGfx::LittleEndianMask);
	mygfx->setAlphaSource(thebits,ls);
    } else if ( pixmap.data->hasAlpha ){
	mygfx->setAlphaType(QGfx::InlineAlpha);
    } else {
	mygfx->setAlphaType(QGfx::IgnoreAlpha);
    }
    mygfx->blt(x,y,sw,sh,sx,sy);

}
void QWSGC::drawTextItem(int x, int y, const QTextItem &ti, int textflags){
    qDebug("QWSGC::drawTextItem");
}

Qt::HANDLE QWSGC::handle() const{
    qDebug("QWSGC::handle");
    return 0;
}


void QWSGC::initialize(){
	qDebug("QWSGC::initialize");
}
    void QWSGC::cleanup(){
	qDebug("QWSGC::cleanup");
}



/* paintevent magic to provide Windows semantics on Qt/E
 */
static QRegion* paintEventClipRegion = 0;
static QRegion* paintEventSaveRegion = 0;
static QPaintDevice* paintEventDevice = 0;

void qt_set_paintevent_clipping( QPaintDevice* dev, const QRegion& region)
{
    if ( !paintEventClipRegion )
	paintEventClipRegion = new QRegion( region );
    else
	*paintEventClipRegion = region;
    paintEventDevice = dev;

#ifdef QWS_EXTRA_DEBUG
    qDebug( "qt_set_paintevent_clipping" );
    QMemArray<QRect> ar = region.rects();
    for ( int i=0; i<int(ar.size()); i++ ) {
	QRect r = ar[i];
        qDebug( "   r[%d]:  %d,%d %dx%d", i,
		r.x(), r.y(), r.width(), r.height() );
    }
#endif

}

void qt_clear_paintevent_clipping()
{
    delete paintEventClipRegion;
    delete paintEventSaveRegion;
    paintEventClipRegion = 0;
    paintEventSaveRegion = 0;
    paintEventDevice = 0;
}


static QList<QPainter*> *widgetPainterList = 0;

void qwsUpdateActivePainters()
{
    /* ##############
    if ( widgetPainterList ) {
	for (int i = 0; i < widgetPainterList->size(); ++i) {
	    QPainter *ptr = widgetPainterList->at(i);
	    ptr->save();
	    delete ptr->gfx;
	    ptr->gfx = ptr->device()->graphicsContext();
	    ptr->setf( QPainter::VolatileDC );
	    ptr->restore();
	}
    }
    */
}


void qt_draw_background( QPainter *pp, int x, int y, int w,  int h )
{
    QPaintDevice *pd = pp->device();
    QWSGC *p = static_cast<QWSGC *>(pd->gc());
// // //     XSetForeground( p->d->dpy, p->d->gc, p->d->bg_brush.color().pixel(p->d->scrn) );
// // //     qt_draw_transformed_rect( pp, x, y, w, h, TRUE);
// // //     XSetForeground( p->d->dpy, p->d->gc, p->d->cpen.color().pixel(p->d->scrn) );
}
// ########

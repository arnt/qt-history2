/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpainter.cpp#66 $
**
** Implementation of QPainter, QPen and QBrush classes
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
** --------------------------------------------------------------------------
** This file containts the platform independent implementation of the
** QPainter class.  Platform dependent functions are implemented in the
** qptr_xxx.cpp files.
*****************************************************************************/

#define QPAINTER_C
#include "qpainter.h"
#include "qpaintdc.h"
#include "qbitmap.h"
#include "qstack.h"
#include "qdstream.h"

RCSTAG("$Id: //depot/qt/main/src/kernel/qpainter.cpp#66 $")


/*----------------------------------------------------------------------------
  \class QPainter qpainter.h
  \brief The QPainter class paints on paint devices.

  \ingroup drawing

  The painter provides graphics rendering of many different varieties
  on any QPaintDevice object, from simple line drawing to scaling,
  rotating, dithering dithered and finaly drawing a pixmap.

  Graphics can be transformed using view transformation, world
  transformation or a combination of these two.	 View transformation
  is a window/viewport transformation with translation and
  scaling. World transformation is a full 2D transformation including
  rotation and shearing.

  The typical use of a painter is:
  <ol>
  <li> Call begin() to begin painting on a device.
  <li> Set a pen, a brush etc.
  <li> Draw.
  <li> Call end() to finish painting.
  </ol>

  Example of use:
  \code
    void MyWidget::paintEvent()
    {
	QPainter paint;
	paint.begin( &this );			// start painting widget
	paint.setPen( blue );			// set blue pen
	paint.drawText( rect(),			// draw a text, centered
			AlignCenter,		//   in the widget
			"The Text" );
	paint.end();				// painting done
    }
  \endcode

  QPainter is almost never used outside \link QWidget::paintEvent()
  paintEvent(). \endlink Any widget <em>must</em> be able to repaint
  itself at any time via paintEvent(), therefore the only sane design
  is to do the painting in paintEvent() and use either
  QWidget::update() or QWidget::repaint() force a paint event as
  necessary.

  Note that both painters and some paint devices have attributes such
  as current font, current foreground colors and so on.

  QPainter::begin() copies these attributes from the paint device, and
  changing a paint device's attributes will have effect only the next
  time a painter is opened on it.

  \warning QPainter::begin resets all attributes to their default
  values, from the device, thus setting fonts, brushes, etc, before
  begin() will have \e no effect.

  \header qdrawutl.h

  \sa QPaintDevice, QWidget, QPixmap
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \internal
  Sets or clears a pointer flag.
 ----------------------------------------------------------------------------*/

void QPainter::setf( ushort b, bool v )
{
    if ( v )
	setf( b );
    else
	clearf( b );
}


/*----------------------------------------------------------------------------
  \fn bool QPainter::isActive() const
  Returns the TRUE if the painter is active painting, i.e. begin() has
  been called and end() has not yet been called.
  \sa QPaintDevice::paintingActive()
 ----------------------------------------------------------------------------*/


struct QPState {				// painter state
    QFont	font;
    QPen	pen;
    QBrush	brush;
    QColor	bgc;
    uchar	bgm;
    uchar	pu;
    uchar	rop;
    QPoint	bro;
    QRect	wr, vr;
    QWMatrix	wm;
    bool	vxf;
    bool	wxf;
    QRegion	rgn;
    bool	clip;
    int		ts;
    int	       *ta;
};

declare(QStackM,QPState);
typedef QStackM(QPState) QPStateStack;


void QPainter::killPStack()
{
    delete (QPStateStack *)ps_stack;
}

/*----------------------------------------------------------------------------
  Saves the current painter state (pushes the state onto a stack).

  A save() must have a corresponding restore().

  \sa restore()
 ----------------------------------------------------------------------------*/

void QPainter::save()
{
    if ( testf(ExtDev) ) {
	pdev->cmd( PDC_SAVE, this, 0 );
    }
    QPStateStack *pss = (QPStateStack *)ps_stack;
    if ( pss == 0 ) {
	pss = new QStackM(QPState);
	pss->setAutoDelete( TRUE );
	CHECK_PTR( pss );
	ps_stack = pss;
    }
    register QPState *ps = new QPState;
    CHECK_PTR( ps );
    ps->font  = cfont;
    ps->pen   = cpen;
    ps->brush = cbrush;
    ps->bgc   = bg_col;
    ps->bgm   = bg_mode;
    ps->rop   = rop;
    ps->bro   = bro;
#if 0
    ps->pu    = pu;				// !!!not used
#endif
    ps->wr    = QRect( wx, wy, ww, wh );
    ps->vr    = QRect( vx, vy, vw, vh );
    ps->wm    = wxmat;
    ps->vxf   = testf(VxF);
    ps->wxf   = testf(WxF);
    ps->rgn   = crgn.copy();
    ps->clip  = testf(ClipOn);
    ps->ts    = tabstops;
    ps->ta    = tabarray;
    pss->push( ps );
}

/*----------------------------------------------------------------------------
  Restores the current painter state (pops a saved state off the stack).
  \sa save()
 ----------------------------------------------------------------------------*/

void QPainter::restore()
{
    if ( testf(ExtDev) ) {
	pdev->cmd( PDC_RESTORE, this, 0 );
    }
    QPStateStack *pss = (QPStateStack *)ps_stack;
    if ( pss == 0 || pss->isEmpty() ) {
#if defined(CHECK_STATE)
	warning( "QPainter::restore: Empty stack error" );
#endif
	return;
    }
    register QPState *ps = pss->pop();
    if ( ps->font != cfont )
	setFont( ps->font );
    if ( ps->pen != cpen )
	setPen( ps->pen );
    if ( ps->brush != cbrush )
	setBrush( ps->brush );
    if ( ps->bgc != bg_col )
	setBackgroundColor( ps->bgc );
    if ( ps->bgm != bg_mode )
	setBackgroundMode( (BGMode)ps->bgm );
    if ( ps->rop != rop )
	setRasterOp( (RasterOp)ps->rop );
#if 0
    if ( ps->pu != pu )				// !!!not used
	pu = ps->pu;
#endif
    QRect wr( wx, wy, ww, wh );
    QRect vr( vx, vy, vw, vh );
    if ( ps->wr != wr )
	setWindow( ps->wr );
    if ( ps->vr != vr )
	setViewport( ps->vr );
    if ( ps->wm != wxmat )
	setWorldMatrix( ps->wm );
    if ( ps->vxf != testf(VxF) )
	setViewXForm( ps->vxf );
    if ( ps->wxf != testf(WxF) )
	setWorldXForm( ps->wxf );
    if ( ps->rgn != crgn )
	setClipRegion( ps->rgn );
    if ( ps->clip != testf(ClipOn) )
	setClipping( ps->clip );
    tabstops = ps->ts;
    tabarray = ps->ta;
    delete ps;
    if ( pss->isEmpty() ) {
	delete pss;
	ps_stack = 0;
    }
}


/*----------------------------------------------------------------------------
  \fn QFontMetrics QPainter::fontMetrics() const
  Returns the font metrics for the painter.
  Font metrics can only be obtained when the painter is active.
  \sa fontInfo(), isActive()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QFontInfo QPainter::fontInfo() const
  Returns the font info for the painter.
  Font info can only be obtained when the painter is active.
  \sa fontMetrics(), isActive()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \fn const QPen &QPainter::pen() const
  Returns the current pen for the painter.
  \sa setPen()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets a new painter pen.

  The pen defines how to draw lines and outlines, and it also defines
  the text color.

  \sa pen()
 ----------------------------------------------------------------------------*/

void QPainter::setPen( const QPen &pen )
{
    cpen = pen;
    updatePen();
}

/*----------------------------------------------------------------------------
  Sets a new painter pen with style \c style, width 0 and black color.
  \sa pen(), QPen
 ----------------------------------------------------------------------------*/

void QPainter::setPen( PenStyle style )
{
    register QPen::QPenData *d = cpen.data;	// low level access
    if ( d->count != 1 ) {
	cpen.detach();
	d = cpen.data;
    }
    d->style = style;
    d->width = 0;
    d->color = black;
    updatePen();
}

/*----------------------------------------------------------------------------
  Sets a new painter pen with style \c SolidLine, width 0 and the specified
  \e color.
  \sa pen(), QPen
 ----------------------------------------------------------------------------*/

void QPainter::setPen( const QColor &color )
{
    register QPen::QPenData *d = cpen.data;	// low level access
    if ( d->count != 1 ) {
	cpen.detach();
	d = cpen.data;
    }
    d->style = SolidLine;
    d->width = 0;
    d->color = color;
    updatePen();
}

/*----------------------------------------------------------------------------
  \fn const QBrush &QPainter::brush() const
  Returns the current painter brush.
  \sa QPainter::setBrush()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets a new painter brush.

  The brush defines how to fill shapes.

  \sa brush()
 ----------------------------------------------------------------------------*/

void QPainter::setBrush( const QBrush &brush )
{
    cbrush = brush;
    updateBrush();
}

/*----------------------------------------------------------------------------
  Sets a new painter brush with black color and the specified \e style.
  \sa brush(), QBrush
 ----------------------------------------------------------------------------*/

void QPainter::setBrush( BrushStyle style )
{
    register QBrush::QBrushData *d = cbrush.data; // low level access
    if ( d->count != 1 ) {
	cbrush.detach();
	d = cbrush.data;
    }
    d->style = style;
    d->color = black;
    if ( d->pixmap ) {
	delete d->pixmap;
	d->pixmap = 0;
    }
    updateBrush();
}

/*----------------------------------------------------------------------------
  Sets a new painter brush with the style \c SolidPattern and the specified
  \e color.
  \sa brush(), QBrush
 ----------------------------------------------------------------------------*/

void QPainter::setBrush( const QColor &color )
{
    register QBrush::QBrushData *d = cbrush.data; // low level access
    if ( d->count != 1 ) {
	cbrush.detach();
	d = cbrush.data;
    }
    d->style = SolidPattern;
    d->color = color;
    if ( d->pixmap ) {
	delete d->pixmap;
	d->pixmap = 0;
    }
    updateBrush();
}


/*----------------------------------------------------------------------------
  Set the number of pixels per tab stop to a fixed number.

  Tab stops are used when drawing formatted text with \c ExpandTabs set.
  This fixed tab stop value has lower precedence than tab array
  settings.

  \sa setTabArray(), drawText(), fontMetrics()
 ----------------------------------------------------------------------------*/

void QPainter::setTabStops( int ts )
{
    tabstops = ts;
    if ( isActive() && testf(ExtDev) ) {	// tell extended device
	QPDevCmdParam param[1];
	param[0].ival = ts;
	pdev->cmd( PDC_SETTABSTOPS, this, param );
    }
}

/*----------------------------------------------------------------------------
  Set an array containing the tab stops.

  Tab stops are used when drawing formatted text with \c ExpandTabs set.

  The last tab stop must be 0 (terminates the array).

  Notice that setting a tab array overrides any fixed tabulator stop
  that is set using setTabStops().

  \sa setTabStops(), drawText(), fontMetrics()
 ----------------------------------------------------------------------------*/

void QPainter::setTabArray( int *ta )
{
    if ( ta != tabarray ) {
	tabarraylen = 0;
	delete [] tabarray;			// delete old array
	if ( ta ) {				// tabarray = copy of 'ta'
	    while ( ta[tabarraylen] )
		tabarraylen++;
	    tabarray = new int[tabarraylen];	// duplicate ta
	    memcpy( tabarray, ta, sizeof(int)*tabarraylen );
	}
	else
	    tabarray = 0;
    }
    if ( isActive() && testf(ExtDev) ) {	// tell extended device
	QPDevCmdParam param[2];
	param[0].ival = tabarraylen;
	param[1].ivec = tabarray;
	pdev->cmd( PDC_SETTABARRAY, this, param );
    }
}


/*****************************************************************************
  QPainter xform settings
 *****************************************************************************/

/*----------------------------------------------------------------------------
  Enables view transformations if \e enable is TRUE, or disables view
  transformations if \e enable is FALSE.
  \sa setWindow(), setViewport(), setWorldMatrix(), setWorldXForm()
 ----------------------------------------------------------------------------*/

void QPainter::setViewXForm( bool enable )
{
    if ( !isActive() || enable == testf(VxF) )
	return;
    setf( VxF, enable );
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].ival = enable;
	pdev->cmd( PDC_SETVXFORM, this, param );
    }
    updateXForm();
}

/*----------------------------------------------------------------------------
  Returns the window rectangle.
  \sa setWindow(), setViewXForm()
 ----------------------------------------------------------------------------*/

QRect QPainter::window() const
{
    return QRect( wx, wy, ww, wh );
}

/*----------------------------------------------------------------------------
  Sets the window rectangle view transformation for the painter and
  enables view transformation.

  The window rectangle is part of the view transformation.  The window
  specifies the logical coordinate system.

  The window and the \link setViewport() viewport\endlink are initially set
  to \e (0,0,width,height), where \e (width,height) is the pixel size of the
  paint device.

  Example:
  \code
      QPainter p;
      p.begin( myWidget );
      p.setWindow( 0, 0, 1000, 2000 );
      p.setViewport( 100,100, 200,200 );
      p.drawPoint( 500, 500 );			// draws pixel at (150,125)
  \endcode

  The preceding example sets up a transformation that maps the logical
  coordinates (0,0,1000,2000) into a (200,200) rectangle at (100,100).

  View transformations can be combined with world transformations.
  World transformations are applied after the view transformations.

  \sa window(), setViewport(), setViewXForm(), setWorldMatrix(),
  setWorldXForm()
 ----------------------------------------------------------------------------*/

void QPainter::setWindow( int x, int y, int w, int h )
{						// set window
    if ( !isActive() )
	return;
    wx = x;
    wy = y;
    ww = w;
    wh = h;
    if ( testf(ExtDev) ) {
	QRect r( x, y, w, h );
	QPDevCmdParam param[1];
	param[0].rect = (QRect*)&r;
	pdev->cmd( PDC_SETWINDOW, this, param );
    }
    if ( testf(VxF) )
	updateXForm();
    else
	setViewXForm( TRUE );
}

/*----------------------------------------------------------------------------
  Returns the viewport rectangle.
  \sa setViewport(), setViewXForm()
 ----------------------------------------------------------------------------*/

QRect QPainter::viewport() const		// get viewport
{
    return QRect( vx, vy, vw, vh );
}

/*----------------------------------------------------------------------------
  Sets the viewport rectangle view transformation for the painter and
  enables view transformation.

  The viewport rectangle is part of the view transformation. The viewport
  specifies the device coordinate system.

  The viewport and the \link setWindow() window\endlink are initially set
  to \e (0,0,width,height), where \e (width,height) is the pixel size of
  the paint device.

  Example:
  \code
      QPainter p;
      p.begin( myWidget );
      p.setWindow( 0, 0, 1000, 2000 );
      p.setViewport( 100,100, 200,200 );
      p.drawPoint( 500, 500 );			// draws pixel at (150,125)
  \endcode

  The preceding example sets up a transformation that maps the logical
  coordinates (0,0,1000,2000) into a (200,200) rectangle at (100,100).

  View transformations can be combined with world transformations.
  World transformations are applied after the view transformations.

  \sa viewport(), setWindow(), setViewXForm(), setWorldMatrix(),
  setWorldXForm()
 ----------------------------------------------------------------------------*/

void QPainter::setViewport( int x, int y, int w, int h )
{
    vx = x;
    vy = y;
    vw = w;
    vh = h;
    if ( testf(ExtDev) ) {
	QRect r( x, y, w, h );
	QPDevCmdParam param[1];
	param[0].rect = (QRect*)&r;
	pdev->cmd( PDC_SETVIEWPORT, this, param );
    }
    if ( testf(VxF) )
	updateXForm();
    else
	setViewXForm( TRUE );
}

/*----------------------------------------------------------------------------
  Enables world transformations if \e enable is TRUE, or disables
  world transformations if \e enable is FALSE.
  \sa setWorldMatrix(), setWindow(), setViewport(), setViewXForm()
 ----------------------------------------------------------------------------*/

void QPainter::setWorldXForm( bool enable )
{
    if ( !isActive() || enable == testf(WxF) )
	return;
    setf( WxF, enable );
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].ival = enable;
	pdev->cmd( PDC_SETWXFORM, this, param );
    }
    updateXForm();
}

/*----------------------------------------------------------------------------
  Returns the world transformation matrix.
  \sa setWorldMatrix()
 ----------------------------------------------------------------------------*/

const QWMatrix &QPainter::worldMatrix() const
{
    return wxmat;
}

/*----------------------------------------------------------------------------
  Sets the world transformation matrix to \e m and enables world
  transformation.

  If \e combine is TRUE, then \e m is combined with the
  current transformation matrix, otherwise \e m will replace
  the current transformation matrix.

  World transformations are applies after the view transformations (\link
  setWindow window\endlink and \link setViewport viewport\endlink).

  The following functions can transform the coordinate system without using
  a QWMatrix:
  <ul>
  <li>translate()
  <li>scale()
  <li>shear()
  <li>rotate()
  </ul>

  They operate on the painter's \link worldMatrix internal matrix\endlink
  and are implemented like this:

  \code
    void QPainter::rotate( float a )
    {
	wxmat.rotate( a );
	setWorldMatrix( wxmat );
    }
  \endcode

  See the \link QWMatrix QWMatrix documentation\endlink for a general
  discussion on coordinate system transformations.

  \sa worldMatrix(), setWorldXForm(), setWindow(), setViewport(),
  setViewXForm()
 ----------------------------------------------------------------------------*/

void QPainter::setWorldMatrix( const QWMatrix &m, bool combine )
{
    if ( combine )
	wxmat = m * wxmat;			// combines
    else
	wxmat = m;				// set new matrix
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[2];
	param[0].matrix = &wxmat;
	param[1].ival = combine;
	pdev->cmd( PDC_SETWMATRIX, this, param );
    }
    if ( !testf(WxF) )
	setWorldXForm( TRUE );
    else
	updateXForm();
}


/*----------------------------------------------------------------------------
  Translates the coordinate system by \e (dx,dy).
  \sa scale(), shear(), rotate(), resetXForm(), setWorldMatrix(), QWMatrix
 ----------------------------------------------------------------------------*/

void QPainter::translate( float dx, float dy )
{
    wxmat.translate( dx, dy );
    setWorldMatrix( wxmat );
}

/*----------------------------------------------------------------------------
  Scales the coordinate system by \e (sx,sy).
  \sa translate(), shear(), rotate(), resetXForm(), setWorldMatrix(), QWMatrix
 ----------------------------------------------------------------------------*/

void QPainter::scale( float sx, float sy )
{
    wxmat.scale( sx, sy );
    setWorldMatrix( wxmat );
}

/*----------------------------------------------------------------------------
  Shears the coordinate system \e (sh,sv).
  \sa translate(), scale(), rotate(), resetXForm(), setWorldMatrix(), QWMatrix
 ----------------------------------------------------------------------------*/

void QPainter::shear( float sh, float sv )
{
    wxmat.shear( sv, sh );
    setWorldMatrix( wxmat );
}

/*----------------------------------------------------------------------------
  Rotates the coordinate system \e a degrees.
  \sa translate(), scale(), shear(), resetXForm(), setWorldMatrix(), QWMatrix
 ----------------------------------------------------------------------------*/

void QPainter::rotate( float a )
{
    wxmat.rotate( a );
    setWorldMatrix( wxmat );
}

/*----------------------------------------------------------------------------
  Resets any transformations that were made using translate(), scale(),
  shear(), rotate() and setWorldMatrix()
 ----------------------------------------------------------------------------*/

void QPainter::resetXForm()
{
    wxmat = QWMatrix();
    setWorldXForm( FALSE );
}


/*----------------------------------------------------------------------------
  Fills the rectangle \e (x,y,w,h) with the \e brush.

  You can specify a QColor as \e brush, since there is a QBrush constructor
  that takes a QColor argument and creates a solid pattern brush.

  \sa drawRect()
 ----------------------------------------------------------------------------*/

void QPainter::fillRect( int x, int y, int w, int h, const QBrush &brush )
{
    QPen   oldPen   = pen();			// save pen
    QBrush oldBrush = this->brush();		// save brush
    setPen( NoPen );
    setBrush( brush );
    drawRect( x, y, w, h );			// draw filled rect
    setBrush( oldBrush );			// restore brush
    setPen( oldPen );				// restore pen
}


/*****************************************************************************
  QPainter member functions (inline if DEBUG not defined)
 *****************************************************************************/

/*----------------------------------------------------------------------------
  \overload void QPainter::setBrushOrigin( const QPoint &p )
 ----------------------------------------------------------------------------*/

void QPainter::setBrushOrigin( const QPoint &p )
{
    setBrushOrigin( p.x(), p.y() );
}

/*----------------------------------------------------------------------------
  \overload void QPainter::setWindow( const QRect &r )
 ----------------------------------------------------------------------------*/

void QPainter::setWindow( const QRect &r )
{
    setWindow( r.x(), r.y(), r.width(), r.height() );
}

/*----------------------------------------------------------------------------
  \overload void QPainter::setViewport( const QRect &r )
 ----------------------------------------------------------------------------*/

void QPainter::setViewport( const QRect &r )
{
    setViewport( r.x(), r.y(), r.width(), r.height() );
}

/*----------------------------------------------------------------------------
  Sets the clip region to \e (x,y,w,h) and enables clipping.
  \sa setClipRegion(), setClipping()
 ----------------------------------------------------------------------------*/

void QPainter::setClipRect( int x, int y, int w, int h )
{
    setClipRect( QRect(x,y,w,h) );
}

/*----------------------------------------------------------------------------
  \overload void QPainter::drawPoint( const QPoint &p )
 ----------------------------------------------------------------------------*/

void QPainter::drawPoint( const QPoint &p )
{
    drawPoint( p.x(), p.y() );
}

/*----------------------------------------------------------------------------
  \overload void QPainter::moveTo( const QPoint &p )
 ----------------------------------------------------------------------------*/

void QPainter::moveTo( const QPoint &p )
{
    moveTo( p.x(), p.y() );
}

/*----------------------------------------------------------------------------
  \overload void QPainter::lineTo( const QPoint &p )
 ----------------------------------------------------------------------------*/

void QPainter::lineTo( const QPoint &p )
{
    lineTo( p.x(), p.y() );
}

/*----------------------------------------------------------------------------
  \overload void QPainter::drawLine( const QPoint &p1, const QPoint &p2 )
 ----------------------------------------------------------------------------*/

void QPainter::drawLine( const QPoint &p1, const QPoint &p2 )
{
    drawLine( p1.x(), p1.y(), p2.x(), p2.y() );
}

/*----------------------------------------------------------------------------
  \overload void QPainter::drawRect( const QRect &r )
 ----------------------------------------------------------------------------*/

void QPainter::drawRect( const QRect &r )
{
    drawRect( r.x(), r.y(), r.width(), r.height() );
}

/*----------------------------------------------------------------------------
  \overload void QPainter::drawRoundRect( const QRect &r, int xRnd, int yRnd )
 ----------------------------------------------------------------------------*/

void QPainter::drawRoundRect( const QRect &r, int xRnd, int yRnd )
{
    drawRoundRect( r.x(), r.y(), r.width(), r.height(), xRnd, yRnd );
}

/*----------------------------------------------------------------------------
  \overload void QPainter::drawEllipse( const QRect &r )
 ----------------------------------------------------------------------------*/

void QPainter::drawEllipse( const QRect &r )
{
    drawEllipse( r.x(), r.y(), r.width(), r.height() );
}

/*----------------------------------------------------------------------------
  \overload void QPainter::drawArc( const QRect &r, int a, int alen )
 ----------------------------------------------------------------------------*/

void QPainter::drawArc( const QRect &r, int a, int alen )
{
    drawArc( r.x(), r.y(), r.width(), r.height(), a, alen );
}

/*----------------------------------------------------------------------------
  \overload void QPainter::drawPie( const QRect &r, int a, int alen )
 ----------------------------------------------------------------------------*/

void QPainter::drawPie( const QRect &r, int a, int alen )
{
    drawPie( r.x(), r.y(), r.width(), r.height(), a, alen );
}

/*----------------------------------------------------------------------------
  \overload void QPainter::drawChord( const QRect &r, int a, int alen )
 ----------------------------------------------------------------------------*/

void QPainter::drawChord( const QRect &r, int a, int alen )
{
    drawChord( r.x(), r.y(), r.width(), r.height(), a, alen );
}

/*----------------------------------------------------------------------------
  \overload void QPainter::drawPixmap( const QPoint &p, const QPixmap &pm, const QRect &sr )
 ----------------------------------------------------------------------------*/

void QPainter::drawPixmap( const QPoint &p, const QPixmap &pm,
			   const QRect &sr )
{
    drawPixmap( p.x(), p.y(), pm, sr.x(), sr.y(), sr.width(), sr.height() );
}

/*----------------------------------------------------------------------------
  \overload void QPainter::drawPixmap( const QPoint &p, const QPixmap &pm )

  This version of the call draws the entire pixmap.
 ----------------------------------------------------------------------------*/

void QPainter::drawPixmap( const QPoint &p, const QPixmap &pm )
{
    drawPixmap( p.x(), p.y(), pm, 0, 0, pm.width(), pm.height() );
}

/*----------------------------------------------------------------------------
  \overload void QPainter::fillRect( const QRect &r, const QBrush &brush )
 ----------------------------------------------------------------------------*/

void QPainter::fillRect( const QRect &r, const QBrush &brush )
{
    fillRect( r.x(), r.y(), r.width(), r.height(), brush );
}

/*----------------------------------------------------------------------------
  Erases the area inside \e (x,y,w,h).
  Equivalent to <code>fillRect( x, y, w, h, backgroundColor() )</code>.
 ----------------------------------------------------------------------------*/

void QPainter::eraseRect( int x, int y, int w, int h )
{
    fillRect( x, y, w, h, backgroundColor() );
}

/*----------------------------------------------------------------------------
  \overload void QPainter::eraseRect( const QRect &r )
 ----------------------------------------------------------------------------*/

void QPainter::eraseRect( const QRect &r )
{
    fillRect( r.x(), r.y(), r.width(), r.height(), backgroundColor() );
}

/*----------------------------------------------------------------------------
  \overload void QPainter::drawText( const QPoint &p, const char *s, int len )
 ----------------------------------------------------------------------------*/

void QPainter::drawText( const QPoint &p, const char *s, int len )
{
    drawText( p.x(), p.y(), s, len );
}

/*----------------------------------------------------------------------------
  \overload void QPainter::drawText( const QRect &r, int tf, const char *str, int len, QRect *br, char **i )
 ----------------------------------------------------------------------------*/

void QPainter::drawText( const QRect &r, int tf, const char *str, int len,
			 QRect *br, char **i )
{
    drawText( r.x(), r.y(), r.width(), r.height(), tf, str, len, br, i );
}

/*----------------------------------------------------------------------------
  \overload QRect QPainter::boundingRect( const QRect &r, int tf,const char *str, int len, char **i )
 ----------------------------------------------------------------------------*/

QRect QPainter::boundingRect( const QRect &r, int tf,
			      const char *str, int len, char **i )
{
    return boundingRect( r.x(), r.y(), r.width(), r.height(), tf, str, len,
			 i );
}


/*****************************************************************************
  QPen member functions
 *****************************************************************************/

/*----------------------------------------------------------------------------
  \class QPen qpen.h
  \brief The QPen class defines how the QPainter should draw lines and outlines
  of shapes.
  \ingroup drawing

  A pen has a style, a width and a color.

  The pen style defines the line type. The default pen style is \c SolidPen.
  Setting the style to \c NoPen tells the painter to not draw lines or
  outlines.

  The pen width defines the line width. The default line width is 0,
  which draws a 1-pixel line very fast, but with lower presicion than
  with a line width of 1. Setting the line width to 1 or more draws
  lines that are precise, but drawing is slower.

  The pen color defines the color of lines and text. The default line
  color is black.  The QColor documentation lists predefined colors.

  Use the QBrush class for specifying fill styles.

  Example of how to use a pen:
  \code
    QPainter painter;
    QPen     pen( red, 2 );		// red solid line, 2 pixel width
    painter.begin( &anyPaintDevice );	// paint something
    painter.setPen( pen );		// set the red, fat pen
    painter.drawRect( 40,30, 200,100 ); // draw rectangle
    painter.setPen( blue );		// set blue pen, 0 pixel width
    painter.drawLine( 40,30, 240,130 ); // draw diagonal in rectangle
    painter.end();			// painting done
  \endcode

  The setStyle() function lists the pen styles.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \internal
  Initializes the pen.
 ----------------------------------------------------------------------------*/

void QPen::init( const QColor &color, uint width, PenStyle style )
{
    data = new QPenData;
    CHECK_PTR( data );
    data->style = style;
    data->width = width;
    data->color = color;
}

/*----------------------------------------------------------------------------
  Constructs a default black solid line pen with 0 width.
 ----------------------------------------------------------------------------*/

QPen::QPen()
{
    init( black, 0, SolidLine );		// default pen
}

/*----------------------------------------------------------------------------
  Constructs a	pen black with 0 width and a specified style.
  \sa setStyle()
 ----------------------------------------------------------------------------*/

QPen::QPen( PenStyle style )
{
    init( black, 0, style );
}

/*----------------------------------------------------------------------------
  Constructs a pen with a specified color, width and style.
  \sa setWidth(), setStyle(), setColor()
 ----------------------------------------------------------------------------*/

QPen::QPen( const QColor &color, uint width, PenStyle style )
{
    init( color, width, style );
}

/*----------------------------------------------------------------------------
  Constructs a pen which is a copy of \e p.
 ----------------------------------------------------------------------------*/

QPen::QPen( const QPen &p )
{
    data = p.data;
    data->ref();
}

/*----------------------------------------------------------------------------
  Destroys the pen.
 ----------------------------------------------------------------------------*/

QPen::~QPen()
{
    if ( data->deref() )
	delete data;
}


/*----------------------------------------------------------------------------
  Detaches from shared pen data to makes sure that this pen is the only
  one referring the data.

  If multiple pens share common data, this pen dereferences the data
  and gets a copy of the data. Nothing is done if there is just a
  single reference.
 ----------------------------------------------------------------------------*/

void QPen::detach()
{
    if ( data->count != 1 )
	*this = copy();
}


/*----------------------------------------------------------------------------
  Assigns \e c to this pen and returns a reference to this pen.
 ----------------------------------------------------------------------------*/

QPen &QPen::operator=( const QPen &p )
{
    p.data->ref();				// beware of p = p
    if ( data->deref() )
	delete data;
    data = p.data;
    return *this;
}


/*----------------------------------------------------------------------------
  Returns a deep copy of the pen.
 ----------------------------------------------------------------------------*/

QPen QPen::copy() const
{
    QPen p( data->color, data->width, data->style );
    return p;
}


/*----------------------------------------------------------------------------
  \fn PenStyle QPen::style() const
  Returns the pen style.
  \sa setStyle()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the pen style to \e s.

  The pen styles are:
  <dl compact>
  <dt> NoPen <dd> no outline is drawn.
  <dt> SolidLine <dd> solid line (default).
  <dt> DashLine <dd> - - - (dashes) line.
  <dt> DotLine <dd> * * * (dots) line.
  <dt> DashDotLine <dd> - * - * line.
  <dt> DashDotDotLine <dd> - ** - ** line.
  </dl>

  \sa style()
 ----------------------------------------------------------------------------*/

void QPen::setStyle( PenStyle s )
{
    if ( data->style == s )
	return;
    detach();
    data->style = s;
}


/*----------------------------------------------------------------------------
  \fn uint QPen::width() const
  Returns the pen width.
  \sa setWidth()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the pen width to \e w.
  \sa width()
 ----------------------------------------------------------------------------*/

void QPen::setWidth( uint w )
{
    if ( data->width == w )
	return;
    detach();
    data->width = w;
}


/*----------------------------------------------------------------------------
  \fn const QColor &QPen::color() const
  Returns the pen color.
  \sa setColor()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the pen color to \e c.
  \sa color()
 ----------------------------------------------------------------------------*/

void QPen::setColor( const QColor &c )
{
    detach();
    data->color = c;
}


/*----------------------------------------------------------------------------
  \fn bool QPen::operator!=( const QPen &p ) const
  Returns TRUE if the pen is different from \e p, or FALSE if the pens are
  equal.

  Two pens are different if they have different styles, widths or colors.

  \sa operator==()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Returns TRUE if the pen is equal to \e p, or FALSE if the pens are
  different.

  Two pens are equal if they have equal styles, widths and colors.

  \sa operator!=()
 ----------------------------------------------------------------------------*/

bool QPen::operator==( const QPen &p ) const
{
    return (p.data == data) || (p.data->style == data->style &&
	    p.data->width == data->width && p.data->color == data->color);
}


/*****************************************************************************
  QPen stream functions
 *****************************************************************************/

/*----------------------------------------------------------------------------
  \relates QPen
  Writes a pen to the stream and returns a reference to the stream.

  The serialization format is:
  <ol>
  <li> The pen style (UINT8)
  <li> The pen width (UINT8)
  <li> The pen color (QColor)
  </ol>
 ----------------------------------------------------------------------------*/

QDataStream &operator<<( QDataStream &s, const QPen &p )
{
    return s << (UINT8)p.style() << (UINT8)p.width() << p.color();
}

/*----------------------------------------------------------------------------
  \relates QPen
  Reads a pen from the stream and returns a reference to the stream.
 ----------------------------------------------------------------------------*/

QDataStream &operator>>( QDataStream &s, QPen &p )
{
    UINT8 style, width;
    QColor color;
    s >> style;
    s >> width;
    s >> color;
    p = QPen( color, (uint)width, (PenStyle)style );
    return s;
}


/*****************************************************************************
  QBrush member functions
 *****************************************************************************/

/*----------------------------------------------------------------------------
  \class QBrush qbrush.h

  \brief The QBrush class defines the fill pattern of shapes drawn using the
  QPainter.

  \ingroup drawing
  \ingroup shared

  A brush has a style and a color.  One of the brush styles is a custom
  pattern, which is defined by a QPixmap.

  The brush style defines the fill pattern. The default brush style is \c
  NoBrush (depends on how you construct a brush).  This style tells the
  painter to not fill shapes. The standard style for filling is called \c
  SolidPattern.

  The brush color defines the color of the fill pattern.
  The QColor documentation lists the predefined colors.

  Use the QPen class for specifying line/outline styles.

  Example of how to use a brush:
  \code
    QPainter painter;
    QBrush   brush( yellow );		// yellow solid pattern
    painter.begin( &anyPaintDevice );	// paint something
    painter.setBrush( brush );		// set the yellow brush
    painter.setPen( NoPen );		// do not draw outline
    painter.drawRect( 40,30, 200,100 ); // draw filled rectangle
    painter.setBrush( NoBrush );	// do not fill
    painter.setPen( black );		// set black pen, 0 pixel width
    painter.drawRect( 10,10, 30,20 );	// draw rectangle outline
    painter.end();			// painting done
  \endcode

  The setStyle() function lists the brush styles.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \internal
  Initializes the brush.
 ----------------------------------------------------------------------------*/

void QBrush::init( const QColor &color, BrushStyle style )
{
    data = new QBrushData;
    CHECK_PTR( data );
    data->style	 = style;
    data->color	 = color;
    data->pixmap = 0;
}

/*----------------------------------------------------------------------------
  Constructs a default black brush with the style \c NoBrush (will not fill
  shapes).
 ----------------------------------------------------------------------------*/

QBrush::QBrush()
{
    init( black, NoBrush );
}

/*----------------------------------------------------------------------------
  Constructs a black brush with the specified style.
  \sa setStyle()
 ----------------------------------------------------------------------------*/

QBrush::QBrush( BrushStyle style )
{
    init( black, style );
}

/*----------------------------------------------------------------------------
  Constructs a brush with a specified color and style.
  \sa setColor(), setStyle()
 ----------------------------------------------------------------------------*/

QBrush::QBrush( const QColor &color, BrushStyle style )
{
    init( color, style );
}

/*----------------------------------------------------------------------------
  Constructs a brush with a specified color and a custom pattern.

  The color will only have an effect for monochrome pixmaps, i.e.
  QPixmap::depth() == 1.

  \sa setColor(), setPixmap()
 ----------------------------------------------------------------------------*/

QBrush::QBrush( const QColor &color, const QPixmap &pixmap )
{
    init( color, CustomPattern );
    data->pixmap = new QPixmap( pixmap );
}

/*----------------------------------------------------------------------------
  Constructs a brush which is a shallow copy of \e b.
 ----------------------------------------------------------------------------*/

QBrush::QBrush( const QBrush &b )
{
    data = b.data;
    data->ref();
}

/*----------------------------------------------------------------------------
  Destroys the brush.
 ----------------------------------------------------------------------------*/

QBrush::~QBrush()
{
    if ( data->deref() ) {
	delete data->pixmap;
	delete data;
    }
}


/*----------------------------------------------------------------------------
  Detaches from shared brush data to makes sure that this brush is the only
  one referring the data.

  If multiple brushes share common data, this pen dereferences the data
  and gets a copy of the data. Nothing is done if there is just a single
  reference.
 ----------------------------------------------------------------------------*/

void QBrush::detach()
{
    if ( data->count != 1 )
	*this = copy();
}


/*----------------------------------------------------------------------------
  Assigns \e b to this brush and returns a reference to this brush.
 ----------------------------------------------------------------------------*/

QBrush &QBrush::operator=( const QBrush &b )
{
    b.data->ref();				// beware of b = b
    if ( data->deref() ) {
	delete data->pixmap;
	delete data;
    }
    data = b.data;
    return *this;
}


/*----------------------------------------------------------------------------
  Returns a deep copy of the brush.
 ----------------------------------------------------------------------------*/

QBrush QBrush::copy() const
{
    if ( data->style == CustomPattern ) {     // brush has pixmap
	QBrush b( data->color, *data->pixmap );
	return b;
    }
    else {				      // brush has std pattern
	QBrush b( data->color, data->style );
	return b;
    }
}


/*----------------------------------------------------------------------------
  \fn BrushStyle QBrush::style() const
  Returns the brush style.
  \sa setStyle()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the brush style to \e s.

  The brush styles are:
  <dl compact>
  <dt> NoBrush <dd> will not fill shapes (default).
  <dt> SolidPattern <dd> solid (100%) fill pattern.
  <dt> Dense1Pattern <dd> 94% fill pattern.
  <dt> Dense2Pattern <dd> 88% fill pattern.
  <dt> Dense3Pattern <dd> 63% fill pattern.
  <dt> Dense4Pattern <dd> 50% fill pattern.
  <dt> Dense5Pattern <dd> 37% fill pattern.
  <dt> Dense6Pattern <dd> 12% fill pattern.
  <dt> Dense7Pattern <dd> 6% fill pattern.
  <dt> HorPattern <dd> horizontal lines pattern.
  <dt> VerPattern <dd> vertical lines pattern.
  <dt> CrossPattern <dd> crossing lines pattern.
  <dt> BDiagPattern <dd> diagonal lines (directed / ) pattern.
  <dt> FDiagPattern <dd> diagonal lines (directed \ ) pattern.
  <dt> DiagCrossPattern <dd> diagonal crossing lines pattern.
  <dt> CustomPattern <dd> set when a pixmap pattern is being used.
  </dl>

  \sa style()
 ----------------------------------------------------------------------------*/

void QBrush::setStyle( BrushStyle s )		// set brush style
{
    if ( data->style == s )
	return;
#if defined(CHECK_RANGE)
    if ( s == CustomPattern )
	warning( "QBrush::setStyle: CustomPattern is for internal use" );
#endif
    detach();
    data->style = s;
}


/*----------------------------------------------------------------------------
  \fn const QColor &QBrush::color() const
  Returns the brush color.
  \sa setColor()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the brush color to \e c.
  \sa color(), setStyle()
 ----------------------------------------------------------------------------*/

void QBrush::setColor( const QColor &c )
{
    detach();
    data->color = c;
}


/*----------------------------------------------------------------------------
  \fn QPixmap *QBrush::pixmap() const
  Returns a pointer to the custom brush pattern.

  A null pointer is returned if no custom brush pattern has been set.

  \sa setPixmap()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the brush pixmap.  The style is set to \c CustomPattern.

  The curren brush color will only have an effect for monochrome pixmaps,
  i.e.	QPixmap::depth() == 1.

  \sa pixmap(), color()
 ----------------------------------------------------------------------------*/

void QBrush::setPixmap( const QPixmap &pixmap )
{
    detach();
    data->style = CustomPattern;
    if ( data->pixmap )
	delete data->pixmap;
    data->pixmap = new QPixmap( pixmap );
}


/*----------------------------------------------------------------------------
  \fn bool QBrush::operator!=( const QBrush &b ) const
  Returns TRUE if the brush is different from \e b, or FALSE if the brushes are
  equal.

  Two brushes are different if they have different styles, colors or pixmaps.

  \sa operator==()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Returns TRUE if the brush is equal to \e b, or FALSE if the brushes are
  different.

  Two brushes are equal if they have equal styles, colors and pixmaps.

  \sa operator!=()
 ----------------------------------------------------------------------------*/

bool QBrush::operator==( const QBrush &b ) const
{
    return (b.data == data) || (b.data->style == data->style &&
	    b.data->color  == data->color &&
	    b.data->pixmap == data->pixmap);
}


/*****************************************************************************
  QBrush stream functions
 *****************************************************************************/

/*----------------------------------------------------------------------------
  \relates QBrush
  Writes a brush to the stream and returns a reference to the stream.

  The serialization format is:
  <ol>
  <li> The brush style (UINT8)
  <li> The brush color (QColor)
  <li> If style == \c CustomPattern: the brush pixmap (QPixmap)
  </ol>
 ----------------------------------------------------------------------------*/

QDataStream &operator<<( QDataStream &s, const QBrush &b )
{
    s << (UINT8)b.style() << b.color();
    if ( b.style() == CustomPattern )
	s << *b.pixmap();
    return s;
}

/*----------------------------------------------------------------------------
  \relates QBrush
  Reads a brush from the stream and returns a reference to the stream.
 ----------------------------------------------------------------------------*/

QDataStream &operator>>( QDataStream &s, QBrush &b )
{
    UINT8 style;
    QColor color;
    s >> style;
    s >> color;
    if ( style == CustomPattern ) {
	QPixmap pm;
	s >> pm;
	b = QBrush( color, pm );
    }
    else
	b = QBrush( color, (BrushStyle)style );
    return s;
}

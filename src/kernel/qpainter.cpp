/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpainter.cpp#198 $
**
** Implementation of QPainter, QPen and QBrush classes
**
** Created : 940112
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qpainter.h"
#include "qbitmap.h"
#include "qstack.h"
#include "qdatastream.h"
#include "qwidget.h"
#include "qimage.h"
#include "q1xcompatibility.h"
#include "qpaintdevicemetrics.h"
#include <stdlib.h>

typedef QStack<QWMatrix> QWMatrixStack;

// NOT REVISED
/*!
  \class QPainter qpainter.h
  \brief The QPainter class paints on paint devices.

  \ingroup drawing

  The painter provides efficient graphics rendering on any QPaintDevice
  object. QPainter can draw everything from simple lines to complex shapes
  like pies and chords. It can also draw aligned text and pixmaps.

  Graphics can be transformed using view transformation, world
  transformation or a combination of these two.	 View transformation
  is a window/viewport transformation with translation and
  scaling. World transformation is a full 2D transformation including
  rotation and shearing.

  The typical use of a painter is:
  <ol>
  <li> Construct a painter.
  <li> Set a pen, a brush etc.
  <li> Draw.
  <li> Destroy the painter.
  </ol>

  This example uses a convenience constructor that calls begin(), and
  relies on the destructor to call end():

  \code
    void MyWidget::paintEvent()
    {
	QPainter paint( this );			// start painting widget
	paint.setPen( Qt::blue );		// set blue pen
	paint.drawText( rect(),			// draw a text, centered
			AlignCenter,		//   in the widget
			"The Text" );
    }
  \endcode

  You can also use the begin() and end() functions to begin and end
  painting explicitly:

  \code
    void MyWidget::paintEvent()
    {
	QPainter paint;
	paint.begin( this );			// start painting widget
	paint.setPen( Qt::blue );		// set blue pen
	paint.drawText( rect(),			// draw a text, centered
			AlignCenter,		//   in the widget
			"The Text" );
	paint.end();				// painting done
    }
  \endcode

  This is useful since it is not possible to have two painters active
  on the same paint device at a time.

  QPainter is almost never used outside \link QWidget::paintEvent()
  paintEvent()\endlink.  Any widget <em>must</em> be able to repaint
  itself at any time via paintEvent(), therefore it's almost always
  best to design the widget so that it does all the painting in
  paintEvent() and use either QWidget::update() or QWidget::repaint()
  force a paint event as necessary.

  Note that both painters and some paint devices have attributes such
  as current font, current foreground colors and so on.

  QPainter::begin() copies these attributes from the paint device, and
  changing a paint device's attributes will have effect only the next
  time a painter is opened on it.

  \warning The range of acceptable coordinate values to QPainter's
  various drawing functions is limited by the capabilities of the
  drawing engine of the underlying window system. Currently, only
  Windows NT is able to handle the full 32 bit range; on other
  platforms, the output may be incorrect when the absolute value of
  any coordinate (after any set world and/or view transforms have been
  applied) exceeds some system-dependent value. All systems we have
  tested were able to correctly handle coordinates up to +/- 2000.

  \warning QPainter::begin() resets all attributes to their default
  values, from the device, thus setting fonts, brushes, etc, before
  begin() will have \e no effect.

  \header qdrawutil.h

  \sa QPaintDevice, QWidget, QPixmap
*/

/*! \enum Qt::AlignmentFlags

  This enum type is used to describe alignment.  It contains four sets
  of flags: Horizontal, vertical and modifying flags.  The horizontal
  flags are: <ul>

  <li> \c AlignLeft - Align with the left edge.
  <li> \c AlignRight - Align with the left edge.
  <li> \c AlignHCenter - Center horizontally in the available space.

  </ul> The vertical flags are: <ul>

  <li> \c AlignTop - Align with the top.
  <li> \c AlignBottom - Align with the bottom.
  <li> \c AlignVCenter - Center vertically in the available space.

  </ul> You can only use one of the horizontal flags at a time.  There
  is one two-dimensional flag: <ul>

  <li> \c AlignCenter - Center in both dimensions.

  </ul> This counts both as a horizontal and vertical flag: It cannot
  be combined with any other horizontal or vertical flags.

  There are also some modifier flags.  All of them apply only to
  printing: <ul>

  <li> \c SingleLine - Treat all white-space as space and print just
  one line.

  <li> \c DontClip - If it's impossible to stay within the given
  bounds, print outside.

  <li> \c ExpandTabs - Make the U+0009 (ascii tab) character move to
  the next tab stop.

  <li> \c ShowPrefix - Display the string "\&P" as an underlined P
  (see QButton for an example).  To get an ampersand, use "\&\&".

  <li> \c WordBreak - Do line breaking at at appropriate points.

  </ul>

  You can only use one of the horizontal flags at a time, and one of
  the vertical flags.  \c AlignCenter counts as both horizontal and
  vertical.  You can use as many modifier flags as you want, except
  that \c SingleLine and \c WordBreak cannot be combined.

  Flags that are inappropriate for a given use (e.g. ShowPrefix to
  QGridLayout::addWidget()) are ignored.

  Conflicting combinations of flags have undefined meanings.
*/


/*! \enum Qt::PenStyle

  This enum type defines the pen styles supported by Qt; ie. that
  sorts of lines that can be drawn using QPainter.  The current styles
  are: <ul>

  <li> \c NoPen - no line at all.  For example, QPainter::drawRect()
  fills but does not draw any explicit boundary line.

  <li> \c SolidLine - a simple line.

  <li> \c DashLine - dashes, separated by a few pixels.

  <li> \c DotLine - dots, separated by a few pixels.

  <li> \c DashDotLine - alternately dots and dashes.

  <li> \c DashDotDotLine - one dash, two dots, one dash, two dots...

  </ul>
*/


/*!
  Constructs a painter.

  Notice that all painter settings (setPen,setBrush etc.) are reset to
  default values when begin() is called.

  \sa begin(), end()
*/

QPainter::QPainter()
{
    init();
}


/*!
  Constructs a painter that begins painting the paint device \a pd
  immediately.

  This constructor is convenient for short-lived painters, e.g. in
  a \link QWidget::paintEvent() paint event\endlink and should be
  used only once. The constructor calls begin() for you and the QPainter
  destructor automatically calls end().

  Example using begin() and end():
  \code
    void MyWidget::paintEvent( QPaintEvent * )
    {
	QPainter p( this );
	p.drawLine( ... );	// drawing code
    }
  \endcode

  Example using this constructor:
  \code
    void MyWidget::paintEvent( QPaintEvent * )
    {
	QPainter p( this );
	p.drawLine( ... );	// drawing code
    }
  \endcode

  \sa begin(), end()
*/

QPainter::QPainter( const QPaintDevice *pd )
{
    init();
    begin( pd );
    flags |= CtorBegin;
}


/*!
  Constructs a painter that begins painting the paint device \a pd
  immediately, with the default arguments taken from \a copyAttributes.

  \sa begin()
*/

QPainter::QPainter( const QPaintDevice *pd,
		    const QWidget *copyAttributes )
{
    init();
    begin( pd, copyAttributes );
    flags |= CtorBegin;
}


/*!
  Destroys the painter.
*/

QPainter::~QPainter()
{
    if ( isActive() )
	end();
    if ( tabarray )				// delete tab array
	delete [] tabarray;
    if ( ps_stack )
	killPStack();
    if (wm_stack )
	delete (QWMatrixStack *)wm_stack;
}


/*!
  \overload bool QPainter::begin( const QPaintDevice *pd, const QWidget *copyAttributes )

  This version opens the painter on a paint device \a pd and sets the initial
  pen, background color and font from \a copyAttributes.  This is equivalent
  with:
  \code
    QPainter p;
    p.begin( pd );
    p.setPen( copyAttributes->foregroundColor() );
    p.setBackgroundColor( copyAttributes->backgroundColor() );
    p.setFont( copyAttributes->font() );
  \endcode

  This begin function is convenient for double buffering.  When you
  draw in a pixmap instead of directly in a widget (to later bitBlt
  the pixmap into the widget) you will need to set the widgets's
  font etc.  This function does exactly that.

  Example:
  \code
    void MyWidget::paintEvent( QPaintEvent * )
    {
	QPixmap pm(size());
	QPainter p;
	p.begin(&pm, this);
	// ... potential flickering paint operation ...
	p.end();
	bitBlt(this, 0, 0, &pm);
    }
  \endcode

  \sa end()
*/

bool QPainter::begin( const QPaintDevice *pd, const QWidget *copyAttributes )
{
    if ( pd == 0 ) {
#if defined(CHECK_NULL)
	qWarning( "QPainter::begin: The widget to copy attributes from cannot "
		 "be null" );
#endif
	return FALSE;
    }
    if ( begin(pd) ) {
	setPen( copyAttributes->foregroundColor() );
	setBackgroundColor( copyAttributes->backgroundColor() );
	setFont( copyAttributes->font() );
	return TRUE;
    }
    return FALSE;
}


/*!
  \internal
  Sets or clears a pointer flag.
*/

void QPainter::setf( uint b, bool v )
{
    if ( v )
	setf( b );
    else
	clearf( b );
}


/*!
  \fn bool QPainter::isActive() const

  Returns TRUE if the painter is active painting, i.e. begin() has
  been called and end() has not yet been called.

  \sa QPaintDevice::paintingActive()
*/

/*!
  \fn QPaintDevice *QPainter::device() const

  Returns the paint device on which this painter is currently
  painting, or null if the painter is not active.

  \sa QPaintDevice::paintingActive()
*/


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
    void* wm_stack;
};

//TODO Matthias store worldmatrix stack in QPState!

typedef QStack<QPState> QPStateStack;


void QPainter::killPStack()
{
    delete (QPStateStack *)ps_stack;
    ps_stack = 0;
}

/*!
  Saves the current painter state (pushes the state onto a stack).

  A save() must have a corresponding restore().

  \sa restore()
*/

void QPainter::save()
{
    if ( testf(ExtDev) ) {
	if ( testf(DirtyFont) )
	    updateFont();
	if ( testf(DirtyPen) )
	    updatePen();
	if ( testf(DirtyBrush) )
	    updateBrush();
	pdev->cmd( QPaintDevice::PdcSave, this, 0 );
    }
    QPStateStack *pss = (QPStateStack *)ps_stack;
    if ( pss == 0 ) {
	pss = new QStack<QPState>;
	CHECK_PTR( pss );
	pss->setAutoDelete( TRUE );
	ps_stack = pss;
    }
    QPState *ps = new QPState;
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
    ps->rgn   = crgn;
    ps->clip  = testf(ClipOn);
    ps->ts    = tabstops;
    ps->ta    = tabarray;
    ps->wm_stack = wm_stack;
    wm_stack = 0;
    pss->push( ps );
}

/*!
  Restores the current painter state (pops a saved state off the stack).
  \sa save()
*/

void QPainter::restore()
{
    if ( testf(ExtDev) ) {
	pdev->cmd( QPaintDevice::PdcRestore, this, 0 );
    }
    QPStateStack *pss = (QPStateStack *)ps_stack;
    if ( pss == 0 || pss->isEmpty() ) {
#if defined(CHECK_STATE)
	qWarning( "QPainter::restore: Empty stack error" );
#endif
	return;
    }
    QPState *ps = pss->pop();
    bool hardRestore = testf(VolatileDC);

    if ( ps->font != cfont || hardRestore )
	setFont( ps->font );
    if ( ps->pen != cpen || hardRestore )
	setPen( ps->pen );
    if ( ps->brush != cbrush || hardRestore )
	setBrush( ps->brush );
    if ( ps->bgc != bg_col || hardRestore )
	setBackgroundColor( ps->bgc );
    if ( ps->bgm != bg_mode || hardRestore )
	setBackgroundMode( (BGMode)ps->bgm );
    if ( ps->rop != rop || hardRestore )
	setRasterOp( (RasterOp)ps->rop );
#if 0
    if ( ps->pu != pu )				// !!!not used
	pu = ps->pu;
#endif
    QRect wr( wx, wy, ww, wh );
    QRect vr( vx, vy, vw, vh );
    if ( ps->wr != wr || hardRestore )
	setWindow( ps->wr );
    if ( ps->vr != vr || hardRestore )
	setViewport( ps->vr );
    if ( ps->wm != wxmat || hardRestore )
	setWorldMatrix( ps->wm );
    if ( ps->vxf != testf(VxF) || hardRestore )
	setViewXForm( ps->vxf );
    if ( ps->wxf != testf(WxF) || hardRestore )
	setWorldXForm( ps->wxf );
    if ( ps->rgn != crgn || hardRestore )
	setClipRegion( ps->rgn );
    if ( ps->clip != testf(ClipOn) || hardRestore )
	setClipping( ps->clip );
    tabstops = ps->ts;
    tabarray = ps->ta;

    if (wm_stack )
	delete (QWMatrixStack *)wm_stack;
    wm_stack = ps->wm_stack;
    delete ps;
}


/*!
  Returns the font metrics for the painter.
  Font metrics can only be obtained when the painter is active.
  \sa fontInfo(), isActive()
*/

QFontMetrics QPainter::fontMetrics() const
{
    if ( pdev && pdev->devType() == QInternal::Picture )
	return QFontMetrics( cfont );

    return QFontMetrics(this);
}

/*!
  Returns the font info for the painter.
  Font info can only be obtained when the painter is active.
  \sa fontMetrics(), isActive()
*/

QFontInfo QPainter::fontInfo() const
{
    if ( pdev && pdev->devType() == QInternal::Picture )
	return QFontInfo( cfont );

    return QFontInfo(this);
}


/*!
  \fn const QPen &QPainter::pen() const
  Returns the current pen for the painter.
  \sa setPen()
*/

/*!
  Sets a new painter pen.

  The pen defines how to draw lines and outlines, and it also defines
  the text color.

  \sa pen()
*/

void QPainter::setPen( const QPen &pen )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	qWarning( "QPainter::setPen: Will be reset by begin()" );
#endif
    cpen = pen;
    updatePen();
}

/*!
  Sets a new painter pen with style \c style, width 0 and black color.
  \sa pen(), QPen
*/

void QPainter::setPen( PenStyle style )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	qWarning( "QPainter::setPen: Will be reset by begin()" );
#endif
    QPen::QPenData *d = cpen.data;	// low level access
    if ( d->count != 1 ) {
	cpen.detach();
	d = cpen.data;
    }
    d->style = style;
    d->width = 0;
    d->color = Qt::black;
    updatePen();
}

/*!
  Sets a new painter pen with style \c SolidLine, width 0 and the specified
  \e color.
  \sa pen(), QPen
*/

void QPainter::setPen( const QColor &color )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	qWarning( "QPainter::setPen: Will be reset by begin()" );
#endif
    QPen::QPenData *d = cpen.data;	// low level access
    if ( d->count != 1 ) {
	cpen.detach();
	d = cpen.data;
    }
    d->style = SolidLine;
    d->width = 0;
    d->color = color;
    updatePen();
}

/*!
  \fn const QBrush &QPainter::brush() const
  Returns the current painter brush.
  \sa QPainter::setBrush()
*/

/*!
  Sets a new painter brush.

  The brush defines how to fill shapes.

  \sa brush()
*/

void QPainter::setBrush( const QBrush &brush )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	qWarning( "QPainter::setBrush: Will be reset by begin()" );
#endif
    cbrush = brush;
    updateBrush();
}

/*!
  Sets a new painter brush with black color and the specified \e style.
  \sa brush(), QBrush
*/

void QPainter::setBrush( BrushStyle style )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	qWarning( "QPainter::setBrush: Will be reset by begin()" );
#endif
    QBrush::QBrushData *d = cbrush.data; // low level access
    if ( d->count != 1 ) {
	cbrush.detach();
	d = cbrush.data;
    }
    d->style = style;
    d->color = Qt::black;
    if ( d->pixmap ) {
	delete d->pixmap;
	d->pixmap = 0;
    }
    updateBrush();
}

/*!
  Sets a new painter brush with the style \c SolidPattern and the specified
  \e color.
  \sa brush(), QBrush
*/

void QPainter::setBrush( const QColor &color )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	qWarning( "QPainter::setBrush: Will be reset by begin()" );
#endif
    QBrush::QBrushData *d = cbrush.data; // low level access
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


/*!
  \fn const QColor &QPainter::backgroundColor() const
  Returns the background color currently set.
  \sa setBackgroundColor()
*/

/*!
  \fn BGMode QPainter::backgroundMode() const
  Returns the background mode currently set.
  \sa setBackgroundMode()
*/

/*!
  \fn RasterOp QPainter::rasterOp() const
  Returns the raster operation currently set.
  \sa setRasterOp()
*/

/*!
  \fn const QPoint &QPainter::brushOrigin() const
  Returns the brush origin currently set.
  \sa setBrushOrigin()
*/


/*!
  \fn int QPainter::tabStops() const
  Returns the tab stop setting.
  \sa setTabStops()
*/

/*!
  Set the number of pixels per tab stop to a fixed number.

  Tab stops are used when drawing formatted text with \c ExpandTabs set.
  This fixed tab stop value has lower precedence than tab array
  settings.

  \sa tabStops(), setTabArray(), drawText(), fontMetrics()
*/

void QPainter::setTabStops( int ts )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	qWarning( "QPainter::setTabStops: Will be reset by begin()" );
#endif
    tabstops = ts;
    if ( isActive() && testf(ExtDev) ) {	// tell extended device
	QPDevCmdParam param[1];
	param[0].ival = ts;
	pdev->cmd( QPaintDevice::PdcSetTabStops, this, param );
    }
}

/*!
  \fn int *QPainter::tabArray() const
  Returns the tab stop array currently set.
  \sa setTabArray()
*/

/*!
  Set an array containing the tab stops.

  Tab stops are used when drawing formatted text with \c ExpandTabs set.

  The last tab stop must be 0 (terminates the array).

  Notice that setting a tab array overrides any fixed tabulator stop
  that is set using setTabStops().

  \sa tabArray(), setTabStops(), drawText(), fontMetrics()
*/

void QPainter::setTabArray( int *ta )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	qWarning( "QPainter::setTabArray: Will be reset by begin()" );
#endif
    if ( ta != tabarray ) {
	tabarraylen = 0;
	if ( tabarray )				// Avoid purify complaint
	    delete [] tabarray;			// delete old array
	if ( ta ) {				// tabarray = copy of 'ta'
	    while ( ta[tabarraylen] )
		tabarraylen++;
	    tabarraylen++; // and 0 terminator
	    tabarray = new int[tabarraylen];	// duplicate ta
	    memcpy( tabarray, ta, sizeof(int)*tabarraylen );
	} else {
	    tabarray = 0;
	}
    }
    if ( isActive() && testf(ExtDev) ) {	// tell extended device
	QPDevCmdParam param[2];
	param[0].ival = tabarraylen;
	param[1].ivec = tabarray;
	pdev->cmd( QPaintDevice::PdcSetTabArray, this, param );
    }
}


/*!
  \fn HANDLE QPainter::handle() const
  Returns the platform-dependent handle used for drawing.
*/


/*****************************************************************************
  QPainter xform settings
 *****************************************************************************/

/*!
  Enables view transformations if \e enable is TRUE, or disables view
  transformations if \e enable is FALSE.
  \sa hasViewXForm(), setWindow(), setViewport(), setWorldMatrix(),
  setWorldXForm(), xForm()
*/

void QPainter::setViewXForm( bool enable )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	qWarning( "QPainter::setViewXForm: Will be reset by begin()" );
#endif
    if ( !isActive() || enable == testf(VxF) )
	return;
    setf( VxF, enable );
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].ival = enable;
	pdev->cmd( QPaintDevice::PdcSetVXform, this, param );
    }
    updateXForm();
}

/*!
  \fn bool QPainter::hasViewXForm() const
  Returns TRUE if view transformation is enabled, otherwise FALSE.
  \sa setViewXForm(), xForm()
*/

/*!
  Returns the window rectangle.
  \sa setWindow(), setViewXForm()
*/

QRect QPainter::window() const
{
    return QRect( wx, wy, ww, wh );
}

/*!
  Sets the window rectangle view transformation for the painter and
  enables view transformation.

  The window rectangle is part of the view transformation.  The window
  specifies the logical coordinate system.

  The window and the \link setViewport() viewport\endlink are initially set
  to \e (0,0,width,height), where \e (width,height) is the pixel size of the
  paint device.

  You can use this method to normalize the coordinate system of the
  painter. The following example will draw a vertical line, from top to
  bottom, at the center of a pixmap, independent of the size of the pixmap:

  \code
      int width, height;
      ...
      QPixmap icon( width, height );
      QPainter p( icon );
      p.setWindow( 0, 0, 100, 100 );
      p.drawLine( 50, 0, 50, 100 );		// draw center line
  \endcode

  The setWindow() method is often used in conjunction with
  setViewport(), as in this example:

  \code
      QPainter p( myWidget );
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
*/

void QPainter::setWindow( int x, int y, int w, int h )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	qWarning( "QPainter::setWindow: Will be reset by begin()" );
#endif
    wx = x;
    wy = y;
    ww = w;
    wh = h;
    if ( testf(ExtDev) ) {
	QRect r( x, y, w, h );
	QPDevCmdParam param[1];
	param[0].rect = (QRect*)&r;
	pdev->cmd( QPaintDevice::PdcSetWindow, this, param );
    }
    if ( testf(VxF) )
	updateXForm();
    else
	setViewXForm( TRUE );
}

/*!
  Returns the viewport rectangle.
  \sa setViewport(), setViewXForm()
*/

QRect QPainter::viewport() const		// get viewport
{
    return QRect( vx, vy, vw, vh );
}

/*!
  Sets the viewport rectangle view transformation for the painter and
  enables view transformation.

  The viewport rectangle is part of the view transformation. The viewport
  specifies the device coordinate system.

  The viewport and the \link setWindow() window\endlink are initially set
  to \e (0,0,width,height), where \e (width,height) is the pixel size of
  the paint device.

  You can use this method to normalize the coordinate system of the
  painter when drawing on a part of a paint device. The following example
  will draw a line from the top left to the bottom right corner of a page,
  excluding margins:

  \code
      QPrinter page;
      int margin, pageWidth, pageHeight;
      ...
      QPainter p( page );
      p.setViewPort( margin, margin, pageWidth - margin, pageHeight - margin );
      p.drawLine( 0, 0, pageWidth - 2*margin, pageHeight - 2*margin );
  \endcode

  The setViewPort() method is often used in conjunction with
  setWindow(), as in this example:

  \code
      QPainter p( myWidget );
      p.setWindow( 0, 0, 1000, 2000 );
      p.setViewport( 100,100, 200,200 );
      p.drawPoint( 500, 500 );			// draws pixel at (150,125)
  \endcode

  The preceding example sets up a transformation that maps the logical
  coordinates (0,0,1000,2000) into a (200,200) rectangle at (100,100).

  View transformations can be combined with world transformations.
  World transformations are applied after the view transformations.

  \sa viewport(), setWindow(), setViewXForm(), setWorldMatrix(),
  setWorldXForm(), xForm()
*/

void QPainter::setViewport( int x, int y, int w, int h )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	qWarning( "QPainter::setViewport: Will be reset by begin()" );
#endif
    vx = x;
    vy = y;
    vw = w;
    vh = h;
    if ( testf(ExtDev) ) {
	QRect r( x, y, w, h );
	QPDevCmdParam param[1];
	param[0].rect = (QRect*)&r;
	pdev->cmd( QPaintDevice::PdcSetViewport, this, param );
    }
    if ( testf(VxF) )
	updateXForm();
    else
	setViewXForm( TRUE );
}

/*!
  Enables world transformations if \e enable is TRUE, or disables
  world transformations if \e enable is FALSE.

  \sa setWorldMatrix(), setWindow(), setViewport(), setViewXForm(), xForm()
*/

void QPainter::setWorldXForm( bool enable )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	qWarning( "QPainter::setWorldXForm: Will be reset by begin()" );
#endif
    if ( !isActive() || enable == testf(WxF) )
	return;
    setf( WxF, enable );
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[1];
	param[0].ival = enable;
	pdev->cmd( QPaintDevice::PdcSetWXform, this, param );
    }
    updateXForm();
}

/*!
  \fn bool QPainter::hasWorldXForm() const
  Returns TRUE if world transformation is enabled, otherwise FALSE.
  \sa setWorldXForm()
*/

/*!
  Returns the world transformation matrix.
  \sa setWorldMatrix()
*/

const QWMatrix &QPainter::worldMatrix() const
{
    return wxmat;
}

/*!
  Sets the world transformation matrix to \e m and enables world
  transformation.

  If \e combine is TRUE, then \e m is combined with the current
  transformation matrix, otherwise \e m will replace the current
  transformation matrix.

  World transformations are applied after the view transformations
  (i.e. \link setWindow window\endlink and \link setViewport viewport\endlink).

  If the matrix set is the identity matrix (\link QWMatrix::m11()
  m11\endlink and \link QWMatrix::m22() m22\endlink are 1.0 and the
  rest are 0.0), this function calls setWorldXForm(FALSE).

  The following functions can transform the coordinate system without using
  a QWMatrix:
  <ul>
  <li>translate()
  <li>scale()
  <li>shear()
  <li>rotate()
  </ul>

  They operate on the painter's \link worldMatrix() world matrix\endlink
  and are implemented like this:

  \code
    void QPainter::rotate( double a )
    {
	QWMatrix m;
	m.rotate( a );
	setWorldMatrix( m, TRUE );
    }
  \endcode

  Note that you should always use combine when you are drawing into a
  QPicture. Otherwise the picture may not be completely encapsulated
  and cannot be replayed with additional transformations. Using the
  translate(), scale(), etc. functions is safe.

  Furthermore, you can easily save and restore the current world
  transformation matrix with the convenient functions
  saveWorldMatrix() and restoreWorldMatrix(), respectively. If you
  need to draw some top-to-bottom text at the position (x y), for
  instance, but everything else shall remain unrotated, you can easily
  achieve this with:

  \code
    void MyWidget::paintEvent()
    {
	QPainter paint( this );
	...
	paint.saveWorldMatrix();
	paint.translate( x, y );
	paint.rotate( 90 );
	paint.drawText( 0, 0, "top-to-bottom text" );
	paint.restoreWorldMatrix();
	....
    }
  \endcode

  See the \link QWMatrix QWMatrix documentation\endlink for a general
  discussion on coordinate system transformations.

  \sa worldMatrix(), setWorldXForm(), setWindow(), setViewport(),
  setViewXForm(), xForm()
*/

void QPainter::setWorldMatrix( const QWMatrix &m, bool combine )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
	qWarning( "QPainter::setWorldMatrix: Will be reset by begin()" );
#endif
    if ( combine )
	wxmat = m * wxmat;			// combines
    else
	wxmat = m;				// set new matrix
    bool identity = wxmat.m11() == 1.0F && wxmat.m22() == 1.0F &&
		    wxmat.m12() == 0.0F && wxmat.m21() == 0.0F &&
		    wxmat.dx()	== 0.0F && wxmat.dy()  == 0.0F;
    if ( testf(ExtDev) ) {
	QPDevCmdParam param[2];
	param[0].matrix = &m;
	param[1].ival = combine;
	pdev->cmd( QPaintDevice::PdcSetWMatrix, this, param );
    }
    if ( identity )
	setWorldXForm( FALSE );
    else if ( !testf(WxF) )
	setWorldXForm( TRUE );
    else
	updateXForm();
}

/*!
  Saves the current world matrix (pushes the matrix onto a stack).

  In sane code a save() has a corresponding restoreWorldMatrix().

  \sa restoreWorldMatrix()
*/

void QPainter::saveWorldMatrix()
{
    QWMatrixStack *stack = (QWMatrixStack *)wm_stack;
    if ( stack == 0 ) {
	stack  = new QStack<QWMatrix>;
	CHECK_PTR( stack );
	stack->setAutoDelete( TRUE );
	wm_stack = stack;
    }

    stack->push( new QWMatrix( wxmat ) );

}

/*!
  Restores the current world matrix (pops a saved matrix off the stack).
  \sa saveWorldMatrix()
*/

void QPainter::restoreWorldMatrix()
{
    QWMatrixStack *stack = (QWMatrixStack *)wm_stack;
    if ( stack == 0 || stack->isEmpty() ) {
#if defined(CHECK_STATE)
	qWarning( "QPainter::restoreWorldMatrix: Empty stack error" );
#endif
	return;
    }
    QWMatrix* m = stack->pop();
    setWorldMatrix( *m );
    delete m;
}


/*!
  Translates the coordinate system by \e (dx,dy).

  For example, the following code draws a single vertical line 20 pixels high.
  \code
    void MyWidget::paintEvent()
    {
	QPainter paint( this );
	paint.drawLine(10,0,10,20);
	paint.translate(100.0,100.0);
	paint.drawLine(-90,-80,-90,-70);
    }
  \endcode

  \sa scale(), shear(), rotate(), resetXForm(), setWorldMatrix(), xForm()
*/

void QPainter::translate( double dx, double dy )
{
    QWMatrix m;
    m.translate( dx, dy );
    setWorldMatrix( m, TRUE );
}

/*!
  Scales the coordinate system by \e (sx,sy).
  \sa translate(), shear(), rotate(), resetXForm(), setWorldMatrix(),
  xForm()
*/

void QPainter::scale( double sx, double sy )
{
    QWMatrix m;
    m.scale( sx, sy );
    setWorldMatrix( m, TRUE );
}

/*!
  Shears the coordinate system \e (sh,sv).
  \sa translate(), scale(), rotate(), resetXForm(), setWorldMatrix(),
  xForm()
*/

void QPainter::shear( double sh, double sv )
{
    QWMatrix m;
    m.shear( sv, sh );
    setWorldMatrix( m, TRUE );
}

/*!
  Rotates the coordinate system \e a degrees.
  \sa translate(), scale(), shear(), resetXForm(), setWorldMatrix(),
  xForm()
*/

void QPainter::rotate( double a )
{
    QWMatrix m;
    m.rotate( a );
    setWorldMatrix( m, TRUE );
}

/*!
  Resets any transformations that were made using translate(), scale(),
  shear(), rotate(), setWorldMatrix(), setViewport() and setWindow()
  \sa worldMatrix(), viewPort(), window()
*/

void QPainter::resetXForm()
{
    if ( !isActive() )
	return;
    wx = wy = vx = vy = 0;			// default view origins
    ww = vw = pdev->metric( QPaintDeviceMetrics::PdmWidth );
    wh = vh = pdev->metric( QPaintDeviceMetrics::PdmHeight );
    wxmat = QWMatrix();
    setWorldXForm( FALSE );
}


const int TxNone      = 0;			// transformation codes
const int TxTranslate = 1;			// copy in qptr_xyz.cpp
const int TxScale     = 2;
const int TxRotShear  = 3;


/*!
  \internal
  Updates an internal integer transformation matrix.
*/

void QPainter::updateXForm()
{
    QWMatrix m;
    if ( testf(VxF) ) {
	m.translate( vx, vy );
	m.scale( 1.0*vw/ww, 1.0*vh/wh );
	m.translate( -wx, -wy );
    }
    if ( testf(WxF) ) {
	if ( testf(VxF) )
	    m = wxmat * m;
	else
	    m = wxmat;
    }
    xmat = m;

    txinv = FALSE;				// no inverted matrix
    txop  = TxNone;
    const double eps = 0.0; // ##### can we get away with this?
    //#define FZ(x) ((x)<eps&&(x)>-eps) ###nonsense if eps==0.0!!!
    #define FZ(x) ((x) == eps)
    //#define FEQ(x,y) (((x)-(y))<eps&&((x)-(y))>-eps) ###nonsense if eps==0.0!
    #define FEQ(x,y) ((x)==(y))
    if ( FZ(m12()) && FZ(m21()) && m11() >= -eps && m22() >= -eps ) {
	if ( FEQ(m11(),1.0) && FEQ(m22(),1.0) ) {
	    if ( !FZ(dx()) || !FZ(dy()) )
		txop = TxTranslate;
	} else {
	    txop = TxScale;
#if defined(_WS_WIN_)
	    setf(DirtyFont);
#endif
	}
    } else {
	txop = TxRotShear;
#if defined(_WS_WIN_)
	setf(DirtyFont);
#endif
    }
    #undef FZ
    #undef FEQ
}


/*!
  \internal
  Updates an internal integer inverse transformation matrix.
*/

void QPainter::updateInvXForm()
{
#if defined(CHECK_STATE)
    ASSERT( txinv == FALSE );
#endif
    txinv = TRUE;				// creating inverted matrix
    bool invertible;
    QWMatrix m;
    if ( testf(VxF) ) {
	m.translate( vx, vy );
	m.scale( 1.0*vw/ww, 1.0*vh/wh );
	m.translate( -wx, -wy );
    }
    if ( testf(WxF) ) {
	if ( testf(VxF) )
	    m = wxmat * m;
	else
	    m = wxmat;
    }
    ixmat = m.invert( &invertible );		// invert matrix
}


/*!
  \internal
  Maps a point from logical coordinates to device coordinates.
*/

void QPainter::map( int x, int y, int *rx, int *ry ) const
{
     switch ( txop ) {
	case TxNone:
	    *rx = x;  *ry = y;
	    break;
	case TxTranslate:
	    // #### "Why no rounding here?", Warwick asked of Haavard.
	    *rx = int(x + dx());
	    *ry = int(y + dy());
	    break;
	case TxScale: {
	    double tx = m11()*x + dx();
	    double ty = m22()*y + dy();
	    *rx = tx >= 0 ? int(tx + 0.5) : int(tx - 0.5);
	    *ry = ty >= 0 ? int(ty + 0.5) : int(ty - 0.5);
	    } break;
	default: {
	    double tx = m11()*x + m21()*y+dx();
	    double ty = m12()*x + m22()*y+dy();
	    *rx = tx >= 0 ? int(tx + 0.5) : int(tx - 0.5);
	    *ry = ty >= 0 ? int(ty + 0.5) : int(ty - 0.5);
	    } break;
    }
}

/*!
  \internal
  Maps a rectangle from logical coordinates to device coordinates.
  This internal function does not handle rotation and/or shear.
*/

void QPainter::map( int x, int y, int w, int h,
		    int *rx, int *ry, int *rw, int *rh ) const
{
     switch ( txop ) {
	case TxNone:
	    *rx = x;  *ry = y;
	    *rw = w;  *rh = h;
	    break;
	case TxTranslate:
	    // #### "Why no rounding here?", Warwick asked of Haavard.
	    *rx = int(x + dx());
	    *ry = int(y + dy());
	    *rw = w;  *rh = h;
	    break;
	case TxScale: {
	    double tx = m11()*x + dx();
	    double ty = m22()*y + dy();
	    double tw = m11()*w;
	    double th = m22()*h;
	    *rx = tx >= 0 ? int(tx + 0.5) : int(tx - 0.5);
	    *ry = ty >= 0 ? int(ty + 0.5) : int(ty - 0.5);
	    *rw = tw >= 0 ? int(tw + 0.5) : int(tw - 0.5);
	    *rh = th >= 0 ? int(th + 0.5) : int(th - 0.5);
	    } break;
	default:
#if defined(CHECK_STATE)
	    qWarning( "QPainter::map: Internal error" );
#endif
	    break;
    }
}

/*!
  \internal
  Maps a point from device coordinates to logical coordinates.
*/

void QPainter::mapInv( int x, int y, int *rx, int *ry ) const
{
#if defined(CHECK_STATE)
    if ( !txinv )
	qWarning( "QPainter::mapInv: Internal error" );
#endif
    double tx = im11()*x + im21()*y+idx();
    double ty = im12()*x + im22()*y+idy();
    *rx = tx >= 0 ? int(tx + 0.5) : int(tx - 0.5);
    *ry = ty >= 0 ? int(ty + 0.5) : int(ty - 0.5);
}

/*!
  \internal
  Maps a rectangle from device coordinates to logical coordinates.
  Cannot handle rotation and/or shear.
*/

void QPainter::mapInv( int x, int y, int w, int h,
		       int *rx, int *ry, int *rw, int *rh ) const
{
#if defined(CHECK_STATE)
    if ( !txinv || txop == TxRotShear )
	qWarning( "QPainter::mapInv: Internal error" );
#endif
    double tx = im11()*x + idx();
    double ty = im22()*y + idy();
    double tw = im11()*w;
    double th = im22()*h;
    *rx = tx >= 0 ? int(tx + 0.5) : int(tx - 0.5);
    *ry = ty >= 0 ? int(ty + 0.5) : int(ty - 0.5);
    *rw = tw >= 0 ? int(tw + 0.5) : int(tw - 0.5);
    *rh = th >= 0 ? int(th + 0.5) : int(th - 0.5);
}


/*!
  Returns the point \e pv transformed from user coordinates to device
  coordinates.

  \sa xFormDev(), QWMatrix::xForm()
*/

QPoint QPainter::xForm( const QPoint &pv ) const
{
    if ( txop == TxNone )
	return pv;
    int x=pv.x(), y=pv.y();
    map( x, y, &x, &y );
    return QPoint( x, y );
}

/*!
  Returns the rectangle \e rv transformed from user coordinates to device
  coordinates.

  If world transformation is enabled and rotation or shearing has been
  specified, then the bounding rectangle is returned.

  \sa xFormDev(), QWMatrix::xForm()
*/

QRect QPainter::xForm( const QRect &rv ) const
{
    if ( txop == TxNone )
	return rv;

    if ( txop == TxRotShear ) {			// rotation/shear
	QPointArray a( rv );
	a = xForm( a );
	return a.boundingRect();
    }

    // Just translation/scale
    int x, y, w, h;
    rv.rect( &x, &y, &w, &h );
    map( x, y, w, h, &x, &y, &w, &h );
    return QRect( x, y, w, h );
}

/*!
  Returns the point array \e av transformed from user coordinates to device
  coordinates.
  \sa xFormDev(), QWMatrix::xForm()
*/

QPointArray QPainter::xForm( const QPointArray &av ) const
{
    QPointArray a = av;
    if ( txop != TxNone ) {
	a = a.copy();
	int x, y, i;
	for ( i=0; i<(int)a.size(); i++ ) {
	    a.point( i, &x, &y );
	    map( x, y, &x, &y );
	    a.setPoint( i, x, y );
	}
    }
    return a;
}

/*!
  Returns the point array \a av transformed from user coordinates to device
  coordinates.  The \a index is the first point in the array and \a npoints
  denotes the number of points to be transformed.  If \a npoints is negative,
  all points from \a av[index] until the last point in the array are
  transformed.

  The returned point array consists of the number of points that were
  transformed.

  Example:
  \code
    QPointArray a(10);
    QPointArray b;
    b = painter.xForm(a,2,4);	// b.size() == 4
    b = painter.xForm(a,2,-1);	// b.size() == 8
  \endcode

  \sa xFormDev(), QWMatrix::xForm()
*/

QPointArray QPainter::xForm( const QPointArray &av, int index,
			     int npoints ) const
{
    int lastPoint = npoints < 0 ? av.size() : index+npoints;
    QPointArray a( lastPoint-index );
    int x, y, i=index, j=0;
    while ( i<lastPoint ) {
	av.point( i++, &x, &y );
	map( x, y, &x, &y );
	a.setPoint( j++, x, y );
    }
    return a;
}

/*!
  Returns the point \e pv transformed from device coordinates to user
  coordinates.
  \sa xForm(), QWMatrix::xForm()
*/

QPoint QPainter::xFormDev( const QPoint &pd ) const
{
    if ( txop == TxNone )
	return pd;
    if ( !txinv ) {
	QPainter *that = (QPainter*)this;	// mutable
	that->updateInvXForm();
    }
    int x=pd.x(), y=pd.y();
    mapInv( x, y, &x, &y );
    return QPoint( x, y );
}

/*!
  Returns the rectangle \e rv transformed from device coordinates to user
  coordinates.

  If world transformation is enabled and rotation or shearing is used,
  then the bounding rectangle is returned.

  \sa xForm(), QWMatrix::xForm()
*/

QRect QPainter::xFormDev( const QRect &rd ) const
{
    if ( txop == TxNone )
	return rd;
    if ( !txinv ) {
	QPainter *that = (QPainter*)this;	// mutable
	that->updateInvXForm();
    }
    if ( txop == TxRotShear ) {			// rotation/shear
	QPointArray a( rd );
	a = xFormDev( a );
	return a.boundingRect();
    }

    // Just translation/scale
    int x, y, w, h;
    rd.rect( &x, &y, &w, &h );
    mapInv( x, y, w, h, &x, &y, &w, &h );
    return QRect( x, y, w, h );
}

/*!
  Returns the point array \e av transformed from device coordinates to user
  coordinates.
  \sa xForm(), QWMatrix::xForm()
*/

QPointArray QPainter::xFormDev( const QPointArray &ad ) const
{
    QPointArray a = ad;
    if ( txop != TxNone ) {
	a = a.copy();
	int x, y, i;
	for ( i=0; i<(int)a.size(); i++ ) {
	    a.point( i, &x, &y );
	    mapInv( x, y, &x, &y );
	    a.setPoint( i, x, y );
	}
    }
    return a;
}

/*!
  Returns the point array \a ad transformed from device coordinates to user
  coordinates.  The \a index is the first point in the array and \a npoints
  denotes the number of points to be transformed.  If \a npoints is negative,
  all points from \a av[index] until the last point in the array are
  transformed.

  The returned point array consists of the number of points that were
  transformed.

  Example:
  \code
    QPointArray a(10);
    QPointArray b;
    b = painter.xFormDev(a,1,3);	// b.size() == 3
    b = painter.xFormDev(a,1,-1);	// b.size() == 9
  \endcode

  \sa xForm(), QWMatrix::xForm()
*/

QPointArray QPainter::xFormDev( const QPointArray &ad, int index,
				int npoints ) const
{
    int lastPoint = npoints < 0 ? ad.size() : index+npoints;
    QPointArray a( lastPoint-index );
    int x, y, i=index, j=0;
    while ( i<lastPoint ) {
	ad.point( i++, &x, &y );
	map( x, y, &x, &y );
	a.setPoint( j++, x, y );
    }
    return a;
}


/*!
  Fills the rectangle \e (x,y,w,h) with the \e brush.

  You can specify a QColor as \e brush, since there is a QBrush constructor
  that takes a QColor argument and creates a solid pattern brush.

  \sa drawRect()
*/

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


/*!
  \overload void QPainter::setBrushOrigin( const QPoint &p )
*/

/*!
  \overload void QPainter::setWindow( const QRect &r )
*/


/*!
  \overload void QPainter::setViewport( const QRect &r )
*/


/*!
  \fn bool QPainter::hasClipping() const
  Returns TRUE if clipping has been set, otherwise FALSE.
  \sa setClipping()
*/

/*!
  \fn const QRegion &QPainter::clipRegion() const

  Returns the clip region currently set.  Note that the clip region is
  given in physical device coordinates and \e not subject to any
  \link setWorldMatrix() coordinate transformation\endlink.

  \sa setClipRegion(), setClipRect(), setClipping()
*/

/*!
  \fn void QPainter::setClipRect( int x, int y, int w, int h )

  Sets the clip region to \e (x,y,w,h) and enables clipping.

  Note that the clip rectangle is given in physical device coordinates and
  \e not subject to any \link setWorldMatrix() coordinate
  transformation\endlink.

  \sa setClipRegion(), clipRegion(), setClipping()
*/

/*!
  \overload void QPainter::drawPoint( const QPoint &p )
*/


/*!
  \overload void QPainter::moveTo( const QPoint &p )
*/

/*!
  \overload void QPainter::lineTo( const QPoint &p )
*/

/*!
  \overload void QPainter::drawLine( const QPoint &p1, const QPoint &p2 )
*/

/*!
  \overload void QPainter::drawRect( const QRect &r )
*/

/*!
  \overload void QPainter::drawWinFocusRect( const QRect &r )
*/

/*!
  \overload void QPainter::drawWinFocusRect( const QRect &r, const QColor &bgColor )
*/


#if !defined(_WS_X11_)
// The doc and X implementation of this functions is in qpainter_x11.cpp
void QPainter::drawWinFocusRect( int, int, int, int,
				 bool, const QColor & )
{
    // do nothing, only called from X11 specific functions
}
#endif


/*!
  \overload void QPainter::drawRoundRect( const QRect &r, int xRnd, int yRnd )
*/

/*!
  \overload void QPainter::drawEllipse( const QRect &r )
*/

/*!
  \overload void QPainter::drawArc( const QRect &r, int a, int alen )
*/

/*!
  \overload void QPainter::drawPie( const QRect &r, int a, int alen )
*/

/*!
  \overload void QPainter::drawChord( const QRect &r, int a, int alen )
*/

/*!
  \overload void QPainter::drawPixmap( const QPoint &p, const QPixmap &pm, const QRect &sr )
*/

/*!
  \overload void QPainter::drawPixmap( const QPoint &p, const QPixmap &pm )

  This version of the call draws the entire pixmap.
*/

void QPainter::drawPixmap( const QPoint &p, const QPixmap &pm )
{
    drawPixmap( p.x(), p.y(), pm, 0, 0, pm.width(), pm.height() );
}


/*!
  Draws at (\a x, \a y) the \a sw by \a sh area of pixels
  from (\a sx, \a sy) in \a image.

  This function simply converts \a image to a QPixmap and draws it.

  \sa drawPixmap() QPixmap::convertFromImage()
*/
void QPainter::drawImage( int x, int y, const QImage & image,
			  int sx, int sy, int sw, int sh )
{
    if ( !isActive() || image.isNull() )
	return;

    // right/bottom
    if ( sw < 0 )
	sw = image.width()  - sx;
    if ( sh < 0 )
	sh = image.height() - sy;

    // Sanity-check clipping
    if ( sx < 0 ) {
	x -= sx;
	sw += sx;
	sx = 0;
    }
    if ( sw + sx > image.width() )
	sw = image.width() - sx;
    if ( sy < 0 ) {
	y -= sy;
	sh += sy;
	sy = 0;
    }
    if ( sh + sy > image.height() )
	sh = image.height() - sy;

    if ( sw <= 0 || sh <= 0 )
	return;

    bool all = image.rect().intersect(QRect(sx,sy,sw,sh)) == image.rect();
    QImage subimage = all ? image : image.copy(sx,sy,sw,sh);

    if ( testf(ExtDev) ) {
	QPDevCmdParam param[2];
	QPoint p(x,y);
	param[0].point = &p;
	param[1].image = &subimage;
#if defined(_WS_WIN_)
	if ( !pdev->cmd( QPaintDevice::PdcDrawImage, this, param ) || !hdc )
#else
	if ( !pdev->cmd( QPaintDevice::PdcDrawImage, this, param ) || !hd )
#endif
	    return;
    }

    QPixmap pm;
    pm.convertFromImage( subimage );
    drawPixmap( x, y, pm );
}

/*!
  \overload void QPainter::drawImage( const QPoint &, const QImage &, const QRect &sr )
*/

/*!
  \overload void QPainter::drawImage( const QPoint &, const QImage & )
*/
void QPainter::drawImage( const QPoint & p, const QImage & i )
{
    drawImage(p, i, i.rect());
}


void bitBlt( QPaintDevice *dst, int dx, int dy,
	     const QImage *src, int sx, int sy, int sw, int sh,
	     int conversion_flags )
{
    QPixmap tmp;
    if ( sx == 0 && sy == 0
	&& (sw<0 || sw==src->width()) && (sh<0 || sh==src->height()) )
    {
	tmp.convertFromImage( *src, conversion_flags );
    } else {
	tmp.convertFromImage( src->copy( sx, sy, sw, sh, conversion_flags),
			      conversion_flags );
    }
    bitBlt( dst, dx, dy, &tmp );
}


/*!
  \overload void QPainter::drawTiledPixmap( const QRect &r, const QPixmap &pm, const QPoint &sp )
*/

/*!
  \overload void QPainter::drawTiledPixmap( const QRect &r, const QPixmap &pm )
*/

/*!
  \overload void QPainter::fillRect( const QRect &r, const QBrush &brush )
*/

/*!
  \fn void QPainter::eraseRect( int x, int y, int w, int h )
  Erases the area inside \e (x,y,w,h).
  Equivalent to <code>fillRect( x, y, w, h, backgroundColor() )</code>.
*/

/*!
  \overload void QPainter::eraseRect( const QRect &r )
*/

/*!
  \overload void QPainter::drawText( const QPoint &p, const QString&, int len )
*/

/*!
  \overload void QPainter::drawText( const QRect &r, int tf, const QString&, int len, QRect *br, char **i )
*/

/*!
  \overload QRect QPainter::boundingRect( const QRect &r, int tf, const QString&, int len, char **i )
*/


static inline void fix_neg_rect( int *x, int *y, int *w, int *h )
{
    if ( *w < 0 ) {
	*w = -*w;
	*x -= *w - 1;
    }
    if ( *h < 0 ) {
	*h = -*h;
	*y -= *h - 1;
    }
}
void QPainter::fix_neg_rect( int *x, int *y, int *w, int *h )
{
    ::fix_neg_rect(x,y,w,h);
}

//
// The drawText function takes two special parameters; 'internal' and 'brect'.
//
// The 'internal' parameter contains a pointer to an array of encoded
// information that keeps internal geometry data.
// If the drawText function is called repeatedly to display the same text,
// it makes sense to calculate text width and linebreaks the first time,
// and use these parameters later to print the text because we save a lot of
// CPU time.
// The 'internal' parameter will not be used if it is a null pointer.
// The 'internal' parameter will be generated if it is not null, but points
// to a null pointer, i.e. internal != 0 && *internal == 0.
// The 'internal' parameter will be used if it contains a non-null pointer.
//
// If the 'brect parameter is a non-null pointer, then the bounding rectangle
// of the text will be returned in 'brect'.
//

/*!
  Draws at most \e len characters from \e str in the rectangle \e (x,y,w,h).

  Note that the meaning of \a y is not the same for the two drawText()
  varieties.

  This function draws formatted text.  The \e tf text formatting is
  the bitwise OR of the following flags:  <ul>
  <li> \c AlignLeft aligns to the left border.
  <li> \c AlignRight aligns to the right border.
  <li> \c AlignHCenter aligns horizontally centered.
  <li> \c AlignTop aligns to the top border.
  <li> \c AlignBottom aligns to the bottom border.
  <li> \c AlignVCenter aligns vertically centered
  <li> \c AlignCenter (= \c AlignHCenter | AlignVCenter)
  <li> \c SingleLine ignores newline characters in the text.
  <li> \c DontClip never clips the text to the rectangle.
  <li> \c ExpandTabs expands tabulators.
  <li> \c ShowPrefix displays "&x" as "x" underlined.
  <li> \c WordBreak breaks the text to fit the rectangle.
  </ul>

  Horizontal alignment defaults to AlignLeft and vertical alignment
  defaults to AlignTop.

  If several of the horizontal or several of the vertical alignment flags
  are set, the resulting alignment is undefined.

  If ExpandTabs is set and no \link setTabStops() tab stops \endlink or
  \link setTabArray() tab array \endlink have been set tabs will expand to
  the closest reasonable tab stop based on the current font. For \link
  QFont::setFixedPitch() fixed pitch\endlink (fixed width) fonts you are
  guaranteed that each tab stop will be at a multiple of eight of the
  width of the characters in the font.

  \a brect (if non-null) is set to the actual bounding rectangle of
  the output.  \a internal is, yes, internal.

  These flags are defined in qnamespace.h.

  \sa boundingRect()
*/

void QPainter::drawText( int x, int y, int w, int h, int tf,
			 const QString& str, int len, QRect *brect,
			 char **internal )
{
    if ( !isActive() )
	return;
    if ( len < 0 )
	len = str.length();
    if ( len == 0 )				// empty string
	return;

    if ( testf(DirtyFont|ExtDev) ) {
	if ( testf(DirtyFont) )
	    updateFont();
	if ( testf(ExtDev) && (tf & DontPrint) == 0 ) {
	    QPDevCmdParam param[3];
	    QRect r( x, y, w, h );
	    QString newstr = str;
	    newstr.truncate( len );
	    param[0].rect = &r;
	    param[1].ival = tf;
	    param[2].str = &newstr;
	    if ( pdev->devType() != QInternal::Printer ) {
#if defined(_WS_WIN_)
		if ( !pdev->cmd( QPaintDevice::PdcDrawText2Formatted,
				 this, param) ||
		     !hdc )
#else
		if ( !pdev->cmd( QPaintDevice::PdcDrawText2Formatted,
				 this, param) ||
		     !hd )
#endif
		    return;			// QPrinter wants PdcDrawText2
	    }
	}
    }

    const QFontMetrics & fm = fontMetrics();		// get font metrics

    qt_format_text(fm, x, y, w, h, tf, str, len, brect,
		   tabstops, tabarray, tabarraylen, internal, this);
}


void qt_format_text( const QFontMetrics& fm, int x, int y, int w, int h,
		     int tf, const QString& str, int len, QRect *brect,
		     int tabstops, int* tabarray, int tabarraylen,
		     char **internal, QPainter* painter )
{
    if ( w <= 0 || h <= 0 )
	fix_neg_rect( &x, &y, &w, &h );

    struct text_info {				// internal text info
	char  tag[4];				// contains "qptr"
	int   w;				// width
	int   h;				// height
	int   tf;				// flags (alignment etc.)
	int   len;				// text length
	int   maxwidth;				// max text width
	int   nlines;				// number of lines
	int   codelen;				// length of encoding
    };

    uint codearray[200];
    int	   codelen    = 200;
    bool   code_alloc = FALSE;
    uint *codes     = codearray;
    uint cc;					// character code
    bool   decode     = internal && *internal;	// decode from internal data
    bool   encode     = internal && !*internal; // build internal data

    if ( len > 150 && !decode ) {		// need to alloc code array
	codelen = len + len/2; // ### enough? 200 != 150*1.5 -- WWA
	codes	= (uint *)malloc( codelen*sizeof(uint) );
	code_alloc = TRUE;
    }

    const uint BEGLINE  = 0x80000000;	// encoding 0x8000zzzz, z=width
    const uint TABSTOP  = 0x40000000;	// encoding 0x4000zzzz, z=tab pos
    const uint PREFIX   = 0x20000000;	// encoding 0x2000hilo
    const uint HI       = 0x0000ff00;	//  hi,lo=QChar
    const uint LO       = 0x000000ff;
    const int HI_SHIFT = 8;
    const int LO_SHIFT = 0;
    // An advanced display function might provide for different fonts, etc.
    const int WIDTHBITS= 0x1fffffff;	// bits for width encoding
    const int MAXWIDTH = 0x1fffffff;	// max width value

    const QChar *p = str.unicode();
    int nlines;					// number of lines
    int index;					// index for codes
    int begline;				// index at beginning of line
    int breakindex;				// index where to break
    int breakwidth;				// width of text at breakindex
    int maxwidth;				// maximum width of a line
    int bcwidth;				// width of break char
    int tabindex;				// tab array index
    int cw;					// character width
    int k;					// index for p
    int tw;					// text width

#define CWIDTH(x) fm.width(x) // Could cache, but put that it in fm
#define ENCCHAR(x) (((x).cell() << LO_SHIFT) | ((x).row() << HI_SHIFT))
#define DECCHAR(x) QChar(((x)&LO)>>LO_SHIFT,((x)&HI)>>HI_SHIFT)
#define ISPRINT(x) ((x).row() || (x).cell()>' ')
    // ##### should use (unicode) QChar::isPrint() -- WWA to AG

    bool wordbreak  = (tf & Qt::WordBreak)  == Qt::WordBreak;
    bool expandtabs = (tf & Qt::ExpandTabs) == Qt::ExpandTabs;
    bool singleline = (tf & Qt::SingleLine) == Qt::SingleLine;
    bool showprefix = (tf & Qt::ShowPrefix) == Qt::ShowPrefix;

    int	 spacewidth = CWIDTH( QChar(' ') );	// width of space char

    nlines = 0;
    index  = 1;					// first index contains BEGLINE
    begline = breakindex = breakwidth = maxwidth = bcwidth = tabindex = 0;
    k = tw = 0;

    if ( decode )				// skip encoding
	k = len;

    int localTabStops = 0;	       		// tab stops
    if ( tabstops )
	localTabStops = tabstops;
    else
	localTabStops = fm.width(QChar('x'))*8;       	// default to 8 times x

    QString word;

    bool fakeBreak = FALSE;
    bool breakwithinwords = FALSE;
    while ( k <= len ) {				// convert string to codes
	if ( !fakeBreak && k < len && ISPRINT(*p) ) {			// printable character
	    if ( *p == '&' && showprefix ) {
		cc = '&';			// assume ampersand
		if ( k < len-1 ) {
		    k++;
		    p++;
		    if ( *p != '&' && ISPRINT(*p) )
			cc = PREFIX | ENCCHAR(*p);// use prefix char
		}
	    } else {
		cc = ENCCHAR(*p);
	    }
	
	    cw = 0;
 	    if ( breakwithinwords ) {
 		breakwidth += fm.width( cc );
 		if ( breakwidth > w ) {
 		    fakeBreak = TRUE;
 		    continue;
 		}
 	    }
	    word += *p;
	} else {				// not printable (except ' ')
	    cw = fm.width(word);
	    if ( !fakeBreak && wordbreak ) {
		if ( tw+cw > w ) {
		    if ( breakindex > 0 ) {
			breakwithinwords = FALSE;
			codes[begline] = BEGLINE | QMIN(tw,MAXWIDTH);
			maxwidth = QMAX(maxwidth,tw);
			begline = breakindex;
			tw = cw;
			breakindex = tabindex = 0;
			cw = 0;
			nlines++;
		    }
		    if ( tw+cw > w && word.length() > 1) {
			breakwithinwords = TRUE;
			breakwidth = 0;
			p -= word.length();	
			k -= word.length();
			index = begline+1;
			tw = 0;
			word = "";
			continue;
		    }
		}
	    }
	    word = "";

 	    if ( fakeBreak ) {
 		cc = BEGLINE;
 		fakeBreak = FALSE;
  		--k;
  		--p;
 	    }
 	    else
	    if ( k == len ) {
		// end (*p not valid)
		cc = 0;
	    } else if ( *p == ' ' ) {			// the space character
		cc = ' ';
		cw += spacewidth;
	    } else if ( *p == '\n' ) {		// newline
		if ( singleline ) {
		    cc = ' ';			// convert newline to space
		    cw += spacewidth;
		} else {
		    cc = BEGLINE;
		}
	    } else if ( *p == '\t' ) {		// TAB character
		if ( expandtabs ) {
		    int ccw = 0;
		    if ( tabarray ) {		// use tab array
			while ( tabindex < tabarraylen ) {
			    if ( tabarray[tabindex] > (tw+cw) ) {
				ccw = tabarray[tabindex] - (tw+cw);
				tabindex++;
				break;
			    }
			    tabindex++;
			}
		    }
		    if ( ccw == 0 )		// use fixed tab stops
			ccw = localTabStops - (tw+cw)%localTabStops;
		    cw += ccw;
		    cc = TABSTOP | QMIN(tw+cw,MAXWIDTH);
		} else {			// convert TAB to space
		    cc = ' ';
		    cw += spacewidth;
		}
	    } else {				// ignore character
		k++;
		p++;
		continue;
	    }
	    breakindex = index;
	    breakwidth = 0;
	    bcwidth = cw;
	}

	tw += cw;				// increment text width

	if ( cc == BEGLINE ) {
	    breakwithinwords = FALSE;
	    codes[begline] = BEGLINE | QMIN(tw,MAXWIDTH);
	    maxwidth = QMAX(maxwidth,tw);
	    begline = index;
	    nlines++;
	    tw = 0;
	    breakindex = tabindex = 0;
	}
	codes[index++] = cc;
	if ( index >= codelen - 1 ) {		// grow code array
	    codelen *= 2;
	    if ( code_alloc ) {
		codes = (uint *)realloc( codes, sizeof(uint)*codelen );
	    } else {
		codes = (uint *)malloc( sizeof(uint)*codelen );
		code_alloc = TRUE;
	    }
	}
	k++;
	p++;
    }

    if ( decode ) {				// decode from internal data
	char	  *data = *internal;
	text_info *ti	= (text_info*)data;
	if ( strncmp(ti->tag,"qptr",4)!=0 || ti->w != w || ti->h != h ||
	     ti->tf != tf || ti->len != len ) {
#if defined(CHECK_STATE)
	    qWarning( "QPainter::drawText: Internal text info is invalid" );
#endif
	    return;
	}
	maxwidth = ti->maxwidth;		// get internal values
	nlines	 = ti->nlines;
	codelen	 = ti->codelen;
	codes	 = (uint *)(data + sizeof(text_info));
    } else {
	codes[begline] = BEGLINE | QMIN(tw,MAXWIDTH);
	maxwidth = QMAX(maxwidth,tw);
	nlines++;
	codes[index++] = 0;
	codelen = index;
    }

    if ( encode ) {				// build internal data
	char	  *data = new char[sizeof(text_info)+codelen*sizeof(uint)];
	text_info *ti	= (text_info*)data;
	strncpy( ti->tag, "qptr", 4 );		// set tag
	ti->w	     = w;			// save parameters
	ti->h	     = h;
	ti->tf	     = tf;
	ti->len	     = len;
	ti->maxwidth = maxwidth;
	ti->nlines   = nlines;
	ti->codelen  = codelen;
	memcpy( data+sizeof(text_info), codes, codelen*sizeof(uint) );
	*internal = data;
    }

    int	    fascent  = fm.ascent();		// get font measurements
    int	    fheight  = fm.height();
    int	    xp, yp;
    int	    xc;					// character xp

    if ( (tf & Qt::AlignVCenter) == Qt::AlignVCenter )	// vertically centered text
	yp = h/2 - nlines*fheight/2;
    else if ( (tf & Qt::AlignBottom) == Qt::AlignBottom)// bottom aligned
	yp = h - nlines*fheight;
    else					// top aligned
	yp = 0;
    int overflow = -fm.minLeftBearing()-fm.minRightBearing();
    if ( (tf & Qt::AlignRight) == Qt::AlignRight ) {
	maxwidth += overflow;
	xp = w - maxwidth;			// right aligned
    } else if ( (tf & Qt::AlignHCenter) == Qt::AlignHCenter ) {
	maxwidth += overflow;
	xp = w/2 - maxwidth/2;			// centered text
    } else {
	maxwidth += overflow;
	xp = 0;				// left aligned
    }

#if defined(CHECK_RANGE)
    int hAlignFlags = 0;
    if ( (tf & Qt::AlignRight) == Qt::AlignRight )
	hAlignFlags++;
    if ( (tf & Qt::AlignHCenter) == Qt::AlignHCenter )
	hAlignFlags++;
    if ( (tf & Qt::AlignLeft ) == Qt::AlignLeft )
	hAlignFlags++;

    if ( hAlignFlags > 1 )
	qWarning("QPainter::drawText: More than one of AlignRight, AlignLeft\n"
		 "\t\t    and AlignHCenter set in the tf parameter.");

    int vAlignFlags = 0;
    if ( (tf & Qt::AlignTop) == Qt::AlignTop )
	vAlignFlags++;
    if ( (tf & Qt::AlignVCenter) == Qt::AlignVCenter )
	vAlignFlags++;
    if ( (tf & Qt::AlignBottom ) == Qt::AlignBottom )
	vAlignFlags++;

    if ( hAlignFlags > 1 )
	qWarning("QPainter::drawText: More than one of AlignTop, AlignBottom\n"
		 "\t\t    and AlignVCenter set in the tf parameter.");
#endif // CHECK_RANGE

    QRect br( x+xp, y+yp, maxwidth, nlines*fheight );
    if ( brect )				// set bounding rect
	*brect = br;

    if ( !painter || (tf & Qt::DontPrint) != 0 ) {// can't/don't print any text
	if ( code_alloc )
	    free( codes );
	return;
    }

    // From here, we have a painter.

    QRegion save_rgn = painter->crgn;		// save the current region
    bool    clip_on  = painter->testf(QPainter::ClipOn);

    if ( br.x() >= x && br.y() >= y && br.width() < w && br.height() < h )
	tf |= Qt::DontClip;				// no need to clip

    if ( (tf & Qt::DontClip) == 0 ) {		// clip text
	QRegion new_rgn;
	QRect r( x, y, w, h );
	if ( painter->txop == TxRotShear ) {		// world xform active
	    QPointArray a( r );			// complex region
	    a = painter->xForm( a );
	    new_rgn = QRegion( a );
	} else {
	    r = painter->xForm( r );
	    new_rgn = QRegion( r );
	}
	if ( clip_on )				// combine with existing region
	    new_rgn = new_rgn.intersect( painter->crgn );
	painter->setClipRegion( new_rgn );
    }

    QBitmap  *mask;
    QPainter *pp;
    QPixmap *pm;

    mask = 0;
    pp = 0;
    pm = 0;

    yp += fascent;

    uint *cp = codes;

    while ( *cp ) {				// finally, draw the text
	tw = *cp++ & WIDTHBITS;			// text width

	if ( tw == 0 ) {			// ignore empty line
	    while ( *cp && (*cp & BEGLINE) != BEGLINE )
		cp++;
	    yp += fheight;
	    continue;
	}

	if ( (tf & Qt::AlignRight) == Qt::AlignRight ) {
	    xc = w - tw + fm.minRightBearing();
	} else if ( (tf & Qt::AlignHCenter) == Qt::AlignHCenter ) {
	    xc = w/2 - (tw-fm.minLeftBearing()-fm.minRightBearing())/2
		-fm.minLeftBearing();
	} else {
	    xc = -fm.minLeftBearing();
	}

	if ( pp )				// erase pixmap if gray text
	    pp->fillRect( 0, 0, w, fheight, Qt::color0 );

	int bxc = xc;				// base x position (chars)
	while ( TRUE ) {
	    QString chunk;
	    while ( *cp && (*cp & (BEGLINE|TABSTOP)) == 0 ) {
		if ( (*cp & PREFIX) == PREFIX ) {
		    int xcpos = fm.width( chunk );
		    if ( pp )			// gray text
			pp->fillRect( xc+xcpos, fascent+fm.underlinePos(),
				      CWIDTH(DECCHAR(*cp)), fm.lineWidth(),
				      Qt::color1 );
		    else
			painter->fillRect( x+xc+xcpos, y+yp+fm.underlinePos(),
					   CWIDTH(DECCHAR(*cp)), fm.lineWidth(),
					   painter->cpen.color() );
		}
		chunk += DECCHAR(*cp);
		++cp;
	    }
	    if ( pp )				// gray text
		pp->drawText( xc, fascent, chunk );
	    else
		painter->drawText( x+xc, y+yp, chunk );// draw the text
	    if ( (*cp & TABSTOP) == TABSTOP ) {
		int w = (*cp++ & WIDTHBITS);
		xc = bxc + w;
	    } else {				// *cp == 0 || *cp == BEGLINE
		break;
	    }
	}
	if ( pp ) {				// gray text
	    pp->setPen(Qt::color0);
	    pp->drawRect( mask->rect() );
	    pp->setPen(Qt::color1);
	    pm->fill( painter->cpen.color() );
	    pp->end();
	    pm->setMask( *mask );
	    painter->drawPixmap( x, y+yp-fascent, *pm );
	    pp->begin( mask );
	}

	yp += fheight;
    }

    if ( pp ) {					// gray text
	pp->end();
	delete pp;
	delete pm;
    }

    if ( (tf & Qt::DontClip) == 0 ) {		// restore clipping
	if ( clip_on ) {			// set original region
	    painter->setClipRegion( save_rgn );
	} else {				// clipping was off
	    painter->crgn = save_rgn;
	    painter->setClipping( FALSE );
	}
    }

    if ( code_alloc )
	free( codes );
}


/*!

  Returns the bounding rectangle of the aligned text that would be
  printed with the corresponding drawText() function (the first \e len
  characters from \e str).  The drawing, and hence the bounding
  rectangle, is constrained to the rectangle \e (x,y,w,h).

  The \e tf text formatting is the bitwise OR of the following flags:
  <ul>
  <li> \c AlignLeft aligns to the left border.
  <li> \c AlignRight aligns to the right border.
  <li> \c AlignHCenter aligns horizontally centered.
  <li> \c AlignTop aligns to the top border.
  <li> \c AlignBottom aligns to the bottom border.
  <li> \c AlignVCenter aligns vertically centered
  <li> \c AlignCenter (= \c AlignHCenter | \c AlignVCenter)
  <li> \c SingleLine ignores newline characters in the text.
  <li> \c ExpandTabs expands tabulators.
  <li> \c ShowPrefix displays "&x" as "x" underlined.
  <li> \c WordBreak breaks the text to fit the rectangle.
  </ul>

  These flags are defined in qnamespace.h.

  \sa drawText(), fontMetrics()
*/

QRect QPainter::boundingRect( int x, int y, int w, int h, int tf,
			      const QString& str, int len, char **internal )
{
    QRect brect;
    if ( str.isEmpty() )
	brect.setRect( x,y, 0,0 );
    else
	drawText( x, y, w, h, tf | DontPrint, str, len, &brect, internal );
    return brect;
}

/*****************************************************************************
  QPen member functions
 *****************************************************************************/

/*!
  \class QPen qpen.h
  \brief The QPen class defines how a QPainter should draw lines and outlines
  of shapes.
  \ingroup drawing
  \ingroup shared

  A pen has a style, a width and a color.

  The pen style defines the line type. The default pen style is \c Qt::SolidLine.
  Setting the style to \c NoPen tells the painter to not draw lines or
  outlines.

  The pen width defines the line width. The default line width is 0,
  which draws a 1-pixel line very fast, but with lower precision than
  with a line width of 1. Setting the line width to 1 or more draws
  lines that are precise, but drawing is slower.

  The pen color defines the color of lines and text. The default line
  color is black.  The QColor documentation lists predefined colors.

  Use the QBrush class for specifying fill styles.

  Example:
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

  See the setStyle() function for a complete list of pen styles.

  \sa QPainter, QPainter::setPen()
*/


/*!
  \internal
  Initializes the pen.
*/

void QPen::init( const QColor &color, uint width, PenStyle style )
{
    data = new QPenData;
    CHECK_PTR( data );
    data->style = style;
    data->width = width;
    data->color = color;
}

/*!
  Constructs a default black solid line pen with 0 width.
*/

QPen::QPen()
{
    init( Qt::black, 0, SolidLine );		// default pen
}

/*!
  Constructs a	pen black with 0 width and a specified style.
  \sa setStyle()
*/

QPen::QPen( PenStyle style )
{
    init( Qt::black, 0, style );
}

/*!
  Constructs a pen with a specified color, width and style.
  \sa setWidth(), setStyle(), setColor()
*/

QPen::QPen( const QColor &color, uint width, PenStyle style )
{
    init( color, width, style );
}

/*!
  Constructs a pen which is a copy of \e p.
*/

QPen::QPen( const QPen &p )
{
    data = p.data;
    data->ref();
}

/*!
  Destroys the pen.
*/

QPen::~QPen()
{
    if ( data->deref() )
	delete data;
}


/*!
  Detaches from shared pen data to makes sure that this pen is the only
  one referring the data.

  If multiple pens share common data, this pen dereferences the data
  and gets a copy of the data. Nothing is done if there is just a
  single reference.
*/

void QPen::detach()
{
    if ( data->count != 1 )
	*this = copy();
}


/*!
  Assigns \e c to this pen and returns a reference to this pen.
*/

QPen &QPen::operator=( const QPen &p )
{
    p.data->ref();				// beware of p = p
    if ( data->deref() )
	delete data;
    data = p.data;
    return *this;
}


/*!
  Returns a
  \link shclass.html deep copy\endlink of the pen.
*/

QPen QPen::copy() const
{
    QPen p( data->color, data->width, data->style );
    return p;
}


/*!
  \fn PenStyle QPen::style() const
  Returns the pen style.
  \sa setStyle()
*/

/*!
  Sets the pen style to \e s.

  \sa style()
*/

void QPen::setStyle( PenStyle s )
{
    if ( data->style == s )
	return;
    detach();
    data->style = s;
}


/*!
  \fn uint QPen::width() const
  Returns the pen width.
  \sa setWidth()
*/

/*!
  Sets the pen width to \e w.
  \sa width()
*/

void QPen::setWidth( uint w )
{
    if ( data->width == w )
	return;
    detach();
    data->width = w;
}


/*!
  \fn const QColor &QPen::color() const
  Returns the pen color.
  \sa setColor()
*/

/*!
  Sets the pen color to \e c.
  \sa color()
*/

void QPen::setColor( const QColor &c )
{
    detach();
    data->color = c;
}


/*!
  \fn bool QPen::operator!=( const QPen &p ) const

  Returns TRUE if the pen is different from \e p, or FALSE if the pens
  are equal.

  Two pens are different if they have different styles, widths or colors.

  \sa operator==()
*/

/*!
  Returns TRUE if the pen is equal to \e p, or FALSE if the pens are
  different.

  Two pens are equal if they have equal styles, widths and colors.

  \sa operator!=()
*/

bool QPen::operator==( const QPen &p ) const
{
    return (p.data == data) || (p.data->style == data->style &&
	    p.data->width == data->width && p.data->color == data->color);
}


/*****************************************************************************
  QPen stream functions
 *****************************************************************************/

/*!
  \relates QPen
  Writes a pen to the stream and returns a reference to the stream.

  The serialization format is:
  <ol>
  <li> The pen style (UINT8)
  <li> The pen width (UINT8)
  <li> The pen color (QColor)
  </ol>
*/

QDataStream &operator<<( QDataStream &s, const QPen &p )
{
    return s << (UINT8)p.style() << (UINT8)p.width() << p.color();
}

/*!
  \relates QPen
  Reads a pen from the stream and returns a reference to the stream.
*/

QDataStream &operator>>( QDataStream &s, QPen &p )
{
    UINT8 style, width;
    QColor color;
    s >> style;
    s >> width;
    s >> color;
    p = QPen( color, (uint)width, (Qt::PenStyle)style );
    return s;
}


/*****************************************************************************
  QBrush member functions
 *****************************************************************************/

/*!
  \class QBrush qbrush.h

  \brief The QBrush class defines the fill pattern of shapes drawn by a QPainter.

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

  Example:
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

  See the setStyle() function for a complete list of brush styles.

  \sa QPainter, QPainter::setBrush(), QPainter::setBrushOrigin()
*/


/*!
  \internal
  Initializes the brush.
*/

void QBrush::init( const QColor &color, BrushStyle style )
{
    data = new QBrushData;
    CHECK_PTR( data );
    data->style	 = style;
    data->color	 = color;
    data->pixmap = 0;
}

/*!
  Constructs a default black brush with the style \c NoBrush (will not fill
  shapes).
*/

QBrush::QBrush()
{
    init( Qt::black, NoBrush );
}

/*!
  Constructs a black brush with the specified style.
  \sa setStyle()
*/

QBrush::QBrush( BrushStyle style )
{
    init( Qt::black, style );
}

/*!
  Constructs a brush with a specified color and style.
  \sa setColor(), setStyle()
*/

QBrush::QBrush( const QColor &color, BrushStyle style )
{
    init( color, style );
}

/*!
  Constructs a brush with a specified color and a custom pattern.

  The color will only have an effect for monochrome pixmaps, i.e.
  QPixmap::depth() == 1.

  \sa setColor(), setPixmap()
*/

QBrush::QBrush( const QColor &color, const QPixmap &pixmap )
{
    init( color, CustomPattern );
    setPixmap( pixmap );
}

/*!
  Constructs a brush which is a
  \link shclass.html shallow copy\endlink of \e b.
*/

QBrush::QBrush( const QBrush &b )
{
    data = b.data;
    data->ref();
}

/*!
  Destroys the brush.
*/

QBrush::~QBrush()
{
    if ( data->deref() ) {
	delete data->pixmap;
	delete data;
    }
}


/*!
  Detaches from shared brush data to makes sure that this brush is the only
  one referring the data.

  If multiple brushes share common data, this pen dereferences the data
  and gets a copy of the data. Nothing is done if there is just a single
  reference.
*/

void QBrush::detach()
{
    if ( data->count != 1 )
	*this = copy();
}


/*!
  Assigns \e b to this brush and returns a reference to this brush.
*/

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


/*!
  Returns a
  \link shclass.html deep copy\endlink of the brush.
*/

QBrush QBrush::copy() const
{
    if ( data->style == CustomPattern ) {     // brush has pixmap
	QBrush b( data->color, *data->pixmap );
	return b;
    } else {				      // brush has std pattern
	QBrush b( data->color, data->style );
	return b;
    }
}


/*!
  \fn BrushStyle QBrush::style() const
  Returns the brush style.
  \sa setStyle()
*/

/*!
  Sets the brush style to \e s.

  The brush styles are:
  <ul>
  <li> \c NoBrush  will not fill shapes (default).
  <li> \c SolidPattern  solid (100%) fill pattern.
  <li> \c Dense1Pattern  94% fill pattern.
  <li> \c Dense2Pattern  88% fill pattern.
  <li> \c Dense3Pattern  63% fill pattern.
  <li> \c Dense4Pattern  50% fill pattern.
  <li> \c Dense5Pattern  37% fill pattern.
  <li> \c Dense6Pattern  12% fill pattern.
  <li> \c Dense7Pattern  6% fill pattern.
  <li> \c HorPattern  horizontal lines pattern.
  <li> \c VerPattern  vertical lines pattern.
  <li> \c CrossPattern  crossing lines pattern.
  <li> \c BDiagPattern  diagonal lines (directed / ) pattern.
  <li> \c FDiagPattern  diagonal lines (directed \ ) pattern.
  <li> \c DiagCrossPattern  diagonal crossing lines pattern.
  <li> \c CustomPattern  set when a pixmap pattern is being used.
  </ul>

  \sa style()
*/

void QBrush::setStyle( BrushStyle s )		// set brush style
{
    if ( data->style == s )
	return;
#if defined(CHECK_RANGE)
    if ( s == CustomPattern )
	qWarning( "QBrush::setStyle: CustomPattern is for internal use" );
#endif
    detach();
    data->style = s;
}


/*!
  \fn const QColor &QBrush::color() const
  Returns the brush color.
  \sa setColor()
*/

/*!
  Sets the brush color to \e c.
  \sa color(), setStyle()
*/

void QBrush::setColor( const QColor &c )
{
    detach();
    data->color = c;
}


/*!
  \fn QPixmap *QBrush::pixmap() const
  Returns a pointer to the custom brush pattern.

  A null pointer is returned if no custom brush pattern has been set.

  \sa setPixmap()
*/

/*!
  Sets the brush pixmap.  The style is set to \c CustomPattern.

  The current brush color will only have an effect for monochrome pixmaps,
  i.e.	QPixmap::depth() == 1.

  \sa pixmap(), color()
*/

void QBrush::setPixmap( const QPixmap &pixmap )
{
    detach();
    if ( data->pixmap )
	delete data->pixmap;
    if ( pixmap.isNull() ) {
	data->style  = NoBrush;
	data->pixmap = 0;
    } else {
	data->style = CustomPattern;
	data->pixmap = new QPixmap( pixmap );
	if ( data->pixmap->optimization() == QPixmap::MemoryOptim )
	    data->pixmap->setOptimization( QPixmap::NormalOptim );
    }
}


/*!
  \fn bool QBrush::operator!=( const QBrush &b ) const
  Returns TRUE if the brush is different from \e b, or FALSE if the brushes are
  equal.

  Two brushes are different if they have different styles, colors or pixmaps.

  \sa operator==()
*/

/*!
  Returns TRUE if the brush is equal to \e b, or FALSE if the brushes are
  different.

  Two brushes are equal if they have equal styles, colors and pixmaps.

  \sa operator!=()
*/

bool QBrush::operator==( const QBrush &b ) const
{
    return (b.data == data) || (b.data->style == data->style &&
	    b.data->color  == data->color &&
	    b.data->pixmap == data->pixmap);
}


/*****************************************************************************
  QBrush stream functions
 *****************************************************************************/

/*!
  \relates QBrush
  Writes a brush to the stream and returns a reference to the stream.

  The serialization format is:
  <ol>
  <li> The brush style (UINT8)
  <li> The brush color (QColor)
  <li> If style == \c CustomPattern: the brush pixmap (QPixmap)
  </ol>
*/

QDataStream &operator<<( QDataStream &s, const QBrush &b )
{
    s << (UINT8)b.style() << b.color();
    if ( b.style() == Qt::CustomPattern )
	s << *b.pixmap();
    return s;
}

/*!
  \relates QBrush
  Reads a brush from the stream and returns a reference to the stream.
*/

QDataStream &operator>>( QDataStream &s, QBrush &b )
{
    UINT8 style;
    QColor color;
    s >> style;
    s >> color;
    if ( style == Qt::CustomPattern ) {
	QPixmap pm;
	s >> pm;
	b = QBrush( color, pm );
    }
    else
	b = QBrush( color, (Qt::BrushStyle)style );
    return s;
}


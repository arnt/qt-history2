/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qpushbutton.cpp#41 $
**
** Implementation of QPushButton class
**
** Author  : Haavard Nord
** Created : 940221
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpushbt.h"
#include "qdialog.h"
#include "qfontmet.h"
#include "qpainter.h"
#include "qpixmap.h"
#include "qpmcache.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qpushbutton.cpp#41 $";
#endif


/*----------------------------------------------------------------------------
  \class QPushButton qpushbt.h
  \brief The QPushButton widget provides a push button with a text label.

  \ingroup realwidgets

  A default push button in a dialog emits the clicked signal if the user
  presses the Enter key.
 ----------------------------------------------------------------------------*/


const int extraMacWidth = 6;			// extra size for def button
const int extraMacHeight = 6;
const int extraPMWidth = 2;
const int extraPMHeight = 2;
const int extraMotifWidth = 10;
const int extraMotifHeight = 10;


static bool extraSize( const QPushButton *b, int &wx, int &hx,
		       bool onlyWhenDefault )
{
    if ( onlyWhenDefault && !b->isDefault() ) {
	wx = hx = 0;
	return FALSE;
    }
    switch ( b->style() ) {
	case MacStyle:				// larger def Mac buttons
	    wx = extraMacWidth;
	    hx = extraMacHeight;
	    break;
	case MotifStyle:			// larger def Motif buttons
	    wx = extraMotifWidth;
	    hx = extraMotifHeight;
	    break;
	default:
	    wx = hx = 0;
	    return FALSE;
    }
    return TRUE;
}

static void resizeDefButton( QPushButton *b )
{
    int wx, hx;
    if ( !extraSize( b, wx, hx, FALSE ) )
	return;
    if ( !b->isDefault() ) {			// not default -> shrink
	wx = -wx;
	hx = -hx;
    }
    QRect r = b->geometry();
    b->QWidget::setGeometry( r.x()-wx/2, r.y()-hx/2,
			     r.width()+wx, r.height()+hx );
}


/*----------------------------------------------------------------------------
  Constructs a push button with no text.

  The \e parent and \e name arguments are sent to the QWidget constructor.
 ----------------------------------------------------------------------------*/

QPushButton::QPushButton( QWidget *parent, const char *name )
	: QButton( parent, name )
{
    init();
}

/*----------------------------------------------------------------------------
  Constructs a push button with a text.

  The \e parent and \e name arguments are sent to the QWidget constructor.
 ----------------------------------------------------------------------------*/

QPushButton::QPushButton( const char *text, QWidget *parent,
			  const char *name )
	: QButton( parent, name )
{
    init();
    setText( text );
}

void QPushButton::init()
{
    initMetaObject();
    autoDefButton = defButton = lastDown = lastDef = FALSE;
}


/*----------------------------------------------------------------------------
  \fn bool QPushButton::autoDefault() const
  Returns TRUE if the button is an auto-default button.

  \sa setAutoDefault()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the push buttons to an auto-default button if \e enable is TRUE,
  or to a normal button if \e enable is FALSE.

  An auto-default button becomes the default push button automatically
  when it receives the keyboard input focus.

  \sa autoDefault(), setDefault()
 ----------------------------------------------------------------------------*/

void QPushButton::setAutoDefault( bool enable )
{
    autoDefButton = enable;
}


/*----------------------------------------------------------------------------
  \fn bool QPushButton::isDefault() const
  Returns TRUE if the button is default.

  \sa setDefault()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the button to be the default button if \e enable is TRUE, or
  to be a normal button if \e enable is FALSE.

  A default push button in a dialog (QDialog) emits the QButton::clicked()
  signal if the user presses the Enter key.  Only one push button in the
  dialog can be default.

  Default push buttons are only allowed in dialogs.

  \sa default(), setAutoDefault()
 ----------------------------------------------------------------------------*/

void QPushButton::setDefault( bool enable )
{
    if ( (defButton && enable) || !(defButton || enable) )
	return;					// no change
    QWidget *p = this;
    while ( p && p->parentWidget() )		// get the top level parent
	p = p->parentWidget();
    if ( !p->inherits("QDialog") )		// not a dialog
	return;
    defButton = enable;
    if ( defButton )
	((QDialog*)p)->setDefault( this );
    int gs = style();
    if ( gs != MacStyle && gs != MotifStyle ) {
	if ( isVisible() )
	    repaint( FALSE );
    }
    else
	resizeDefButton( (QPushButton*)this );
}


/*----------------------------------------------------------------------------
  Adjusts the size of the push button to fit the contents.

  This function is called automatically whenever the contents change and
  auto-resizing is enabled.

  \sa setAutoResize()
 ----------------------------------------------------------------------------*/

void QPushButton::adjustSize()
{
    int w, h;
    if ( pixmap() ) {
	QPixmap *pm = (QPixmap *)pixmap();
	w = pm->width()  + 6;
	h = pm->height() + 6;
    }
    else {
	QFontMetrics fm = fontMetrics();
	QRect br = fm.boundingRect( text() );
	w = br.width()	+ 6;
	h = br.height() + 6;
	w += w/8 + 16;
	h += h/8 + 4;
    }
    resize( w, h );
}


/*----------------------------------------------------------------------------
  Reimplements QWidget::move().
 ----------------------------------------------------------------------------*/

void QPushButton::move( int x, int y )
{
    int wx, hx;
    extraSize( this, wx, hx, TRUE );
    QWidget::move( x-wx/2, y-hx/2 );
}

/*----------------------------------------------------------------------------
  Reimplements QWidget::move().
 ----------------------------------------------------------------------------*/

void QPushButton::move( const QPoint &p )
{
    move( p.x(), p.y() );
}

/*----------------------------------------------------------------------------
  Reimplements QWidget::resize().
 ----------------------------------------------------------------------------*/

void QPushButton::resize( int w, int h )
{
    int wx, hx;
    extraSize( this, wx, hx, TRUE );
    QWidget::resize( w+wx, h+hx );
}

/*----------------------------------------------------------------------------
  Reimplements QWidget::resize().
 ----------------------------------------------------------------------------*/

void QPushButton::resize( const QSize &s )
{
    resize( s.width(), s.height() );
}

/*----------------------------------------------------------------------------
  Reimplements QWidget::setGeometry().
 ----------------------------------------------------------------------------*/

void QPushButton::setGeometry( int x, int y, int w, int h )
{
    int wx, hx;
    extraSize( this, wx, hx, TRUE );
    QWidget::setGeometry( x-wx/2, y-hx/2, w+wx, h+hx );
}

/*----------------------------------------------------------------------------
  Reimplements QWidget::setGeometry().
 ----------------------------------------------------------------------------*/

void QPushButton::setGeometry( const QRect &r )
{
    setGeometry( r.x(), r.y(), r.width(), r.height() );
}


/*----------------------------------------------------------------------------
  Draws the pust button, but not the button label.
  \sa drawButtonLabel()
 ----------------------------------------------------------------------------*/

void QPushButton::drawButton( QPainter *paint )
{
    register QPainter *p = paint;
    GUIStyle	gs = style();
    QColorGroup g  = colorGroup();
    bool	updated = isDown() != (bool)lastDown || lastDef != defButton;
    QColor	fillcol = g.background();
    int		x1, y1, x2, y2;

    rect().coords( &x1, &y1, &x2, &y2 );	// get coordinates

#define SAVE_PUSHBUTTON_PIXMAPS
#if defined(SAVE_PUSHBUTTON_PIXMAPS)
    QString pmkey;				// pixmap key
    int w, h;
    w = x2 + 1;
    h = y2 + 1;
    pmkey.sprintf( "$qt_push_%d_%d_%d_%d_%d_%d", gs, palette().serialNumber(),
		   isDown(), defButton, w, h );
    QPixmap *pm = QPixmapCache::find( pmkey );
    QPainter pmpaint;
    if ( pm ) {					// pixmap exists
	QPixmap pm_direct = *pm;
	pmpaint.begin( &pm_direct );
	pmpaint.drawPixmap( 0, 0, *pm );
	if ( text() )
	    pmpaint.setFont( font() );
	drawButtonLabel( &pmpaint );
	pmpaint.end();
	p->drawPixmap( 0, 0, pm_direct );
	lastDown = isDown();
	lastDef = defButton;
	return;
    }
    bool use_pm = TRUE;
    if ( use_pm ) {
	pm = new QPixmap( w, h );		// create new pixmap
	CHECK_PTR( pm );
	pmpaint.begin( pm );
	p = &pmpaint;				// draw in pixmap
	p->setBackgroundColor( fillcol );
	p->eraseRect( 0, 0, w, h );
    }
#endif

    p->setPen( g.foreground() );
    p->setBrush( QBrush(fillcol,NoBrush) );
    if ( gs == MacStyle ) {			// Macintosh push button
	if ( defButton ) {
	    p->setPen( QPen(g.foreground(), 3) );
	    x1++; y1++; x2--; y2--;
	    p->drawRoundRect( x1, y1, x2-x1+1, y2-y1+1, 25, 25 );
	    x1 += extraMacWidth/2;
	    y1 += extraMacHeight/2;
	    x2 -= extraMacWidth/2;
	    y2 -= extraMacHeight/2;
	    p->setPen( g.foreground() );
	}
	if ( updated ) {			// fill
	    if ( isDown() )
		p->setBrush( g.foreground() );
	    else
		p->setBrush( fillcol );
	}
	p->drawRoundRect( x1, y1, x2-x1+1, y2-y1+1, 20, 20 );
    }
    else if ( gs == WindowsStyle ) {		// Windows push button
	QPointArray a;
	if ( isDown() ) {
	    if ( defButton ) {
		p->setPen( black );
		p->drawRect( x1, y1, x2-x1+1, y2-y1+1 );
		p->setPen( g.dark() );
		p->drawRect( x1+1, y1+1, x2-x1-1, y2-y1-1 );
	    }
	    else {
		a.setPoints( 3, x1,y2-1, x1,y1, x2-1,y1 );
		p->setPen( g.dark() );
		p->drawPolyline( a );
		a.setPoints( 3, x1+1,y2-2, x1+1,y1+1, x2-2,y1+1 );
		p->setPen( black );
		p->drawPolyline( a );
		a.setPoints( 3, x1,y2, x2,y2, x2,y1 );
		p->setPen( g.light() );
		p->drawPolyline( a );
	    }
	}
	else {
	    if ( defButton ) {
		p->setPen( black );
		p->drawRect( x1, y1, w, h );
		x1++; y1++;
		x2--; y2--;
	    }
	    a.setPoints( 3, x1,y2-1, x1,y1, x2-1,y1 );
	    p->setPen( g.light() );
	    p->drawPolyline( a );
	    a.setPoints( 3, x1+1,y2-2, x1+1,y1+1, x2-2,y1+1 );
	    p->setPen( g.background() );
	    p->drawPolyline( a );
	    a.setPoints( 3, x1+1,y2-1, x2-1,y2-1, x2-1,y1+1 );
	    p->setPen( g.dark() );
	    p->drawPolyline( a );
	    a.setPoints( 3, x1,y2, x2,y2, x2,y1 );
	    p->setPen( black );
	    p->drawPolyline( a );
	}
	if ( updated )
	    p->fillRect( x1+2, y1+2, x2-x1-3, y2-y1-3, g.background() );
    }
    else if ( gs == Win3Style ) {		// Windows 3.x push button
	QPointArray a;
	a.setPoints( 8, x1+1,y1, x2-1,y1, x1+1,y2, x2-1,y2,
			x1,y1+1, x1,y2-1, x2,y1+1, x2,y2-1 );
	p->drawLineSegments( a );		// draw frame
	x1++; y1++;
	x2--; y2--;
	if ( defButton || (autoDefButton & isDown()) ) {
	    p->drawRect( x1, y1, x2-x1+1, y2-y1+1 );
	    x1++; y1++;
	    x2--; y2--;
	}
	if ( isDown() )
	    p->drawShadePanel( x1, y1, x2-x1+1, y2-y1+1, g.dark(), g.light(),
			       1, fillcol, updated );
	else
	    p->drawShadePanel( x1, y1, x2-x1+1, y2-y1+1, g.light(), g.dark(),
			       2, fillcol, updated );
    }
    else if ( gs == PMStyle ) {			// PM push button
	p->setPen( g.dark() );
	if ( updated )				// fill
	    p->setBrush( fillcol );
	p->drawRect( x1, y1, x2-x1+1, y2-y1+1 );
	if ( !defButton ) {
	    p->setPen( g.background() );
	    p->drawPoint( x1, y1 );
	    p->drawPoint( x1, y2 );
	    p->drawPoint( x2, y1 );
	    p->drawPoint( x2, y2 );
	    p->setPen( g.dark() );
	}
	x1++; y1++;
	x2--; y2--;
	QPointArray atop, abottom;
	atop.setPoints( 3, x1,y2-1, x1,y1, x2,y1 );
	abottom.setPoints( 3, x1,y2, x2,y2, x2,y1+1 );
	QColor tc, bc;
	if ( isDown() ) {
	    tc = g.dark();
	    bc = g.light();
	}
	else {
	    tc = g.light();
	    bc = g.dark();
	}
	p->setPen( tc );
	p->drawPolyline( atop );
	p->setPen( bc );
	p->drawPolyline( abottom );
    }
    else if ( gs == MotifStyle ) {		// Motif push button
	QColor tColor, bColor;
	if ( defButton ) {			// default Motif button
	    p->drawShadePanel( x1, y1, x2-x1+1, y2-y1+1, g.dark(), g.light() );
	    x1 += extraMotifWidth/2;
	    y1 += extraMotifHeight/2;
	    x2 -= extraMotifWidth/2;
	    y2 -= extraMotifHeight/2;
	}
	if ( isDown() ) {
	    tColor  = g.dark();
	    bColor  = g.light();
	    fillcol = g.mid();
	}
	else {
	    tColor = g.light();
	    bColor = g.dark();
	}
	p->drawShadePanel( x1, y1, x2-x1+1, y2-y1+1, tColor, bColor,
			   2, fillcol, updated );
    }
    if ( p->brush().style() != NoBrush )
	p->setBrush( NoBrush );

#if defined(SAVE_PUSHBUTTON_PIXMAPS)
    if ( use_pm ) {
	pmpaint.end();
	p = paint;				// draw in default device
	p->drawPixmap( 0, 0, *pm );
	QPixmapCache::insert( pmkey, pm );	// save for later use
    }
#endif
    drawButtonLabel( p );
    lastDown = isDown();
    lastDef = defButton;
}


/*----------------------------------------------------------------------------
  Draws the push button label.
  \sa drawButton()
 ----------------------------------------------------------------------------*/

void QPushButton::drawButtonLabel( QPainter *paint )
{
    register QPainter *p = paint;
    GUIStyle	gs = style();
    QColorGroup g  = colorGroup();
    int		dt = 0;
    switch ( gs ) {
	case MacStyle:
	    p->setPen( isDown() ? white : g.text() );
	    break;
	case Win3Style:
	    dt = 1;
	case WindowsStyle:
	    dt++;
	case PMStyle:
	case MotifStyle:
	    p->setPen( g.text() );
	    break;
    }
    QRect r = rect();
    int x, y, w, h;
    r.rect( &x, &y, &w, &h );
    if ( isDown() || isOn() ) {			// shift pixmap/text
	x += dt;
	y += dt;
    }
    x += 2;  y += 2;  w -= 4;  h -= 4;
    if ( pixmap() ) {
	QPixmap *pm = (QPixmap *)pixmap();	
	if ( pm->width() > w || pm->height() > h )
	    p->setClipRect( x, y, w, h );
	if ( pm->depth() == 1 && isUp() )
	    p->setBackgroundMode( OpaqueMode );
	x += w/2 - pm->width()/2;		// center
	y += h/2 - pm->height()/2;
	p->drawPixmap( x, y, *pm );
	p->setClipping( FALSE );
    }
    else if ( text() )
	p->drawText( x, y, w, h, AlignCenter|ShowPrefix, text() );
}

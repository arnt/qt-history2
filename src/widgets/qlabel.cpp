/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlabel.cpp#33 $
**
** Implementation of QLabel widget class
**
** Author  : Eirik Eng
** Created : 941215
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#include "qlabel.h"
#include "qpixmap.h"
#include "qpainter.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qlabel.cpp#33 $")


/*----------------------------------------------------------------------------
  \class QLabel qlabel.h
  \brief The QLabel widget displays a static text or pixmap.

  \ingroup realwidgets

  A label is a text or pixmap field that can have an optional frame
  (since QLabel inherits QFrame).

  The contents of a label can be specified as a normal text, as a
  numeric value (which is internally converted to a text) or, as a
  pixmap.

  A label can be aligned in many different ways. The alignment setting
  specifies where to position the contents relative to the frame
  rectangle. See setAlignment() for a description of the alignment flags.

  Enabling auto-resizing will make a label resize itself whenever the
  contents change.  The top left corner is kept unchanged.

  Example of use:
  \code
    QLabel *label = new QLabel;
    label->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    label->setText( "first line\nsecond line" );
    label->setAlignment( AlignBottom | AlignRight );
  \endcode
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Constructs an empty label which is left-aligned, vertically centered and
  without automatic resizing.

  The \e parent and \e name arguments are passed to the QWidget constructor.

  See QFrame for details about the label's frame.
 ----------------------------------------------------------------------------*/

QLabel::QLabel( QWidget *parent, const char *name )
	: QFrame( parent, name )
{
    initMetaObject();
    lpixmap    = 0;
    align      = AlignLeft | AlignVCenter | ExpandTabs;
    autoresize = FALSE;
}

/*----------------------------------------------------------------------------
  Constructs a label with a text. The label is left-aligned, vertically
  centered and without automatic resizing.

  The \e parent and \e name arguments are passed to the QWidget constructor.

  See QFrame for details about the label's frame.
 ----------------------------------------------------------------------------*/

QLabel::QLabel( const char *text, QWidget *parent, const char *name )
	: QFrame( parent, name ), ltext(text)
{
    initMetaObject();
    lpixmap    = 0;
    align      = AlignLeft | AlignVCenter | ExpandTabs;
    autoresize = FALSE;
}

/*----------------------------------------------------------------------------
  Destroys the label.
 ----------------------------------------------------------------------------*/

QLabel::~QLabel()
{
    if ( lpixmap )
	delete lpixmap;
}


/*----------------------------------------------------------------------------
  \fn const char *QLabel::text() const
  Returns the label text.
  \sa setText()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the label contents to \e text and redraws the contents.

  The label resizes itself if auto-resizing is enabled.  Nothing
  happens if \e text is the same as the current label.

  \sa text(), setPixmap(), setAutoResize()
 ----------------------------------------------------------------------------*/

void QLabel::setText( const char *text )
{
    if ( ltext == text )
	return;
    ltext = text;
    if ( lpixmap ) {
	delete lpixmap;
	lpixmap = 0;
    }
    if ( autoresize )
	adjustSize();
    else
	updateLabel();
}

/*----------------------------------------------------------------------------
  \fn QPixmap *QLabel::pixmap() const
  Returns the label pixmap.
  \sa setPixmap()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the label contents to \e pixmap and redraws the contents.

  The label resizes itself if auto-resizing is enabled.  Nothing
  happens if \e pixmap is the same as the current label.

  \sa pixmap(), setText(), setAutoResize()
 ----------------------------------------------------------------------------*/

void QLabel::setPixmap( const QPixmap &pixmap )
{
    int w, h;
    if ( lpixmap ) {
	w = lpixmap->width();
	h = lpixmap->height();
    }
    else {
	lpixmap = new QPixmap;
	w = h = -1;
    }
    *lpixmap = pixmap;
    if ( !ltext.isNull() )
	ltext.resize( 0 );
    if ( autoresize && (w != lpixmap->width() || h != lpixmap->height()) )
	adjustSize();
    else {
	if ( w >= 0 && w <= lpixmap->width() && h <= lpixmap->height() ) {
	    QPainter paint;
	    paint.begin( this );
	    drawContents( &paint );		// don't erase contentsRect()
	    paint.end();
	}
	else
	    updateLabel();
    }
}


/*----------------------------------------------------------------------------
  Sets the label contents to \e num (converts it to text) and redraws the
  contents.

  The label resizes itself if auto-resizing is enabled.  Nothing
  happens if \e num reads the same as the current label.

  \sa setAutoResize()
 ----------------------------------------------------------------------------*/

void QLabel::setNum( int num )
{
    QString str;
    str.setNum( num );
    if ( str != ltext ) {
	ltext = str;
	if ( autoresize )
	    adjustSize();
	else
	    updateLabel();
    }
}

/*----------------------------------------------------------------------------
  Sets the label contents to \e num (converts it to text) and redraws the
  contents.

  The label resizes itself if auto-resizing is enabled.

  \sa setAutoResize()
 ----------------------------------------------------------------------------*/

void QLabel::setNum( double num )
{
    QString str;
    str.sprintf( "%g", num );
    if ( str != ltext ) {
	ltext = str;
	if ( autoresize )
	    adjustSize();
	else
	    updateLabel();
    }
}


/*----------------------------------------------------------------------------
  \fn int QLabel::alignment() const
  Returns the alignment setting.

  The default alignment is <code>AlignLeft | AlignVCenter | ExpandTabs</code>.

  \sa setAlignment()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the alignment of the label contents and redraws itself.

  The \e alignment is the bitwise OR of the following flags:
  <ul>
  <li> \c AlignLeft aligns to the left border.
  <li> \c AlignRight aligns to the right border.
  <li> \c AlignHCenter aligns horizontally centered.
  <li> \c AlignTop aligns to the top border.
  <li> \c AlignBottom aligns to the bottom border.
  <li> \c AlignVCenter aligns vertically centered
  <li> \c AlignCenter (= \c AlignHCenter | AlignVCenter)
  <li> \c ExpandTabs expands tabulators.
  <li> \c WordBreak enables automatic word breaking.
  </ul>

  \sa alignment()
 ----------------------------------------------------------------------------*/

void QLabel::setAlignment( int alignment )
{
    align = alignment;
    updateLabel();
}


/*----------------------------------------------------------------------------
  \fn bool QLabel::autoResize() const
  Returns TRUE if auto-resizing is enabled, or FALSE if auto-resizing is
  disabled.

  Auto-resizing is disabled by default.

  \sa setAutoResize()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------  
  Enables auto-resizing if \e enable is TRUE, or disables it if \e
  enable is FALSE.

  When auto-resizing is enabled, the label will resize itself whenever the
  contents change.  The top left corner is not moved.

  \sa autoResize(), adjustSize()
 ----------------------------------------------------------------------------*/

void QLabel::setAutoResize( bool enable )
{
    if ( autoresize != enable ) {
	autoresize = enable;
	if ( autoresize )
	    adjustSize();			// calls resize which repaints
    }
}


/*!
  Adjusts the size of the label to fit the contents.  The top left
  corner is not moved.

  This function is called automatically whenever the contents change and
  auto-resizing is enabled.

  \bug Does not work well with the WordBreak flag

  \sa setAutoResize()

  */

void QLabel::adjustSize()
{
    QPainter p;
    p.begin( this );
    QRect br = p.boundingRect( 0,0, 1000,1000, alignment(), text() );
    p.end();
    int w = br.width()	+ 4 + frameWidth();
    int h = br.height() + 4 + frameWidth();
    if ( w == width() && h == height() )
	updateLabel();
    else
	resize( w, h );
}


/*! \fn int QLabel::margin()

  Returns the margin of the label in pixels.

  The margin applies to the left edge if alignment() is \c AlignLeft,
  to the right edge if alignment() is \c AlignRight, to the top edge
  if alignment() is \c AlignTop, and to to the bottom edge if
  alignment() is \c AlignBottom.

  \sa setMargin() */


/*! Sets the margin of the label to \e pixels.

  The margin applies to the left edge if alignment() is \c AlignLeft,
  to the right edge if alignment() is \c AlignRight, to the top edge
  if alignment() is \c AlignTop, and to to the bottom edge if
  alignment() is \c AlignBottom.

  \sa margin() */

void QLabel::setMargin( int pixels ) {
    m = pixels >= 0 ? pixels : 0;
}


/*----------------------------------------------------------------------------
  Draws the label contents using the painter \e p.
 ----------------------------------------------------------------------------*/

void QLabel::drawContents( QPainter *p )
{
    p->setPen( colorGroup().text() );
    QRect cr = contentsRect();
    if ( align & AlignLeft )
	cr.setLeft( cr.left() + m );
    if ( align & AlignRight )
	cr.setRight( cr.right() - m );
    if ( align & AlignTop )
	cr.setTop( cr.top() + m );
    if ( align & AlignBottom )
	cr.setBottom( cr.bottom() - m );
    if ( lpixmap ) {
	int fw = frameWidth();
	int x, y, w, h;
	cr.rect( &x, &y, &w, &h );
	int pmw=lpixmap->width(), pmh=lpixmap->height();
	if ( fw > 0 && (pmw > w || pmh > h) )
	     p->setClipRect( x, y, w, h );
	if ( (align & AlignRight) == AlignRight )
	    x += w - pmw;
	else if ( (align & AlignHCenter) == AlignHCenter )
	    x += w/2 - pmw/2;
	if ( (align & AlignBottom) == AlignBottom )
	    y += h - pmh;
	else if ( (align & AlignVCenter) == AlignVCenter )
	    y += h/2 - pmh/2;
	p->drawPixmap( x, y, *lpixmap );
	if ( fw > 0 )
	    p->setClipping( FALSE );
    }
    else
	p->drawText( cr, align, ltext );
}


/*----------------------------------------------------------------------------
  Updates the label, not the frame.
 ----------------------------------------------------------------------------*/

void QLabel::updateLabel()
{
    QPainter paint;
    paint.begin( this );
    paint.eraseRect( contentsRect() );
    drawContents( &paint );
    paint.end();
}

/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlabel.cpp#43 $
**
** Implementation of QLabel widget class
**
** Created : 941215
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#include "qlabel.h"
#include "qpixmap.h"
#include "qpainter.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qlabel.cpp#43 $");


/*!
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
*/

/*!
  Constructs an empty label which is left-aligned, vertically centered,
  has an automatic margin and with manual resizing.

  The \e parent, \e name and \e f arguments are passed to the QFrame
  constructor.

  \sa setAlignment(), setFrameStyle(), setMargin(), setAutoResize()
*/

QLabel::QLabel( QWidget *parent, const char *name, WFlags f )
    : QFrame( parent, name, f )
{
    initMetaObject();
    lpixmap    = 0;
    align      = AlignLeft | AlignVCenter | ExpandTabs;
    extraMargin= -1;
    autoresize = FALSE;
}

/*!
  Constructs a label with a text. The label is left-aligned, vertically
  centered, has an automatic margin and with manual resizing.

  The \e parent, \e name and \e f arguments are passed to the QFrame
  constructor.

  \sa setAlignment(), setFrameStyle(), setMargin(), setAutoResize()
*/

QLabel::QLabel( const char *text, QWidget *parent, const char *name, WFlags f )
	: QFrame( parent, name, f ), ltext(text)
{
    initMetaObject();
    lpixmap    = 0;
    align      = AlignLeft | AlignVCenter | ExpandTabs;
    extraMargin= -1;
    autoresize = FALSE;
}

/*!
  Destroys the label.
*/

QLabel::~QLabel()
{
    if ( lpixmap )
	delete lpixmap;
}


/*!
  \fn const char *QLabel::text() const
  Returns the label text.
  \sa setText()
*/

/*!
  Sets the label contents to \e text and redraws the contents.

  The label resizes itself if auto-resizing is enabled.	 Nothing
  happens if \e text is the same as the current label.

  \sa text(), setPixmap(), setAutoResize()
*/

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

/*!
  \fn QPixmap *QLabel::pixmap() const
  Returns the label pixmap.
  \sa setPixmap()
*/

/*!
  Sets the label contents to \e pixmap and redraws the contents.

  The label resizes itself if auto-resizing is enabled.	 Nothing
  happens if \e pixmap is the same as the current label.

  \sa pixmap(), setText(), setAutoResize()
*/

void QLabel::setPixmap( const QPixmap &pixmap )
{
    int w, h;
    if ( lpixmap ) {
	w = lpixmap->width();
	h = lpixmap->height();
    } else {
	lpixmap = new QPixmap;
	w = h = -1;
    }
    *lpixmap = pixmap;
    if ( !ltext.isNull() )
	ltext.resize( 0 );
    if ( autoresize && (w != lpixmap->width() || h != lpixmap->height()) ) {
	adjustSize();
    } else {
	if ( w >= 0 && w <= lpixmap->width() && h <= lpixmap->height() ) {
	    QPainter paint;
	    paint.begin( this );
	    drawContents( &paint );		// don't erase contentsRect()
	    paint.end();
	} else {
	    updateLabel();
	}
    }
}


/*!
  Sets the label contents to \e num (converts it to text) and redraws the
  contents.

  The label resizes itself if auto-resizing is enabled.	 Nothing
  happens if \e num reads the same as the current label.

  \sa setAutoResize()
*/

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

/*!
  Sets the label contents to \e num (converts it to text) and redraws the
  contents.

  The label resizes itself if auto-resizing is enabled.

  \sa setAutoResize()
*/

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


/*!
  \fn int QLabel::alignment() const
  Returns the alignment setting.

  The default alignment is <code>AlignLeft | AlignVCenter | ExpandTabs</code>.

  \sa setAlignment()
*/

/*!
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
*/

void QLabel::setAlignment( int alignment )
{
    align = alignment;
    updateLabel();
}


/*!
  \fn int QLabel::margin() const

  Returns the margin of the label.

  \sa setMargin()
*/

/*!
  Sets the margin of the label to \e margin pixels.

  The margin applies to the left edge if alignment() is \c AlignLeft,
  to the right edge if alignment() is \c AlignRight, to the top edge
  if alignment() is \c AlignTop, and to to the bottom edge if
  alignment() is \c AlignBottom.

  If \e margin is negative (as it is by default), the label computes the
  margin as follows: If the \link frameWidth() frame width\endlink is zero,
  the effective margin becomes 0. If the frame style is greater than zero,
  the effective margin becomes half the width of the "x" character (of the
  widget's current \link font() font\endlink.

  Setting a non-negative margin gives the specified margin in pixels.

  \sa margin(), frameWidth(), font()
*/

void QLabel::setMargin( int margin )
{
    extraMargin = margin;
}


/*!
  \fn bool QLabel::autoResize() const
  Returns TRUE if auto-resizing is enabled, or FALSE if auto-resizing is
  disabled.

  Auto-resizing is disabled by default.

  \sa setAutoResize()
*/

/*!
  Enables auto-resizing if \e enable is TRUE, or disables it if \e
  enable is FALSE.

  When auto-resizing is enabled, the label will resize itself whenever the
  contents change.  The top left corner is not moved.

  \sa autoResize(), adjustSize()
*/

void QLabel::setAutoResize( bool enable )
{
    if ( autoresize != enable ) {
	autoresize = enable;
	if ( autoresize )
	    adjustSize();			// calls resize which repaints
    }
}

/*!
  Returns a size which fits the contents of the label.

  \bug Does not work well with the WordBreak flag
*/
QSize QLabel::sizeHint() const
{
    QPainter p;
    p.begin( this );
    QRect br = p.boundingRect( 0,0, 1000,1000, alignment(), text() );
    int m  = 2*margin();
    int fw = frameWidth();
    if ( m < 0 ) {
	if ( fw > 0 )
	    m = p.fontMetrics().width( "x" );
	else
	    m = 0;
    }
    int w = br.width()	+ m + 2*fw;
    int h = br.height() + m + 2*fw;
    p.end();

    return QSize( w, h );
}


/*!
  Draws the label contents using the painter \e p.
*/

void QLabel::drawContents( QPainter *p )
{
    p->setPen( colorGroup().text() );
    QRect cr = contentsRect();
    int fw = frameWidth();
    int m  = margin();
    if ( m < 0 ) {
	if ( fw > 0 )
	    m = p->fontMetrics().width("x")/2;
	else
	    m = 0;
    }
    if ( align & AlignLeft )
	cr.setLeft( cr.left() + m );
    if ( align & AlignRight )
	cr.setRight( cr.right() - m );
    if ( align & AlignTop )
	cr.setTop( cr.top() + m );
    if ( align & AlignBottom )
	cr.setBottom( cr.bottom() - m );
    if ( lpixmap ) {
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


/*!
  Updates the label, not the frame.
*/

void QLabel::updateLabel()
{
    QPainter paint;
    paint.begin( this );
    paint.eraseRect( contentsRect() );
    drawContents( &paint );
    paint.end();
}

/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlabel.cpp#16 $
**
** Implementation of QLabel widget class
**
** Author  : Eirik Eng
** Created : 941215
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#include "qlabel.h"
#include "qpainter.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qlabel.cpp#16 $";
#endif

/*! \class QLabel qlabel.h

  \brief The QLabel class can display a string in a fixed location.

  It is intended for e.g. window or dialog box titles/headlines.
  The label can be aligned in many different ways and its value
  specified from either a text of any of several numeric types. */

/*! This constructor creates a new label which is left-aligned,
  vertically centered and without automatic resizing.  The \e parent
  and \e name arguments are passed to the QWidget constructor. \sa
  setAlignment(), setAutoResizing(), setLabel().*/
QLabel::QLabel( QWidget *parent, const char *name )
	: QFrame( parent, name )
{
    initMetaObject();
    align      = AlignLeft | AlignVCenter | ExpandTabs;
    autoResize = FALSE;
}

/*! This constructor creates a new label which is left-aligned,
  vertically centered and without resizing.  It will contain the text
  \e label.  The \e parent and \e name arguments are passed to the
  QWidget construcor.  \sa setSlignment(), setAutoResizing(),
  setLabel(). */
QLabel::QLabel( const char *label, QWidget *parent, const char *name )
	: QFrame( parent, name ), str(label)
{
    initMetaObject();
    align      = AlignLeft | AlignVCenter | ExpandTabs;
    autoResize = FALSE;
}

/*! \fn const char *QLabel::label() const
  Returns a pointer to the contents of the label. */

/*! The label sets it contents to \e s, adjusts its position and size
  if it's supposed to, and repaints itself. \sa setAutoResize() */

void QLabel::setLabel( const char *s )
{
    if ( str == s )				// no change
	return;
    str = s;
    if ( autoResize )
        adjustSize();
    updateLabel();
}

/*! The label sets it contents to \e l, adjusts its position and size
  if it's supposed to, and repaints itself. \todo Add optional
  arguments to specify conversion format. \sa setAutoResize(). */

void QLabel::setLabel( long l )
{
    QString tmp;
    tmp.sprintf( "%ld", l );
    if ( tmp != str ) {
	str = tmp;
        if ( autoResize )
            adjustSize();
	updateLabel();
    }
}

/*! The label sets it contents to \e s, adjusts its position and size
  if it's supposed to, and repaints itself. \todo Add optional
  arguments to specify conversion format. \sa setAutoResize(). */

void QLabel::setLabel( double d )
{
    QString tmp;
    tmp.sprintf( "%g", d );
    if ( tmp != str ) {
	str = tmp;
        if ( autoResize )
  	    adjustSize();
	updateLabel();
    }
}

/*! \fn void QLabel::setLabel( int   i )

  The label sets it contents to \e o, adjusts its position and size
  if it's supposed to, and repaints itself. \todo Add optional
  arguments to specify conversion format. \sa setAutoResize(). */

/*! \fn void QLabel::setLabel( float i )

  The label sets it contents to \e f, adjusts its position and size
  if it's supposed to, and repaints itself. \todo Add optional
  arguments to specify conversion format. \sa setAutoResize(). */

/*! \fn int QLabel::alignment() const

  Returns the label's alignment.  The default alignment is
  <code>AlignLeft|AlignVCenter|ExpandTabs</code>. \sa
  setAlignment(). */

/*! The label's alignment is changed to \e alignment and its
  on-screen appearance changed at once.

  The \e alignment is the bitwise OR of <code>AlignLeft, AlignRight,
  AlignCenter, AlignHCenter, AlignTop, AlignBottom, AlignVCenter,
  SingleLine, DontClip, ExpandTabs, ShowPrefix, WordBreak,
  GrayText</code> and <code>DontPrint.</code>  QPainter::drawText()
  documents these in more detail, and qwindefs.h defines them. */

void QLabel::setAlignment( int alignment )
{
    align = alignment;
    updateLabel();
}

/*! \fn bool QLabel::autoResizing() const

  Returns TRUE if the label will resize itself automatically when its
  contens are changed.

  It returns, I'm sure this will surprise you, FALSE if not.

  \sa setAutoResizing(). */

/*! Enables or disables automatic size adjustment of the label.  If
  size adjustment is enabled, the label will repaint itself very
  prettily whenever its contents change. \sa setLabel(). */
void QLabel::setAutoResizing( bool enable )
{
    if ( autoResize != enable ) {
	autoResize = enable;
	if ( autoResize )
	    adjustSize();			// calls resize which repaints
    }
}

/*! Adjusts the size of the label.  If you've called
  setAutoResizing(TRUE) you'll never need to think about this. \sa
  setAutoResizing(). */
void QLabel::adjustSize()
{
    QFontMetrics fm( font() );
    resize( fm.width( str ) + 4 + frameWidth() + midLineWidth(),
	    fm.height()     + 4 + frameWidth() + midLineWidth() );
}


void QLabel::drawContents( QPainter *p )
{
    p->setPen( colorGroup().text() );
    p->drawText( contentsRect(), align, str );
}

void QLabel::updateLabel()			// update label, not frame
{
    QPainter paint;
    paint.begin( this );
    paint.eraseRect( contentsRect() );
    drawContents( &paint );
    paint.end();
}

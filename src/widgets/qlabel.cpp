/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlabel.cpp#21 $
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
static char ident[] = "$Id: //depot/qt/main/src/widgets/qlabel.cpp#21 $";
#endif


/*!
\class QLabel qlabel.h

\brief The QLabel widget displays a static text.

\ingroup realwidgets

A label is a text field that can have an optional frame (since QLabel
inherits QFrame).

The contents of a label can be specified as a normal text or as a
numeric value (which is internally converted to a text).

A label can be aligned in many different ways.	The alignment setting
specifies where to position the contents relative to the frame
rectangle. See setAlignment() for a description of the alignment flags.

Enabling auto-resizing will make a label resize itself whenever
the contents change.

Example of use:
\code
  QLabel *label = new QLabel;
  label->setFrame( QFrame::Panel | QFrame::Sunken );
  label->setText( "first line\nsecond line" );
  label->setAlignment( AlignBottom | AlignRight );
\endcode
*/

/*!
Constructs an empty label which is left-aligned, vertically centered and
without automatic resizing.

The \e parent and \e name arguments are passed to the QWidget constructor.
*/

QLabel::QLabel( QWidget *parent, const char *name )
	: QFrame( parent, name )
{
    initMetaObject();
    align      = AlignLeft | AlignVCenter | ExpandTabs;
    autoResize = FALSE;
}

/*!
Constructs a label with a text. The label is left-aligned, vertically
centered and without automatic resizing.

The \e parent and \e name arguments are passed to the QWidget constructor.
*/

QLabel::QLabel( const char *text, QWidget *parent, const char *name )
	: QFrame( parent, name ), str(text)
{
    initMetaObject();
    align      = AlignLeft | AlignVCenter | ExpandTabs;
    autoResize = FALSE;
}

/*!
\fn const char *QLabel::text() const
Returns the label text.
*/

/*!
Sets the label contents to \e text and redraws the contents.

The label resizes itself if auto-resizing is enabled.

\sa setAutoResize().
*/

void QLabel::setText( const char *text )
{
    if ( str == text )				// no change
	return;
    str = text;
    if ( autoResize )
	adjustSize();
    else
	updateLabel();
}

/*!
Sets the label contents to \e num (converts it to text) and redraws the
contents.

The label resizes itself if auto-resizing is enabled.

\sa setAutoResize().
*/

void QLabel::setNum( long num )
{
    QString tmp;
    tmp.sprintf( "%ld", num );
    if ( tmp != str ) {
	str = tmp;
	if ( autoResize )
	    adjustSize();
	else
	    updateLabel();
    }
}

/*!
Sets the label contents to \e num (converts it to text) and redraws the
contents.

The label resizes itself if auto-resizing is enabled.

\sa setAutoResize().
*/

void QLabel::setNum( double num )
{
    QString tmp;
    tmp.sprintf( "%g", num );
    if ( tmp != str ) {
	str = tmp;
	if ( autoResize )
	    adjustSize();
	else
	    updateLabel();
    }
}

/*!
\fn void QLabel::setNum( int num )
Sets the label contents to \e num (converts it to text) and redraws the
contents.

The label resizes itself if auto-resizing is enabled.

\sa setAutoResize().
*/

/*!
\fn void QLabel::setNum( float num )
Sets the label contents to \e num (converts it to text) and redraws the
contents.

The label resizes itself if auto-resizing is enabled.

\sa setAutoResize().
*/


/*!
\fn int QLabel::alignment() const
Returns the alignment setting.

The default alignment is <code>AlignLeft | AlignVCenter | ExpandTabs</code>.

\sa setAlignment().
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

\sa alignment().
*/

void QLabel::setAlignment( int alignment )
{
    align = alignment;
    updateLabel();
}


/*!
\fn bool QLabel::autoResizing() const
Returns TRUE if auto-resizing is enabled, or FALSE if auto-resizing is
disabled.

Auto-resizing is disabled by default.

\sa setAutoResizing().
*/

/*!
Enables auto-resizing if \e enable is TRUE, or disables it if \e enable is
FALSE.

When auto-resizing is enabled, the label will resize itself whenever
the contents change.

\sa autoResizing() and adjustSize().
*/

void QLabel::setAutoResizing( bool enable )
{
    if ( autoResize != enable ) {
	autoResize = enable;
	if ( autoResize )
	    adjustSize();			// calls resize which repaints
    }
}


/*!
Adjusts the size of the label to fit the contents.

This function is called automatically whenever the contents change and
auto-resizing is enabled.

\sa setAutoResizing()
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


/*!
Draws the label contents using the painter \e p.
*/

void QLabel::drawContents( QPainter *p )
{
    p->setPen( colorGroup().text() );
    p->drawText( contentsRect(), align, str );
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

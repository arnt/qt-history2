/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qgroupbox.cpp#68 $
**
** Implementation of QGroupBox widget class
**
** Created : 950203
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

#include "qgroupbox.h"
#include "qlayout.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "qaccel.h"
#include "qradiobutton.h"
#include "qfocusdata.h"
#include "qobjectlist.h"
#include "qdrawutil.h"

#ifdef QT_BUILDER
#include "qdom.h"
#endif

/*!
  \class QGroupBox qgroupbox.h
  \brief The QGroupBox widget provides a group box frame with a title.

  \ingroup realwidgets

  The intended use of a group box is to show that a number of widgets
  (i.e. child widgets) are logically related.  QPrintDialog is a good
  example; each of its three panes is one group box.

  By default, the group's setFont() and setPalette() functions do not
  change the appearance of the widgets it contains, but you can use
  setFontPropagation() and setPalettePropagation() to change that.

  Qt also provides a more specialized group box, QButtonGroup, that is
  very useful for organizing buttons in a group.

  <img src=qgrpbox-m.png> <img src=qgrpbox-w.png>
*/


/*!
  Constructs a group box widget with no title.

  The \e parent and \e name arguments are passed to the QWidget constructor.
*/

QGroupBox::QGroupBox( QWidget *parent, const char *name )
    : QFrame( parent, name )
{
    init();
}

/*!
  Constructs a group box with a title.

  The \e parent and \e name arguments are passed to the QWidget constructor.
*/

QGroupBox::QGroupBox( const QString &title, QWidget *parent, const char *name )
    : QFrame( parent, name )
{
    init();
    setTitle( title );
}

/*!
  Constructs a group box with no title. Child widgets will be arranged
  in \a strips rows or columns (depending on \a orientation).

  The \e parent and \e name arguments are passed to the QWidget constructor.
*/

QGroupBox::QGroupBox( int strips, Orientation orientation,
		    QWidget *parent, const char *name )
    : QFrame( parent, name )
{
    init();
    setColumnLayout( strips, orientation );
}

/*!
  Constructs a group box with a \a title. Child widgets will be arranged
  in \a strips rows or columns (depending on \a orientation).

  The \e parent and \e name arguments are passed to the QWidget constructor.
*/

QGroupBox::QGroupBox( int strips, Orientation orientation,
		    const QString &title, QWidget *parent,
		    const char *name )
    : QFrame( parent, name )
{
    init();
    setTitle( title );
    setColumnLayout( strips, orientation );
}

void QGroupBox::init()
{
    int fs;
    align = AlignLeft;
    fs = QFrame::Box | QFrame::Sunken;
    setFrameStyle( fs );
    accel = 0;
    vbox = 0;
    grid = 0;
    lenvisible = 0;
}


/*!
  Sets the group box title text to \a title, and add a focus-change
  accelerator if the \a title contains & followed by an appropriate
  letter.  This produces "User information" with the U underscored and
  Alt-U moves the keyboard focus into the group.

  \code
    g->setTitle( "&User information" );
  \endcode
*/

void QGroupBox::setTitle( const QString &title )
{
    if ( str == title )				// no change
	return;
    if ( accel )
	delete accel;
    accel = 0;
    str = title;
    int s = QAccel::shortcutKey( title );
    if ( s ) {
	accel = new QAccel( this, "automatic focus-change accelerator" );
	accel->connectItem( accel->insertItem( s, 0 ),
			    this, SLOT(fixFocus()) );
    }
    calculateFrame();
    if ( isVisible() )
	repaint();
}

/*!
  \fn QString QGroupBox::title() const
  Returns the group box title text.
*/

/*!
  \fn int QGroupBox::alignment() const
  Returns the alignment of the group box title.

  The default alignment is \c AlignLeft.

  \sa setAlignment()
*/

/*!
  Sets the alignment of the group box title.

  The title is always placed on the upper frame line, however,
  the horizontal alignment can be specified by the \e alignment parameter.

  The \e alignment is the bitwise OR of the following flags:
  <ul>
  <li> \c AlignLeft aligns the title text to the left.
  <li> \c AlignRight aligns the title text to the right.
  <li> \c AlignHCenter aligns the title text centered.
  </ul>

  \sa alignment()
*/

void QGroupBox::setAlignment( int alignment )
{
    align = alignment;
    repaint();
}

/*! \reimp
*/
void QGroupBox::resizeEvent( QResizeEvent *e )
{
    QFrame::resizeEvent(e);
    calculateFrame();
}

/*!
  Handles paint events for the group box.

  \internal
  overrides QFrame::paintEvent
*/

void QGroupBox::paintEvent( QPaintEvent *event )
{
    QPainter paint( this );

    if ( lenvisible ) {					// draw title
	QFontMetrics fm = paint.fontMetrics();
	int h = fm.height();
	int tw = fm.width( str, lenvisible ) + 2*fm.width(QChar(' '));
	int x;
	if ( align & AlignHCenter )		// center alignment
	    x = frameRect().width()/2 - tw/2;
	else if ( align & AlignRight )	// right alignment
	    x = frameRect().width() - tw - 8;
	else				// left alignment
	    x = 8;
	qDrawItem( &paint, style(), x, 0, tw, h, AlignCenter + ShowPrefix,
		   colorGroup(), isEnabled(), 0, str, lenvisible, 0 );
 	QRect r( x, 0, tw, h );
	paint.setClipRegion( event->region().subtract( r ) );	// clip everything but title
    }
    drawFrame( &paint );			// draw the frame
    drawContents( &paint );			// draw the contents
}


/*\reimp
 */
void QGroupBox::updateMask(){
    int		tw  = 0;
    QRect	cr  = rect();
    QRect	r   = cr;
    QRect t;
    int		len = str.length();
     QBitmap bm( size() );
     bm.fill( color0 );
     {
 	QPainter p( &bm, this );
	QFontMetrics fm = p.fontMetrics();
	int h = fm.height();
	while ( len ) {
	    tw = fm.width( str, len ) + 2*fm.width(QChar(' '));
	    if ( tw < cr.width() )
		break;
	    len--;
	}
	if ( len ) {
	    r.setTop( h/2 );			// frame rect should be
	    int x;
	    if ( align & AlignHCenter )		// center alignment
		x = r.width()/2 - tw/2;
	    else if ( align & AlignRight )	// right alignment
		x = r.width() - tw - 8;
	    else				// left alignment
		x = 8;
 	    t.setRect( x, 0, tw, h );
	}
 	p.fillRect( r, color1 );
	if ( tw ) {					// draw the title
	    p.setPen( color1 );
	    p.drawText( t, AlignCenter, str, len );
	}
     }

    setMask( bm );

}


/*!
  Changes the layout of the group box. This function is only useful in combination
  with the default constructor that does not take any layout information.
  This function will put all existing children in the new layout. Nevertheless is
  is not good programming style to call this function after children have been inserted.
 */
void QGroupBox::setColumnLayout(int columns, Orientation direction)
{
    if ( layout() )
      delete layout();
    vbox = 0;
    grid = 0;

    if ( columns == 0 )
      return;

    vbox = new QVBoxLayout( this, 8, 0 );

    // Torbens hack for the builder. I dont want to
    // have this QGridLayout. I want to make it on
    // my own.
    if ( columns == -1 )
    {
	dir = direction;
	nCols = -1;
	nRows = -1;
	return;
    }
    
    if ( !str.isEmpty() ) {
	//### we should have a changeable spacer item
	QFontMetrics fm = fontMetrics();
	vbox->addSpacing( fm.lineSpacing() );
    }
    dir = direction;
    if ( dir == Horizontal ) {
      nCols = columns;
      nRows = 1;
    } else {
      nCols = 1;
      nRows = columns;
    }
    grid = new QGridLayout( nRows, nCols, 5 );
    row = col = 0;

    vbox->addLayout( grid );

    // Add all children
    const QObjectList *list = children();
    if ( list )
    {
      QObjectListIt it( *list );
      for( ; it.current(); ++it )
	if ( it.current()->inherits( "QWidget" ) )
	  insertWid( (QWidget*)it.current() );
    }
}

void QGroupBox::childEvent( QChildEvent *c )
{
    // Similar to QGrid::childEvent()
    if ( !grid || !c->inserted() || !c->child()->isWidgetType() )
        return;
    QWidget *w = (QWidget*)c->child();
    insertWid( w );
}

void QGroupBox::insertWid( QWidget* _w )
{
    // Torbens hack for the builder. The builder does
    // not like the QGridLayout and associated magic.
    if ( nCols == -1 && nRows == -1 )
	return;
    
    if ( row >= nRows || col >= nCols )
        grid->expand( row+1, col+1 );
    grid->addWidget( _w, row, col );
    skip();
}


void QGroupBox::skip()
{
    // Same as QGrid::skip()
    if ( dir == Horizontal ) {
	if ( col+1 < nCols ) {
	    col++;
	} else {
	    col = 0;
	    row++;
	}
    } else { //Vertical
	if ( row+1 < nRows ) {
	    row++;
	} else {
	    row = 0;
	    col++;
	}
    }
}


/*!  This private slot finds a nice widget in this group box that can
accept focus, and gives it focus.
*/

void QGroupBox::fixFocus()
{
    QFocusData * fd = focusData();
    QWidget * orig = fd->focusWidget();
    QWidget * best = 0;
    QWidget * candidate = 0;
    QWidget * w = orig;
    do {
	QWidget * p = w;
	while( p && p != this && !p->isTopLevel() )
	    p = p->parentWidget();
	if ( p == this && ( w->focusPolicy() & TabFocus ) == TabFocus ) {
	    if ( w->hasFocus() ||
		 ( !best &&
		   w->inherits( "QRadioButton" ) &&
		   ((QRadioButton*)w)->isChecked() ) )
		// we prefer a checked radio button or a widget that
		// already has focus, if there is one
		best = w;
	    else if ( !candidate )
		// but we'll accept anything that takes focus
		candidate = w;
	}
	w = fd->next();
    } while( w != orig );
    if ( best )
	best->setFocus();
    else if ( candidate )
	candidate->setFocus();
}


/*!
  Sets the right framerect depending on the title. Also calculates the
  visible part of the title.
 */
void QGroupBox::calculateFrame()
{
    lenvisible = str.length();

    if ( lenvisible ) { // do we have a label?
	QFontMetrics fm = fontMetrics();
	int h = fm.height();
	while ( lenvisible ) {
	    int tw = fm.width( str, lenvisible ) + 2*fm.width(QChar(' '));
	    if ( tw < width() )
		break;
	    lenvisible--;
	}
	if ( lenvisible ) { // but do we also have a visible label?
	    QRect r = rect();
	    r.setTop( h/2 );				// frame rect should be
	    setFrameRect( r );			//   smaller than client rect
	    return;
	}
    }

    // no visible label
    setFrameRect( QRect(0,0,0,0) );		//  then use client rect
}

#ifdef QT_BUILDER
bool QGroupBox::setConfiguration( const QDomElement& element )
{
    // Use the -1 one here to tell the group box:
    // Dont use your QGridLayout. We will use ours.
    setColumnLayout( -1, Qt::Vertical );

    // Handle our children. QWidget knows about this and wont
    // create them again.
    QDomElement l = element.namedItem( "Layout" ).toElement();
    if ( !l.isNull() )
    {
	if ( !( l.firstChild().toElement().toLayout( vbox ) ) )
	    return FALSE;
    }
    else
    {
	// The "Widget" tag only appears because people want to have
	// absolute positioning. We have to create a placeholder widget here
	// in order of not overwriting the title. This will break some Styles, but
	// when people ue absolute positioning I cant help them anyways.
	QWidget* w = new QWidget( this );
	vbox->addWidget( w );
	QDomElement e = element.firstChild().toElement();
	for( ; !e.isNull(); e = e.nextSibling().toElement() )
        {
	    if ( e.tagName() == "Widget" )
	    {
		if ( !e.firstChild().toElement().toWidget( w ) )
		    return FALSE;
	    }
	}
    }
    
    if ( !QFrame::setConfiguration( element ) )
	return FALSE;

    return TRUE;
}
#endif // QT_BUILDER

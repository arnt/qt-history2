/**********************************************************************
** $Id: $
**
** Implementation of QGroupBox widget class
**
** Created : 950203
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qgroupbox.h"
#ifndef QT_NO_GROUPBOX
#include "qlayout.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "qaccel.h"
#include "qradiobutton.h"
#include "qfocusdata.h"
#include "qobjectlist.h"
#include "qdrawutil.h"
#include "qapplication.h"
#include "qstyle.h"
#if defined(QT_ACCESSIBILITY_SUPPORT)
#include "qaccessible.h"
#endif

/*!
  \class QGroupBox qgroupbox.h
  \brief The QGroupBox widget provides a group box frame with a title.

  \ingroup organizers
  \ingroup geomanagement
  \ingroup appearance
  \mainclass

  A group box provides a frame, a title and a keyboard shortcut, and
  displays various other widgets inside itself.  The title is on top,
  the keyboard shortcut moves keyboard focus to one of the group box's
  child widgets, and the child widgets are arranged in an array inside
  the frame.

  The simplest way to use it is to create a group box with the desired
  number of columns and orientation, and then just create widgets with
  the group box as parent.

  However, it is also possible to change the orientation() and number
  of columns() after construction, or to ignore all the automatic
  layout support and manage all that yourself. You can add 'empty'
  spaces to the group box with addSpace().

  QGroupBox also lets you set the title() (normally set in the
  constructor) and the title's alignment().

  You can change the spacing used by the group box with
  setInsideMargin() and setInsideSpacing().

  <img src=qgrpbox-w.png>

  \sa QButtonGroup
*/


/*!
  Constructs a group box widget with no title.

  The \a parent and \a name arguments are passed to the QWidget constructor.

  This constructor does not do automatic layout.
*/

QGroupBox::QGroupBox( QWidget *parent, const char *name )
    : QFrame( parent, name )
{
    init();
}

/*!
  Constructs a group box with the title \a title.

  The \a parent and \a name arguments are passed to the QWidget constructor.

  This constructor does not do automatic layout.
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

  The \a parent and \a name arguments are passed to the QWidget constructor.
*/

QGroupBox::QGroupBox( int strips, Orientation orientation,
		    QWidget *parent, const char *name )
    : QFrame( parent, name )
{
    init();
    setColumnLayout( strips, orientation );
}

/*!
  Constructs a group box titled \a title. Child widgets will be arranged
  in \a strips rows or columns (depending on \a orientation).

  The \a parent and \a name arguments are passed to the QWidget constructor.
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
    align = AlignAuto;
    fs = QFrame::Box | QFrame::Sunken;
    setFrameStyle( fs );
#ifndef QT_NO_ACCEL
    accel = 0;
#endif
    vbox = 0;
    grid = 0;
    d = 0;	//we use d directly to store a QSpacerItem
    lenvisible = 0;
    nCols = nRows = 0;
    dir = Horizontal;
    marg = 11;
    spac = 5;
}

void QGroupBox::setTextSpacer()
{
    QSpacerItem *sp = (QSpacerItem*)d;
    if ( ! sp )
	return;
    int h = 0;
    int w = 0;
    if ( lenvisible ) {
	QFontMetrics fm = fontMetrics();
	h = fm.height();
	w = fm.width( str, lenvisible ) + 2*fm.width( "xx" );
	if ( layout() ) {
	    int m = layout()->margin();
	    // do we have a child layout?
	    for ( QLayoutIterator it = layout()->iterator(); it.current(); ++it ) {
		if ( it.current()->layout() ) {
		    m += it.current()->layout()->margin();
		    break;
		}
	    }
	    if ( m > 4 )
		h -= m - 4;
	    h = QMAX( 0, h );
	}
    }
    sp->changeSize( w, h, QSizePolicy::Minimum, QSizePolicy::Fixed );
}


void QGroupBox::setTitle( const QString &title )
{
    if ( str == title )				// no change
	return;
    str = title;
#ifndef QT_NO_ACCEL
    if ( accel )
	delete accel;
    accel = 0;
    int s = QAccel::shortcutKey( title );
    if ( s ) {
	accel = new QAccel( this, "automatic focus-change accelerator" );
	accel->connectItem( accel->insertItem( s, 0 ),
			    this, SLOT(fixFocus()) );
    }
#endif
    calculateFrame();
    setTextSpacer();
    if ( layout() ) {
	layout()->activate();
	QSize s( size() );
	QSize ms( minimumSizeHint() );
	resize( QMAX( s.width(), ms.width() ),
		QMAX( s.height(), ms.height() ) );
    }

    update();
    updateGeometry();
#if defined(QT_ACCESSIBILITY_SUPPORT)
    QAccessible::updateAccessibility( this, 0, QAccessible::NameChanged );
#endif
}

/*!
  \property QGroupBox::title
  \brief the group box title text.

  The group box title text will have a focus-change keyboard
  accelerator if the title contains \&, followed by a letter.

  \code
      g->setTitle( "&User information" );
  \endcode
  This produces "User information" with the U underlined;
  Alt-U moves the keyboard focus to the group box.

*/

/*!
  \property QGroupBox::alignment
  \brief the alignment of the group box title.

  The title is always placed on the upper frame line; however,
  the horizontal alignment can be specified by the alignment parameter.

  The alignment is one of the following flags:
  \list
  \i \c AlignAuto aligns the title accroding to the language, usually left.
  \i \c AlignLeft aligns the title text to the left.
  \i \c AlignRight aligns the title text to the right.
  \i \c AlignHCenter aligns the title text centered.
  \endlist

  The default alignment is \c AlignAuto.

  \sa Qt::AlignmentFlags
*/

void QGroupBox::setAlignment( int alignment )
{
    align = alignment;
    update();
}

/*! \reimp
*/
void QGroupBox::resizeEvent( QResizeEvent *e )
{
    QFrame::resizeEvent(e);
    calculateFrame();
}

/*! \reimp

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
	else if ( align & AlignLeft )		 // left alignment
	    x = 8;
	else { // auto align
	    if( QApplication::reverseLayout() )
		x = frameRect().width() - tw - 8;
	    else
		x = 8;
	}
	QRect r( x, 0, tw, h );

	style().drawItem( &paint, r, AlignCenter | ShowPrefix, colorGroup(),
			  isEnabled(), 0, str );
	paint.setClipRegion( event->region().subtract( r ) ); // clip everything but title
    }
    // I really think this should call drawFrame() instead...
    style().drawPrimitive( QStyle::PE_GroupBoxFrame, &paint, frameRect(),
			   colorGroup(), QStyle::Style_Default, 
			   QStyleOption(lineWidth(), midLineWidth(),
			    frameShape(), frameShadow()) );
    drawContents( &paint );			// draw the contents
}


/*!
  Adds an empty cell at the next free position. If \a size is greater
  than 0, the empty cell has a fixed height or width.
  If the group box is oriented horizontally, the empty cell has a fixed
  height; if oriented vertically, it has a fixed width.

  Use this method to separate the widgets in the group box or to skip
  the next free cell. For performance reasons, call this method after
  calling setColumnLayout() or by changing the \l QGroupBox::columns or
  \l QGroupBox::orientation properties. It is generally a good idea to call
  these methods first (if needed at all), and insert the widgets and
  spaces afterwards.
*/
void QGroupBox::addSpace( int size )
{
    QApplication::sendPostedEvents( this, QEvent::ChildInserted );

    if ( nCols <= 0 || nRows <= 0 )
	return;

    if ( row >= nRows || col >= nCols )
	grid->expand( row+1, col+1 );

    if ( size > 0 ) {
	QSpacerItem *spacer
	    = new QSpacerItem( ( dir == Horizontal ) ? 0 : size,
			       ( dir == Vertical ) ? 0 : size,
			       QSizePolicy::Fixed, QSizePolicy::Fixed );
	grid->addItem( spacer, row, col );
    }

    skip();
}

/*!
  \property QGroupBox::columns
  \brief the number of columns in the group box

  Usually it is not a good idea to set this property because it is slow
  (it does a complete layout).  It is better to set the number of columns
  directly in the constructor.
*/
int QGroupBox::columns() const
{
    if ( dir == Horizontal )
	return nCols;
    return nRows;
}

void QGroupBox::setColumns( int c )
{
    setColumnLayout( c, dir );
}

/*!
  Returns the width of the blank spacing between the items in the group
  and the frame of the group.

  Only applies if the groupbox has a defined orientation.

  The default is about 11.

  \sa setInsideMargin(), orientation
*/
int QGroupBox::insideMargin() const
{
    return marg;
}

/*!
  Returns the width of the blank spacing between each of the items in the
  group.

  Only applies if the groupbox has a defined orientation.

  The default is about 5.

  \sa setInsideSpacing(), orientation
*/
int QGroupBox::insideSpacing() const
{
    return spac;
}

/*!
  Sets the the width of the blank spacing between each of the items in the
  group to \a m pixels.

  \sa insideSpacing()
*/
void QGroupBox::setInsideMargin( int m )
{
    marg = m;
    setColumnLayout( columns(), dir );
}

/*!
  Sets the width of the blank spacing between each of the items in the
  group to \a s pixels.
*/
void QGroupBox::setInsideSpacing( int s )
{
    spac = s;
    setColumnLayout( columns(), dir );
}

/*!
  \property QGroupBox::orientation
  \brief the current orientation of the group box.

  Usually it is not a good idea to set this property because it is slow
  (it does a complete layout). It is better to set the orientation directly
  in the constructor.
*/
void QGroupBox::setOrientation( Qt::Orientation o )
{
    setColumnLayout( columns(), o );
}

/*!
  Changes the layout of the group box. This function is useful only in
  combination with the default constructor that does not take any
  layout information. This function will put all existing children in
  the new layout. It is not good Qt programming style to
  call this function after children have been inserted.
  Sets the number of columns to be \a columns and the orientation to
  be \a direction.

  \sa orientation columns
 */
void QGroupBox::setColumnLayout(int columns, Orientation direction)
{
    if ( layout() )
      delete layout();
    vbox = 0;
    grid = 0;

    if ( columns < 0 ) // if 0, we create the vbox but not the grid. See below.
	return;

    vbox = new QVBoxLayout( this, marg, 0 );

    QSpacerItem *spacer = new QSpacerItem( 0, 0, QSizePolicy::Minimum,
					   QSizePolicy::Fixed );
    d = (QGroupBoxPrivate*) spacer;
    setTextSpacer();
    vbox->addItem( spacer );

    nCols = 0;
    nRows = 0;
    dir = direction;

    // Send all child events and ignore them. Otherwise we will end up
    // with doubled insertion. This won't do anything because nCols ==
    // nRows == 0.
    QApplication::sendPostedEvents( this, QEvent::ChildInserted );

    // if 0 or smaller , create a vbox-layout but no grid. This allows
    // the designer to handle its own grid layout in a group box.
    if ( columns <= 0 )
	return;

    dir = direction;
    if ( dir == Horizontal ) {
	nCols = columns;
	nRows = 1;
    } else {
	nCols = 1;
	nRows = columns;
    }
    grid = new QGridLayout( nRows, nCols, spac );
    row = col = 0;
    grid->setAlignment( AlignTop );
    vbox->addLayout( grid );

    // Add all children
    if ( children() ) {
	QObjectListIt it( *children() );
	QWidget *w;
	while( (w=(QWidget *)it.current()) != 0 ) {
	    ++it;
	    if ( w->isWidgetType() )
		insertWid( w );
	}
    }
}


/*! \reimp  */
bool QGroupBox::event( QEvent * e )
{
    if ( e->type() == QEvent::LayoutHint && layout() )
	setTextSpacer();
    return QFrame::event( e );
}

/*!\reimp */
void QGroupBox::childEvent( QChildEvent *c )
{
    // Similar to QGrid::childEvent()
    if ( !grid || !c->inserted() || !c->child()->isWidgetType() )
	return;
    insertWid( (QWidget*)c->child() );
}

void QGroupBox::insertWid( QWidget* w )
{
    if ( row >= nRows || col >= nCols )
	grid->expand( row+1, col+1 );
    grid->addWidget( w, row, col );
    skip();
    QApplication::postEvent( this, new QEvent( QEvent::LayoutHint ) );
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


/*!  This private slot finds a widget in this group box that can
accept focus, and gives the focus to that widget.
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
	if ( p == this && ( w->focusPolicy() & TabFocus ) == TabFocus && w->isVisible() ) {
	    if ( w->hasFocus()
#ifndef QT_NO_RADIOBUTTON
		 || ( !best &&
		   w->inherits( "QRadioButton" ) &&
		   ((QRadioButton*)w)->isChecked() )
#endif
		    )
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



/*! \reimp
 */
void QGroupBox::focusInEvent( QFocusEvent * )
{ // note no call to super
    fixFocus();
}


/*!\reimp
 */
void QGroupBox::fontChange( const QFont & oldFont )
{
    calculateFrame();
    setTextSpacer();
    QWidget::fontChange( oldFont );
}

/*!
  \reimp
*/

QSize QGroupBox::sizeHint() const
{
    QFontMetrics fm( font() );
    int tw = fm.width( title() ) + 2 * fm.width( "xx" );

    QSize s;
    if ( layout() ) {
	s = QFrame::sizeHint();
	return s.expandedTo( QSize( tw, 0 ) );
    } else {
	QRect r = childrenRect();
	QSize s( 100, 50 );
	s = s.expandedTo( QSize( tw, 0 ) );
	if ( r.isNull() )
	    return s;

	return s.expandedTo( QSize( r.width() + 2 * r.x(), r.height()+ 2 * r.y() ) );
    }
}

#endif

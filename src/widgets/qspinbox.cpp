/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qspinbox.cpp#13 $
**
** Implementation of QSpinBox widget class
**
** Created : yes
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qspinbox.h"
#include "qpushbt.h"
#include "qstrlist.h"
#include "qpainter.h"
#include "qkeycode.h"
#include "qbitmap.h"

/*! \class QSpinBox qspinbox.h

  \brief The QSpinBox class provides a spin box, sometimes called
  up-down widget, little arrows widget or spin button.

  The spin box deviates from Motif look a bit.

  <img src=qspinbox-m.gif> <img src=qspinbox-w.gif>
*/


/*!  Creates an empty, non-wrapping spin box with TabFocus \link
  setFocusPolicy() focus policy. \endlink.
*/

QSpinBox::QSpinBox( QWidget * parent , const char * name )
    : QFrame( parent, name )
{
    d = 0; // not used
    wrap = FALSE;
    c = 0;
    l = 0;
    setFocusPolicy( TabFocus );
    up = new QPushButton( this, "up" );
    down = new QPushButton( this, "down" );
    up->setFocusPolicy( QWidget::NoFocus );
    down->setFocusPolicy( QWidget::NoFocus );
    up->setAutoRepeat( TRUE );
    down->setAutoRepeat( TRUE );
    if ( style() == WindowsStyle )
	setFrameStyle( WinPanel | Sunken );
    else
	setFrameStyle( Panel | Sunken );
    setLineWidth( 2 );

    connect( up, SIGNAL(clicked()), SLOT(next()) );
    connect( down, SIGNAL(clicked()), SLOT(prev()) );
}


/*!  Deletes the spin box, freeing all memory and other resoures.
*/

QSpinBox::~QSpinBox()
{
    delete l;
    l = 0;
}


/*!  Appends a deep copy of \a item to the end of the spin box list.

  \sa insert clear()
*/

void QSpinBox::append( const char * item )
{
    if ( !item )
	return;

    if ( !l )
	l = new QStrList;

    l->append( item );
    enableButtons();
}


/*!  Appends deep copies of \a items to the and of the spin box list.

  \sa insert clear()
*/

void QSpinBox::append( const char ** items )
{
    if ( !items || !*items )
	return;

    if ( !l )
	l = new QStrList;

    int i = 0;
    while ( items[i] )
	l->append( items[i++] );
    enableButtons();
}


/*!  Appends deep copies of \a items to the and of the spin box list.

  \sa insert() clear()
*/

void QSpinBox::append( const QStrList * items )
{
    if ( !items || items->count() == 0 )
	return;

    if ( !l )
	l = new QStrList;

    QStrListIterator it( *items );
    const char * tmp;
    while ( (tmp=it.current()) ) {
	++it;
	l->append( tmp );
    }
}


/*!  Inserts a deep copy of \a item at position \a index.  The first
  position is numbered 0.

  The current position is unchanged, so the displayed data may change.

  \sa append() clear() current() text()
*/

void QSpinBox::insert( const char * item, int index )
{
    if ( !item )
	return;

    if ( !l )
	l = new QStrList;

    l->insert( index, item );
    enableButtons();
    repaint();
}


/*!  Inserts deep copies of \a items starting at position \a index.
  The first position is numbered 0.

  The current position is unchanged, so the displayed data may change.

  \sa append() clear() current() text()
*/

void QSpinBox::insert( const char ** items, int index )
{
    if ( !items || !*items )
	return;

    if ( !l )
	l = new QStrList;

    int i = 0;
    while ( items[i] )
	l->insert(  index+i, items[i++] );
    enableButtons();
    repaint();
}


/*!  Inserts deep copies of \a items starting at position \a index.
  The first position is numbered 0.

  The current position is unchanged, so the displayed data may change.

  \sa append() clear() current() text()
*/

void QSpinBox::insert( const QStrList * items, int index )
{
    if ( !items || items->count() == 0 )
	return;

    if ( !l )
	l = new QStrList;

    QStrListIterator it( *items );
    const char * tmp;
    while ( (tmp=it.current()) ) {
	++it;
	l->insert( index++, tmp );
    }
    repaint();
}


/*!  Clears the entire list.  This leaves the spin box in limbo
  until you insert something else it can reasonably display.

  \sa append() insert()
*/

void QSpinBox::clear()
{
    delete l;
    l = 0;
    c = 0;
    enableButtons();
}


/*!  Returns the item at \a index, or 0 if \a index is out of bounds.

  \sa current()
*/

const char * QSpinBox::text( int index ) const
{
    return l ? l->at( index ) : 0;
}


/*!  Makes the spin box wrap around from the last to the first item of
  \a w is TRUE, or not if not.

  \sa wrapping() setCurrent()
*/

void QSpinBox::setWrapping( bool w )
{
    wrap = w;
    enableButtons();
}

/*!  \fn bool QSpinBox::wrapping() const

  Returns the most recent setWrapping() value.
*/


/*!  Sets the spin box to display item \a i.

  This function is virtual and is the only way the value ever changes
  (except in the constructor, of course).

  \sa next() prev() current()
*/

void QSpinBox::setCurrent( int i )
{
    if ( c != i ) {
	c = i;
	enableButtons();
	repaint();
	emit selected( text( i ) );
    }
}

/*!  \fn int QSpinBox::current() const

  Returns the most recent setCurrent() value.
*/

/*!  Moves the spin box to the next value.  This is the same as
  clicking on the pointing-up button, and can be used for e.g.
  keyboard accelerators.

  This function is not virtual, but it calls setCurrent(), which is.

  \sa prev(), setCurrent(), current()
*/

void QSpinBox::next()
{
    if ( l != 0 && c < count()-1 )
	setCurrent( c + 1 );
    else if ( wrapping() )
	setCurrent( 0 );
}


/*!  Moves the spin box to the previous value.  This is the same as
  clicking on the pointing-down button, and can be used for e.g.
  keyboard accelerators.

  This function is not virtual, but it calls setCurrent(), which is.

  \sa next(), setCurrent(), current()
*/

void QSpinBox::prev()
{
    if ( c > 0 )
	setCurrent( c - 1 );
    else if ( wrapping() && l != 0 )
	setCurrent( count() - 1 );
}


/*! \fn void QSpinBox::selected( const char * )

  This signal is emitted every time the value of the spin box changes
  (by setCurrent(), by a keyboard accelerator, by mouse clicks, or by
  telepathy).

  Note that it is emitted \e every time, not just for the \"final\"
  step - if the user clicks 'up' three times, this signal is emitted
  three times.
*/


/*!  Draws the current value of the spin box using \a p.  The function
  is called by QFrame::paintEvent() and simply writes the current
  text and possibly a focus indication.
*/

void QSpinBox::drawContents( QPainter * p )
{
    QRect r = contentsRect();
    QFontMetrics fm = p->fontMetrics();

    if ( style() == WindowsStyle )
	p->fillRect( r, colorGroup().base() );

    p->setPen( colorGroup().text() );
    p->drawText( r.left() + 4,
		 ( r.height() - fm.height() ) / 2 + fm.ascent() + r.top(),
		 text( current() ) );

    if ( hasFocus() ) {
	if ( style() == WindowsStyle ) {
	    p->drawWinFocusRect( r.left()+1, r.top()+1,
				 up->pos().x() - r.left() - 2, r.height()-2 );
	} else {
	    p->setPen( colorGroup().foreground() );
	    p->drawRect( r.left()+1, r.top()+1,
			 up->pos().x() - r.left() - 2, r.height()-2 );
	}
    }
}


/*!  Returns a good-looking size for the spin box.  This functions
  takes into account the font and possible values of the spin box,
  so it's fairly slow.
*/

QSize QSpinBox::sizeHint() const
{
    QFontMetrics fm = fontMetrics();
    int h = fm.height();
    if ( h < 22 ) // enough space for the button pixmaps
	h = 22;
    int w = 40; // never less than 40 pixels for the value

    if ( l && l->first() ) { // find longest string
	int lw;
	do {
	    lw = fm.width( l->current() );
	    if ( lw > w )
		w = lw;
	} while ( l->next() );
    }
    
    return QSize( frameWidth() * 2 // right/left frame
		  + (8*h)/5 // buttons - approximate golden ratio
		  + 6 // right/left margins
		  + w, // longest value
		  frameWidth() * 2 // top/bottom frame
		  + 4 // top/bottom margins
		  + h // font height
		  );
}


/*!  Interprets the up and down keys; ignore everything else.

*/

void QSpinBox::keyPressEvent( QKeyEvent * e )
{
    if ( e->key() == Key_Up ) {
	next();
	e->accept();
    } else if ( e->key() == Key_Down ) {
	prev();
	e->accept();
    } else {
	e->ignore();
    }
}


/*!  Handles resize events for the spin box.  
*/

void QSpinBox::resizeEvent( QResizeEvent * e )
{
    if ( !up || !down ) // happens if the application has a pointer error
	return;

    QSize bs; // no, it's short for 'button size'
    bs.setHeight( e->size().height()/2 - frameWidth() );
    if ( bs.height() < 9 )
	bs.setHeight( 9 );
    bs.setWidth( bs.height() * 8 / 5 );

    if ( up->size() != bs ) {
	up->resize( bs );
	QBitmap bm( (bs.height() - 6) * 2 - 1, bs.height() - 6 );
	QPointArray a;
	a.setPoints( 3,
		     bm.height()-1, 0,
		     0, bm.height()-1,
		     bm.width()-1, bm.height()-1 );
	QPainter p;
	p.begin( &bm );
	p.eraseRect( 0, 0, bm.width(), bm.height() );
	p.setBrush( color1 );
	p.drawPolygon( a );
	p.end();
	up->setPixmap( bm );
    }

    if ( down->size() != bs ) {
	down->resize( bs );
	QBitmap bm( (bs.height() - 6) * 2 - 1, bs.height() - 6 );
	QPointArray a;
	a.setPoints( 3,
		     bm.height()-1, bm.height()-1,
		     0, 0,
		     bm.width()-1, 0 );
	QPainter p;
	p.begin( &bm );
	p.eraseRect( 0, 0, bm.width(), bm.height() );
	p.setBrush( color1 );
	p.drawPolygon( a );
	p.end();
	down->setPixmap( bm );
    }

    int x = e->size().width() - frameWidth() - bs.width();

    up->move( x, frameWidth() );
    down->move( x, height() - frameWidth() - up->height() );
}


/*!  Set the up and down buttons to enabled or disabled state, as
  appropriate.
*/

void QSpinBox::enableButtons()
{
    up->setEnabled( l && ( wrapping() || c < count()-1 ) );
    down->setEnabled( l && ( wrapping() || c > 0 ) );
}


/*!
  Returns the number of items in the spin box.
*/

int QSpinBox::count() const
{
    return l ? (int)(l->count()) : 0 ;
}
